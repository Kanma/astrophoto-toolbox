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
        masterDarkThread = new threads::MasterDarkThread<BITMAP>(this, folder / MASTER_DARK, folder / "tmp_masterdark");
        lightFramesThread = new threads::LightFrameThread<BITMAP>(this, folder / CALIBRATED_LIGHT_FRAMES_PATH);
        registrationThread = new threads::RegistrationThread<BITMAP>(this, folder / CALIBRATED_LIGHT_FRAMES_PATH);
        stackingThread = new threads::StackingThread<BITMAP>(this, folder / STACKED_FILE);
        stackingThread->setup(10, folder / STACKING_TEMP_PATH);
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
    infos.lightFrames.entries.clear();

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

    std::lock_guard<std::mutex> lock(framesMutex);

    if (!darkFrames.empty())
    {
        output << "DARKFRAMES" << std::endl;

        for (const auto& entry : darkFrames)
            output << entry.filename << std::endl;

        output << "---" << std::endl;
    }

    if (!infos.lightFrames.entries.empty())
    {
        output << "LIGHTFRAMES" << std::endl;

        for (const auto& entry : infos.lightFrames.entries)
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
    std::filesystem::path path = getAbsoluteFilename(filename);
    if (!std::filesystem::exists(path))
        return false;

    framesMutex.lock();

    dark_frame_t darkFrame;
    darkFrame.filename = filename;

    darkFrames.push_back(darkFrame);

    infos.nbDarkFrames = darkFrames.size();

    framesMutex.unlock();

    if (running)
    {
        // Reset all pending jobs
        masterDarkThread->reset();
        lightFramesThread->reset();
        registrationThread->reset();
        stackingThread->reset();

        // Delete the files that will need to be recomputed
        std::filesystem::remove(folder / MASTER_DARK);
        std::filesystem::remove(folder / STACKED_FILE);
        std::filesystem::remove_all(folder / CALIBRATED_LIGHT_FRAMES_PATH);

        // Recreate the needed folders
        std::filesystem::create_directories(folder / CALIBRATED_LIGHT_FRAMES_PATH);

        // Reset the light frames status
        for (auto& entry : infos.lightFrames.entries)
        {
            entry.calibrated = false;
            entry.registered = false;
            entry.stacked = false;
            entry.valid = true;
            entry.processing = false;
        }

        infos.lightFrames.nb = infos.lightFrames.entries.size();
        infos.lightFrames.nbProcessed = 0;
        infos.lightFrames.nbRegistered = 0;
        infos.lightFrames.nbValid = 0;
        infos.lightFrames.nbStacking = 0;
        infos.lightFrames.nbStacked = 0;

        // Restart the processing
        nextStep();
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::addLightFrame(const std::string& filename)
{
    std::filesystem::path path = getAbsoluteFilename(filename);
    if (!std::filesystem::exists(path))
        return false;

    live_stacking_light_frame_t entry;
    entry.filename = filename;

    auto calibratedFilename = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(filename);
    entry.calibrated = std::filesystem::exists(calibratedFilename);

    if (entry.calibrated)
    {
        FITS fits;
        if (fits.open(calibratedFilename))
        {
            fits.read("REGISTERED", entry.valid);
            star_list_t stars = fits.readStars();
            entry.registered = !stars.empty();
        }
    }

    framesMutex.lock();

    infos.lightFrames.entries.push_back(entry);
    infos.lightFrames.nb = infos.lightFrames.entries.size();

    if (entry.calibrated)
        ++infos.lightFrames.nbProcessed;

    if (entry.registered)
    {
        ++infos.lightFrames.nbRegistered;

        if (entry.valid)
            ++infos.lightFrames.nbValid;
    }

    if (infos.lightFrames.entries.size() == 1)
        referenceFrame = 0;

    if (running)
        listener->progressNotification(infos);

    framesMutex.unlock();

    if (step == STEP_STACKING)
    {
        if (!entry.calibrated)
           lightFramesThread->processFrames({ path });
        else if (!entry.registered)
            registrationThread->processFrames({ calibratedFilename });
        else
            stackingThread->processFrames({ calibratedFilename });
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::setReference(size_t index)
{
    framesMutex.lock();

    if ((index < 0) || (index >= infos.lightFrames.entries.size()))
    {
        framesMutex.unlock();
        return;
    }

    if (index == referenceFrame)
    {
        framesMutex.unlock();
        return;
    }

    referenceFrame = index;

    framesMutex.unlock();

    bool hasPendingJobs = (step != STEP_NONE);

    // Reset all pending jobs
    if (hasPendingJobs)
    {
        lightFramesThread->reset();
        registrationThread->reset();
        stackingThread->reset();

        if (step == STEP_STACKING)
            step = STEP_MASTER_DARK;
    }

    framesMutex.lock();

    // Delete the files that will need to be recomputed
    std::filesystem::remove(folder / STACKED_FILE);
    std::filesystem::remove_all(folder / CALIBRATED_LIGHT_FRAMES_PATH);

    // Recreate the needed folders
    std::filesystem::create_directories(folder / CALIBRATED_LIGHT_FRAMES_PATH);

    // Reset the light frames status
    for (auto& entry : infos.lightFrames.entries)
    {
        entry.calibrated = false;
        entry.registered = false;
        entry.stacked = false;
        entry.valid = true;
        entry.processing = false;
    }

    infos.lightFrames.nb = infos.lightFrames.entries.size();
    infos.lightFrames.nbProcessed = 0;
    infos.lightFrames.nbRegistered = 0;
    infos.lightFrames.nbValid = 0;
    infos.lightFrames.nbStacking = 0;
    infos.lightFrames.nbStacked = 0;

    framesMutex.unlock();

    // Restart the processing
    if (hasPendingJobs)
        nextStep();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::setLuminancyThreshold(int threshold)
{
    threshold = std::min(std::max(threshold, -1), 100);

    if (threshold == luminancyThreshold)
        return;

    luminancyThreshold = threshold;

    bool hasPendingJobs = (step != STEP_NONE);

    // Reset all relevant pending jobs
    if (hasPendingJobs)
    {
        registrationThread->reset();
        stackingThread->reset();
    }

    // Delete the files that will need to be recomputed
    std::filesystem::remove(folder / STACKED_FILE);

    framesMutex.lock();

    // Reset the light frames status
    for (auto& entry : infos.lightFrames.entries)
    {
        entry.registered = false;
        entry.stacked = false;
        entry.valid = true;
        entry.processing = false;
    }

    infos.lightFrames.nbRegistered = 0;
    infos.lightFrames.nbValid = 0;
    infos.lightFrames.nbStacked = 0;

    // Restart the processing
    if (hasPendingJobs)
    {
        auto reference = infos.lightFrames.entries[referenceFrame];
        std::vector<std::string> lightFramesToRegister;

        for (auto& entry : infos.lightFrames.entries)
        {
            if ((entry.filename != reference.filename) && entry.calibrated)
                lightFramesToRegister.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
        }

        framesMutex.unlock();

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
        framesMutex.unlock();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LiveStacking<BITMAP>::start()
{
    if (running)
        return false;

    running = true;
    step = STEP_NONE;

    std::latch latch(4);

    masterDarkThread->start(&latch);
    lightFramesThread->start(&latch);
    registrationThread->start(&latch);
    stackingThread->start(&latch);

    latch.wait();

    nextStep();

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::cancel()
{
    if (!running)
        return;

    std::latch latch(4);

    masterDarkThread->cancel(&latch);
    lightFramesThread->cancel(&latch);
    registrationThread->cancel(&latch);
    stackingThread->cancel(&latch);

    latch.wait();

    masterDarkThread->join();
    lightFramesThread->join();
    registrationThread->join();
    stackingThread->join();

    running = false;
    step = STEP_NONE;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::cancelAsync()
{
    if (!running)
        return;

    stopThread = std::thread(&LiveStacking<BITMAP>::cancel, this);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::stop()
{
    if (!running)
        return;

    masterDarkThread->stop();
    masterDarkThread->join();

    lightFramesThread->stop();
    lightFramesThread->join();

    registrationThread->stop();
    registrationThread->join();

    stackingThread->stop();
    stackingThread->join();

    running = false;
    step = STEP_NONE;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::stopAsync()
{
    if (!running)
        return;

    stopThread = std::thread(&LiveStacking<BITMAP>::stop, this);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::wait()
{
    if (stopThread.joinable())
        stopThread.join();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::masterDarkFrameComputed(const std::string& filename, bool success)
{
    framesMutex.lock();

    for (auto& entry : darkFrames)
    {
        if (entry.processing)
        {
            entry.processing = false;
            entry.stacked = success;
        }
    }

    if (success)
        lightFramesThread->setMasterDark(filename);

    listener->progressNotification(infos);

    framesMutex.unlock();

    if (success)
        nextStep();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameProcessingStarted(const std::string& filename)
{
    framesMutex.lock();

    std::string internalFilename = getInternalFilename(filename);

    for (auto& entry : infos.lightFrames.entries)
    {
        if (entry.filename == internalFilename)
        {
            entry.processing = true;
            break;
        }
    }

    listener->progressNotification(infos);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameProcessed(const std::string& filename, bool success)
{
    framesMutex.lock();

    std::string internalFilename = getInternalFilename(filename);

    for (auto& entry : infos.lightFrames.entries)
    {
        if (entry.filename == internalFilename)
        {
            if (success)
            {
                entry.calibrated = true;
                entry.processing = false;
                ++infos.lightFrames.nbProcessed;

                auto fullpath = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(filename);

                if (infos.lightFrames.entries[referenceFrame].filename == internalFilename)
                    registrationThread->processReferenceFrame({ fullpath }, luminancyThreshold);
                else
                    registrationThread->processFrames({ fullpath });
            }
            else
            {
                entry.valid = false;
                entry.processing = false;
            }

            break;
        }
    }

    listener->progressNotification(infos);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameRegistrationStarted(const std::string& filename)
{
    framesMutex.lock();

    std::string internalFilename = getInternalFilename(filename);

    for (auto& entry : infos.lightFrames.entries)
    {
        if (entry.filename == internalFilename)
        {
            entry.processing = true;
            break;
        }
    }

    listener->progressNotification(infos);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFrameRegistered(const std::string& filename, bool success)
{
    framesMutex.lock();

    for (auto& entry : infos.lightFrames.entries)
    {
        auto fullpath = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename);

        if (fullpath == filename)
        {
            entry.registered = true;
            entry.processing = false;
            ++infos.lightFrames.nbRegistered;

            if (success)
            {
                ++infos.lightFrames.nbValid;
                stackingThread->processFrames({ fullpath });
            }
            else
            {
                entry.valid = false;
            }

            break;
        }
    }

    listener->progressNotification(infos);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFramesStackingStarted(unsigned int nbFrames)
{
    framesMutex.lock();

    unsigned int nb = 0;

    for (auto& entry : infos.lightFrames.entries)
    {
        if (entry.registered && entry.valid)
        {
            entry.processing = true;
            ++nb;

            if (nb == nbFrames)
                break;
        }
    }

    infos.lightFrames.nbStacking = nbFrames;

    listener->progressNotification(infos);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::lightFramesStacked(const std::string& filename, unsigned int nbFrames)
{
    framesMutex.lock();

    for (auto& entry : infos.lightFrames.entries)
    {
        if (entry.registered && entry.valid && entry.processing)
        {
            entry.stacked = true;
            entry.processing = false;
        }
    }

    infos.lightFrames.nbStacked = infos.lightFrames.nbStacking;
    infos.lightFrames.nbStacking = 0;

    listener->progressNotification(infos);
    listener->stackingDone(filename);

    framesMutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LiveStacking<BITMAP>::nextStep()
{
    framesMutex.lock();

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
        step = STEP_MASTER_DARK;

        std::vector<std::string> allDarkFrames;
        for (auto& entry : darkFrames)
        {
            std::filesystem::path path = getAbsoluteFilename(entry.filename);
            allDarkFrames.push_back(path);
            entry.processing = true;
        }

        framesMutex.unlock();

        masterDarkThread->processFrames(allDarkFrames);
    }
    else if (!infos.lightFrames.entries.empty())
    {
        step = STEP_STACKING;

        auto reference = infos.lightFrames.entries[referenceFrame];
        std::vector<std::string> lightFramesToProcess;
        std::vector<std::string> lightFramesToRegister;
        std::vector<std::string> lightFramesToStack;

        for (auto& entry : infos.lightFrames.entries)
        {
            if (!entry.valid || entry.processing)
                continue;

            if (entry.filename != reference.filename)
            {
                if (!entry.calibrated)
                {
                    std::filesystem::path path = getAbsoluteFilename(entry.filename);
                    lightFramesToProcess.push_back(path);
                }
                else if (!entry.registered)
                {
                    lightFramesToRegister.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
                }
                else if (!entry.stacked)
                {
                    lightFramesToStack.push_back(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(entry.filename));
                }
            }
        }

        framesMutex.unlock();

        if (reference.calibrated)
        {
            FITS fits;
            fits.open(folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(reference.filename));
            lightFramesThread->setParameters(fits.readBackgroundCalibrationParameters());

            if (reference.registered)
            {
                int luminancyThreshold;
                auto stars = fits.readStars(0, nullptr, &luminancyThreshold);
                registrationThread->setParameters(stars, luminancyThreshold);
            }
            else
            {
                registrationThread->processReferenceFrame(
                    folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(reference.filename)
                );
            }
        }
        else
        {
            std::filesystem::path path = getAbsoluteFilename(reference.filename);
            lightFramesThread->processReferenceFrame(path);
        }

        if (reference.registered)
            stackingThread->processFrames({ getAbsoluteFilename(reference.filename) });

        if (!lightFramesToProcess.empty())
            lightFramesThread->processFrames(lightFramesToProcess);

        if (!lightFramesToRegister.empty())
            registrationThread->processFrames(lightFramesToRegister);

        if (!lightFramesToStack.empty())
            stackingThread->processFrames(lightFramesToStack);
    }
    else
    {
        framesMutex.unlock();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string LiveStacking<BITMAP>::getInternalFilename(const std::string& path) const
{
    if (!std::filesystem::path(path).is_absolute())
        return path;

    if (path.starts_with(folder.string()))
        return path.substr(folder.string().size() + 1);

    return path;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string LiveStacking<BITMAP>::getAbsoluteFilename(const std::string& path) const
{
    if (!std::filesystem::path(path).is_absolute())
        return folder / path;

    return path;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string LiveStacking<BITMAP>::getCalibratedFilename(const std::string& path) const
{
    std::string filename = std::filesystem::path(path).filename().string();
    std::string extension = std::filesystem::path(path).extension().string();
    return filename.replace(filename.find(extension), extension.size(), ".fits");
}

}
}
