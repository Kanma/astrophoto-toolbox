/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is essentially a reimplementation of parts of 'DeepSkyStacker',
 * which is released under a BSD 3-Clause license,
 * Copyright (c) 2006-2019, LucCoiffier
 * Copyright (c) 2018-2023,
 *      David C. Partridge, Tony Cook, Mat Draper,
 *      Simon C. Smith, Vitali Pelenjow, Michal Schulz, Martin Toeltsch
*/

#pragma once

#include <astrophoto-toolbox/stacking/utils/bitmapstacker.h>
#include <astrophoto-toolbox/stacking/utils/registration.h>
#include <astrophoto-toolbox/stacking/utils/backgroundcalibration.h>
#include <astrophoto-toolbox/stacking/utils/starmatcher.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <sstream>
#include <fstream>

namespace astrophototoolbox {
namespace stacking {


static const char* CONFIG_FILE = "stacking.txt";
static const char* MASTER_DARK = "master_dark.fits";
static const char* STACKED_FILE = "stacked.fits";

static std::filesystem::path CALIBRATED_PATH = "calibrated";
static std::filesystem::path CALIBRATED_LIGHT_FRAMES_PATH = CALIBRATED_PATH / "lightframes";

static std::filesystem::path MASTER_DARK_TEMP_PATH = "tmp_master_dark";
static std::filesystem::path STACKING_TEMP_PATH = "tmp_stacking";


//-----------------------------------------------------------------------------

template<class BITMAP>
LiveStacking<BITMAP>::~LiveStacking()
{
    cancel();
    wait();

    delete masterDarkThread;
    delete lightFramesThread;
    delete registrationThread;
    delete stackingThread;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::setup(
    LiveStackingListener* listener, const std::filesystem::path& folder, int luminancyThreshold
)
{
    if (running)
        return false;

    this->listener = listener;
    this->folder = folder;

    referenceFrame = -1;
    this->luminancyThreshold = luminancyThreshold;

    memset(&infos, 0, sizeof(infos));

    if (!masterDarkThread)
    {
        masterDarkThread = new threads::MasterDarkThread<BITMAP>(this, folder / MASTER_DARK);
        lightFramesThread = new threads::LightFrameThread<BITMAP>(this, folder / CALIBRATED_LIGHT_FRAMES_PATH);
        registrationThread = new threads::RegistrationThread<BITMAP>(this, folder / CALIBRATED_LIGHT_FRAMES_PATH);
        stackingThread = new threads::StackingThread<BITMAP>(this, folder / STACKED_FILE);
        stackingThread->setup(100, folder / STACKING_TEMP_PATH);
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::load()
{
    if (running)
        return false;

    std::ifstream input(folder / CONFIG_FILE, std::ios::in);
    if (!input.is_open())
        return false;

    darkFrames.clear();
    lightFrames.clear();

    memset(&infos, 0, sizeof(infos));

    std::string section = "";

    std::string line;
    while (std::getline(input, line))
    {
        if (line == "---")
        {
            section = "";
        }
        else if (section == "")
        {
            if ((line == "DARKFRAMES") || (line == "LIGHTFRAMES"))
                section = line;
        }
        else if (section == "DARKFRAMES")
        {
            addDarkFrame(line);
        }
        else if (section == "LIGHTFRAMES")
        {
            if (line.starts_with("REF "))
                referenceFrame = std::stoi(line.substr(4));
            else
                addLightFrame(line);
        }
    }

    bool hasMasterDark = std::filesystem::exists(folder / MASTER_DARK);

    if (hasMasterDark)
    {
        for (auto& entry : darkFrames)
            entry.stacked = true;
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::save()
{
    std::filesystem::create_directory(folder);

    std::ofstream output(folder / CONFIG_FILE, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        return false;

    std::lock_guard<std::mutex> lock(mutex);

    if (!darkFrames.empty())
    {
        output << "DARKFRAMES" << std::endl;

        for (const auto& entry : darkFrames)
            output << entry.filename << std::endl;

        output << "---" << std::endl;
    }

    if (!lightFrames.empty())
    {
        output << "LIGHTFRAMES" << std::endl;

        for (const auto& entry : lightFrames)
            output << entry.filename << std::endl;

        output << "REF " << referenceFrame << std::endl;
        output << "---" << std::endl;
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::addDarkFrame(const std::string& filename)
{
    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(path))
        return false;

    mutex.lock();
    dark_frame_t darkFrame;
    darkFrame.filename = filename;

    darkFrames.push_back(darkFrame);

    infos.nbDarkFrames = darkFrames.size();
    mutex.unlock();

    if (running)
    {
        // Cancel all pending jobs
        cancel();
        wait();

        // Delete the files that will need to be recomputed
        std::filesystem::remove(folder / MASTER_DARK);
        std::filesystem::remove(folder / STACKED_FILE);
        std::filesystem::remove_all(folder / CALIBRATED_LIGHT_FRAMES_PATH);

        // Recreate the needed folders
        std::filesystem::create_directories(folder / CALIBRATED_LIGHT_FRAMES_PATH);

        // Reset the light frames status
        for (auto& entry : lightFrames)
        {
            entry.calibrated = false;
            entry.registered = false;
            entry.stacked = false;
            entry.valid = true;
            entry.ready = true;
        }

        memset(&infos.lightFrames, 0, sizeof(infos.lightFrames));
        infos.lightFrames.nb = lightFrames.size();

        stackingThread->setup(100, folder / STACKING_TEMP_PATH);

        // Restart the processing
        start();
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::addLightFrame(const std::string& filename)
{
    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(path))
        return false;

    light_frame_t entry;
    entry.filename = filename;

    auto calibratedFilename = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(filename);
    entry.calibrated = std::filesystem::exists(calibratedFilename);

    if (entry.calibrated)
    {
        FITS fits;
        if (fits.open(calibratedFilename))
        {
            star_list_t stars = fits.readStars();
            entry.registered = !stars.empty();
        }
    }

    mutex.lock();

    lightFrames.push_back(entry);
    infos.lightFrames.nb = lightFrames.size();

    if (entry.calibrated)
        ++infos.lightFrames.nbProcessed;

    if (entry.registered)
    {
        ++infos.lightFrames.nbRegistered;
        ++infos.lightFrames.nbValid;
    }

    if (lightFrames.size() == 1)
        referenceFrame = 0;

    mutex.unlock();

    if (running)
    {
        if (!entry.calibrated)
           lightFramesThread->processFrames({ filename });
        else if (!entry.registered)
            registrationThread->processFrames({ filename });
        else
            stackingThread->processFrames({ filename });
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::setReference(size_t index)
{
    mutex.lock();

    if ((index < 0) || (index >= lightFrames.size()))
    {
        mutex.unlock();
        return;
    }

    if (index == referenceFrame)
    {
        mutex.unlock();
        return;
    }

    referenceFrame = index;

    mutex.unlock();

    bool isRunning = running;

    // Cancel all pending jobs
    if (isRunning)
    {
        cancel();
        wait();
    }

    // Delete the files that will need to be recomputed
    std::filesystem::remove(folder / STACKED_FILE);
    std::filesystem::remove_all(folder / CALIBRATED_LIGHT_FRAMES_PATH);

    // Recreate the needed folders
    std::filesystem::create_directories(folder / CALIBRATED_LIGHT_FRAMES_PATH);

    // Reset the light frames status
    for (auto& entry : lightFrames)
    {
        entry.calibrated = false;
        entry.registered = false;
        entry.stacked = false;
        entry.valid = true;
        entry.ready = true;
    }

    memset(&infos.lightFrames, 0, sizeof(infos.lightFrames));
    infos.lightFrames.nb = lightFrames.size();

    stackingThread->setup(100, folder / STACKING_TEMP_PATH);

    // Restart the processing
    if (isRunning)
        start();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::setLuminancyThreshold(int threshold)
{
    threshold = std::min(std::max(threshold, -1), 100);

    if (threshold == luminancyThreshold)
        return;

    luminancyThreshold = threshold;
    changingLuminancyThreshold = true;

    bool isRunning = running;

    // Cancel all relevant pending jobs
    if (isRunning)
    {
        registrationThread->cancel();
        stackingThread->cancel();
        
        registrationThread->wait();
        stackingThread->wait();
    }

    // Delete the files that will need to be recomputed
    std::filesystem::remove(folder / STACKED_FILE);

    mutex.lock();

    // Reset the light frames status
    for (auto& entry : lightFrames)
    {
        entry.registered = false;
        entry.stacked = false;
        entry.valid = true;
        entry.ready = entry.calibrated;
    }

    infos.lightFrames.nbRegistered = 0;
    infos.lightFrames.nbValid = 0;
    infos.lightFrames.nbStacked = 0;

    stackingThread->setup(100, folder / STACKING_TEMP_PATH);

    // Restart the processing
    if (isRunning)
    {
        auto reference = lightFrames[referenceFrame];
        std::vector<std::string> lightFramesToRegister;

        for (auto& entry : lightFrames)
        {
            if (!entry.valid || !entry.ready)
                continue;

            if (entry.filename != reference.filename)
            {
                if (entry.calibrated)
                    lightFramesToRegister.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
            }

            entry.ready = false;
        }

        mutex.unlock();

        if (reference.calibrated)
        {
            registrationThread->processReferenceFrame(
                folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(reference.filename), luminancyThreshold
            );
        }

        if (!lightFramesToRegister.empty())
            registrationThread->processFrames(lightFramesToRegister);
    }
    else
    {
        mutex.unlock();
    }

    changingLuminancyThreshold = false;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::start()
{
    if (running)
        return false;

    cancelled = false;
    running = true;
    nextStep();

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::cancel()
{
    if (!running)
        return;

    cancelled = true;

    masterDarkThread->cancel();
    lightFramesThread->cancel();
    registrationThread->cancel();
    stackingThread->cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::wait()
{
    masterDarkThread->wait();
    lightFramesThread->wait();
    registrationThread->wait();
    stackingThread->wait();

    running = false;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::masterDarkFrameComputed(const std::string& filename, bool success)
{
    mutex.lock();

    for (auto& entry : darkFrames)
    {
        if (!entry.ready)
        {
            entry.ready = true;
            entry.stacked = success;
        }
    }

    if (success)
        lightFramesThread->setMasterDark(filename);

    if (!cancelled)
        listener->progressNotification(infos);

    mutex.unlock();

    if (success && !cancelled)
        nextStep();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameProcessed(const std::string& filename, bool success)
{
    mutex.lock();

    for (auto& entry : lightFrames)
    {
        if (entry.filename == filename)
        {
            if (success)
            {
                entry.calibrated = true;
                ++infos.lightFrames.nbProcessed;

                if (!cancelled && !changingLuminancyThreshold)
                {
                    auto fullpath = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(filename);

                    if (lightFrames[referenceFrame].filename == filename)
                        registrationThread->processReferenceFrame({ fullpath }, luminancyThreshold);
                    else
                        registrationThread->processFrames({ fullpath });
                }
                else
                {
                    entry.ready = true;
                }
            }
            else
            {
                entry.valid = false;
                entry.ready = true;
            }

            break;
        }
    }

    if (!cancelled)
        listener->progressNotification(infos);

    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameRegistered(const std::string& filename, bool success)
{
    mutex.lock();

    for (auto& entry : lightFrames)
    {
        auto fullpath = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename);

        if (fullpath == filename)
        {
            if (success)
            {
                entry.registered = true;
                ++infos.lightFrames.nbRegistered;
                ++infos.lightFrames.nbValid;

                if (!cancelled)
                    stackingThread->processFrames({ fullpath });
                else
                    entry.ready = true;
            }
            else
            {
                entry.valid = false;
                entry.ready = true;
            }

            break;
        }
    }

    if (!cancelled)
        listener->progressNotification(infos);

    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFramesStacked(const std::string& filename, unsigned int nbFrames)
{
    if (!cancelled)
    {
        unsigned int nb = 0;

        for (auto& entry : lightFrames)
        {
            if (entry.registered && entry.valid)
            {
                entry.stacked = true;
                entry.ready = true;
                ++nb;

                if (nb == nbFrames)
                    break;
            }
        }

        infos.lightFrames.nbStacked = nbFrames;

        listener->progressNotification(infos);
        listener->stackingDone(filename);
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::nextStep()
{
    mutex.lock();

    bool mustRecomputeMasterDark = false;
    for (const auto& entry : darkFrames)
    {
        if (!entry.stacked)
        {
            mustRecomputeMasterDark = true;
            break;
        }
    }

    if (mustRecomputeMasterDark)
    {
        std::vector<std::string> allDarkFrames;
        for (auto& entry : darkFrames)
        {
            allDarkFrames.push_back(entry.filename);
            entry.ready = false;
        }

        mutex.unlock();

        masterDarkThread->processFrames(allDarkFrames, folder / MASTER_DARK_TEMP_PATH);
    }
    else if (!lightFrames.empty())
    {
        auto reference = lightFrames[referenceFrame];
        std::vector<std::string> lightFramesToProcess;
        std::vector<std::string> lightFramesToRegister;
        std::vector<std::string> lightFramesToStack;

        for (auto& entry : lightFrames)
        {
            if (!entry.valid || !entry.ready)
                continue;

            if (entry.filename != reference.filename)
            {
                if (!entry.calibrated)
                    lightFramesToProcess.push_back(entry.filename);
                else if (!entry.registered)
                    lightFramesToRegister.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
                else if (!entry.stacked)
                    lightFramesToStack.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
            }

            entry.ready = false;
        }

        mutex.unlock();

        lightFramesThread->processReferenceFrame(reference.filename);

        if (reference.calibrated)
            registrationThread->processReferenceFrame(reference.filename);

        if (reference.registered)
            stackingThread->processFrames({ reference.filename });

        if (!lightFramesToProcess.empty())
            lightFramesThread->processFrames(lightFramesToProcess);

        if (!lightFramesToRegister.empty())
            registrationThread->processFrames(lightFramesToRegister);

        if (!lightFramesToStack.empty())
            stackingThread->processFrames(lightFramesToStack);
    }
    else
    {
        mutex.unlock();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string LiveStacking<BITMAP>::getCalibratedFilename(const std::string& path)
{
    std::string filename = std::filesystem::path(path).filename().string();
    std::string extension = std::filesystem::path(path).extension().string();
    return filename.replace(filename.find(extension), extension.size(), ".fits");
};

}
}
