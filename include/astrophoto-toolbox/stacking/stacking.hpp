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
void Stacking<BITMAP>::setup(const std::filesystem::path& folder)
{
    this->folder = folder;

    referenceFrame = -1;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::load()
{
    std::ifstream input(folder / CONFIG_FILE, std::ios::in);
    if (!input.is_open())
        return false;

    darkFrames.clear();
    lightFrames.clear();

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

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::save()
{
    std::filesystem::create_directory(folder);

    std::ofstream output(folder / CONFIG_FILE, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        return false;

    if (!darkFrames.empty())
    {
        output << "DARKFRAMES" << std::endl;

        for (const auto& filename : darkFrames)
            output << filename.string() << std::endl;

        output << "---" << std::endl;
    }

    if (!lightFrames.empty())
    {
        output << "LIGHTFRAMES" << std::endl;

        for (const auto& filename : lightFrames)
            output << filename.string() << std::endl;

        output << "REF " << referenceFrame << std::endl;
        output << "---" << std::endl;
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::addDarkFrame(const std::filesystem::path& filename)
{
    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(path))
        return false;

    darkFrames.push_back(filename);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::addLightFrame(const std::filesystem::path& filename, bool reference)
{
    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(path))
        return false;

    lightFrames.push_back(filename);

    if (reference || (lightFrames.size() == 1))
        referenceFrame = lightFrames.size() - 1;

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* Stacking<BITMAP>::process(int luminancyThreshold)
{
    std::filesystem::create_directories(folder);

    BITMAP* masterDark = nullptr;
    point_list_t hotPixels;

    if (!darkFrames.empty())
    {
        masterDark = masterDarkGenerator.compute(
            darkFrames, folder / MASTER_DARK, folder / MASTER_DARK_TEMP_PATH
        );

        if (masterDark)
            hotPixels = masterDarkGenerator.getHotPixels();
    }

    if (masterDark)
        lightFrameProcessor.setMasterDark(std::make_shared<BITMAP>(masterDark), hotPixels);

    if (lightFrames.empty())
        return nullptr;

    std::filesystem::create_directories(folder / CALIBRATED_LIGHT_FRAMES_PATH);

    lightFrameProcessor.process(
        lightFrames[referenceFrame], true,
        folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(lightFrames[referenceFrame])
    );

    for (unsigned int i = 0; i < lightFrames.size(); ++i)
    {
        if (i == referenceFrame)
            continue;

        lightFrameProcessor.process(
            lightFrames[i], false,
            folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(lightFrames[i])
        );
    }

    auto filename = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(lightFrames[referenceFrame]);

    auto stars = registrationProcessor.processReference(filename, luminancyThreshold, filename);
    if (stars.empty())
        return nullptr;

    std::vector<std::filesystem::path> toStack;
    toStack.push_back(filename);

    for (unsigned int i = 0; i < lightFrames.size(); ++i)
    {
        if (i == referenceFrame)
            continue;

        auto filename = folder / CALIBRATED_LIGHT_FRAMES_PATH / getCalibratedFilename(lightFrames[i]);

        auto result = registrationProcessor.process(filename, filename);
        if (!get<0>(result).empty())
            toStack.push_back(filename);
    }

    framesStacker.setup(toStack.size(), folder / STACKING_TEMP_PATH);
    for (const auto& filename : toStack)
        framesStacker.addFrame(filename);

    return framesStacker.process(folder / STACKED_FILE);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string Stacking<BITMAP>::getCalibratedFilename(const std::filesystem::path& path)
{
    std::string filename = path.filename().string();
    std::string extension = path.extension().string();
    return filename.replace(filename.find(extension), extension.size(), ".fits");
}

}
}
