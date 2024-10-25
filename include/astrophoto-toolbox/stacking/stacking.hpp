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

#include <astrophoto-toolbox/stacking/bitmapstacker.h>
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/stacking/backgroundcalibration.h>
#include <astrophoto-toolbox/stacking/starmatcher.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <sstream>
#include <fstream>

namespace astrophototoolbox {
namespace stacking {


static const char* CONFIG_FILE = "stacking.txt";
static const char* MASTER_DARK = "master_dark.fits";


//-----------------------------------------------------------------------------

template<class BITMAP>
Stacking<BITMAP>::~Stacking()
{
    clear();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::setup(const std::filesystem::path& folder)
{
    clear();

    this->folder = folder;

    referenceFrame = -1;
    lightFramesCalibrated = false;
    nbLightFramesCalibrated = 0;
    nbLightFramesUnusable = 0;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::load()
{
    loading = true;

    clear();

    darkFrames.clear();
    lightFrames.clear();

    std::ifstream input(folder / CONFIG_FILE, std::ios::in);
    if (!input.is_open())
        return false;

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

    loading = false;

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::save() const
{
    std::filesystem::create_directory(folder);

    std::ofstream output(folder / CONFIG_FILE, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        return false;

    if (!darkFrames.empty())
    {
        output << "DARKFRAMES" << std::endl;

        for (const auto& filename : darkFrames)
            output << filename << std::endl;

        output << "---" << std::endl;
    }

    if (!lightFrames.empty())
    {
        output << "LIGHTFRAMES" << std::endl;

        for (const auto& filename : lightFrames)
            output << filename << std::endl;

        output << "REF " << referenceFrame << std::endl;
        output << "---" << std::endl;
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::addDarkFrame(const std::string& filename)
{
    if (!loading)
        invalidateDarks();

    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(folder / filename))
        return false;

    darkFrames.push_back(filename);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::computeMasterDark()
{
    cancelled = false;

    clearMasterDark();

    if (darkFrames.empty())
        return false;

    invalidateLights();

    if (darkFrames.size() > 1)
    {
        BitmapStacker<BITMAP> stacker;
        stacker.setup(darkFrames.size(), folder / "tmp");

        for (const auto& filename : darkFrames)
        {
            auto* bitmap = loadBitmap(filename);
            if (bitmap)
            {
                stacker.addBitmap(bitmap);
                delete bitmap;
            }
        }

        cancelFunction = [&stacker]() { stacker.cancel(); };
        masterDark = stacker.process();
        cancelFunction = nullptr;
    }
    else
    {
        masterDark = loadBitmap(darkFrames[0]);
    }

    std::filesystem::remove(folder / "tmp");

    if (!masterDark)
        return false;

    detectHotPixels();

    if (!saveBitmap(masterDark, folder / MASTER_DARK, &hotPixels))
    {
        clearMasterDark();
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::hasMasterDark() const
{
    return (masterDark || std::filesystem::exists(folder / MASTER_DARK));
}

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* Stacking<BITMAP>::getMasterDark()
{
    if (!masterDark && std::filesystem::exists(folder / MASTER_DARK))
    {
        masterDark = loadBitmap(folder / MASTER_DARK, &hotPixels);
        if (masterDark && hotPixels.empty())
            detectHotPixels();
    }

    return masterDark;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::addLightFrame(const std::string& filename, bool reference)
{
    std::filesystem::path path(filename);
    if (!path.is_absolute())
        path = folder / filename;

    if (!std::filesystem::exists(folder / filename))
        return false;

    lightFrames.push_back(filename);

    if (reference)
        referenceFrame = lightFrames.size() - 1;

    lightFramesCalibrated = false;

    if (std::filesystem::exists(folder / "calibrated" / "lights" / getCalibratedFilename(filename)))
        ++nbLightFramesCalibrated;

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::processLightFrames()
{
    cancelled = false;

    if (lightFrames.empty())
        return false;

    if ((referenceFrame < 0) || (referenceFrame >= lightFrames.size()))
        return false;

    if (!masterDark)
    {
        if (hasMasterDark())
        {
            getMasterDark();
        }
        else if (!darkFrames.empty())
        {
            if (!computeMasterDark())
                return false;
        }
    }

    std::filesystem::path path = folder / "calibrated" / "lights";
    std::filesystem::create_directories(path);

    Registration registration;
    BackgroundCalibration<BITMAP> calibration;
    StarMatcher matcher;
    Transformation transformation;

    BITMAP* bitmap = nullptr;
    star_list_t refStars;

    if (std::filesystem::exists(path / getCalibratedFilename(lightFrames[referenceFrame])))
    {
        bitmap = loadBitmap(path / getCalibratedFilename(lightFrames[referenceFrame]), nullptr, &refStars);
        calibration.setReference(bitmap);
    }
    else
    {
        bitmap = loadBitmap(lightFrames[referenceFrame]);

        if (masterDark)
        {
            substract(bitmap, masterDark);
            removeHotPixels(bitmap);
        }

        calibration.setReference(bitmap);

        refStars = registration.registerBitmap(bitmap, -1);
        luminancyThreshold = registration.getLuminancyThreshold();

        saveBitmap(bitmap, path / getCalibratedFilename(lightFrames[referenceFrame]), nullptr, &refStars);
    }

    delete bitmap;

    if (cancelled)
        return false;

    for (size_t i = 0; i < lightFrames.size(); ++i)
    {
        if (i == referenceFrame)
            continue;

        if (std::filesystem::exists(path / getCalibratedFilename(lightFrames[i])))
            continue;

        bitmap = loadBitmap(lightFrames[i]);

        if (masterDark)
        {
            substract(bitmap, masterDark);
            removeHotPixels(bitmap);
        }

        calibration.calibrate(bitmap);

        star_list_t stars = registration.registerBitmap(bitmap, luminancyThreshold);

        bool valid = matcher.computeTransformation(
            stars, refStars, size2d_t(bitmap->width(), bitmap->height()),
            transformation
        );

        saveBitmap(
            bitmap, path / getCalibratedFilename(lightFrames[i]), nullptr, &stars,
            valid ? &transformation : nullptr
        );

        delete bitmap;

        if (cancelled)
        {
            lightFramesCalibrated = (i == lightFrames.size() - 1);
            nbLightFramesCalibrated = i;
            return lightFramesCalibrated;
        }
    }

    lightFramesCalibrated = true;
    nbLightFramesCalibrated = lightFrames.size();

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* Stacking<BITMAP>::process(unsigned int nbExpectedLightFrames)
{
    cancelled = false;

    if (!lightFramesCalibrated)
    {
        if (!processLightFrames())
            return nullptr;
    }

    std::filesystem::path path = folder / "calibrated" / "lights";

    if (nbLightFramesCalibrated == 1)
        return loadBitmap(path / getCalibratedFilename(lightFrames[0]));

    if (!stacker.isInitialised())
    {
        stacker.setup(
            (nbExpectedLightFrames == 0 ? lightFrames.size() : nbExpectedLightFrames),
            folder / "tmp"
        );

        outputRect = rect_t(
            0, 0, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()
        );
    }

    Transformation transformation;
    size_t nbStackedFrames = stacker.nbStackedBitmaps();

    for (auto it = lightFrames.begin() + stacker.nbStackedBitmaps() + nbLightFramesUnusable;
         it != lightFrames.end(); ++it)
    {
        const auto filename = *it;

        auto* bitmap = loadBitmap(
            path / getCalibratedFilename(filename), nullptr, nullptr, &transformation
        );

        if (bitmap)
        {
            rect_t transformedRect = transformation.transform(
                rect_t{ 0, 0, (int) bitmap->width(), (int) bitmap->height() }
            );

            if ((transformedRect.left < transformedRect.right) &&
                (transformedRect.top < transformedRect.bottom))
            {
                auto* transformed = transformation.transform(bitmap);
                stacker.addBitmap(transformed);

                outputRect = outputRect.intersection(transformedRect);

                delete transformed;
            }
            else
            {
                ++nbLightFramesUnusable;
            }

            delete bitmap;
        }

        if (cancelled)
            return nullptr;
    }

    std::filesystem::path resultPath = folder / "stacked.fits";

    if (nbStackedFrames < stacker.nbStackedBitmaps())
    {
        BITMAP* stacked = stacker.process();
        if (!stacked)
            return nullptr;

        BITMAP* result = new BITMAP(outputRect.width(), outputRect.height());
        for (unsigned int y = 0; y < result->height(); ++y)
        {
            typename BITMAP::type_t* src = stacked->data(outputRect.left, outputRect.top + y);
            typename BITMAP::type_t* dest = result->data(y);

            memcpy(dest, src, result->bytesPerRow());
        }

        delete stacked;

        if (std::filesystem::exists(resultPath))
            std::filesystem::remove(resultPath);

        saveBitmap(result, resultPath);

        return result;
    }

    return loadBitmap(resultPath);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::cancel()
{
    cancelled = true;
    stacker.cancel();

    if (cancelFunction)
        cancelFunction();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::detectHotPixels() requires(BITMAP::Channels == 3)
{
    const double hotFactor = 4.0;

    hotPixels.clear();

    const auto computeThreshold = [](BITMAP* bitmap, unsigned int channelIndex) -> double
    {
        double median = computeMedian(bitmap, channelIndex);
        double average;
        double sigma = computeStandardDeviation(bitmap, average, channelIndex);
        return median + 16.0 * sigma;
    };

    double redThreshold = computeThreshold(masterDark, 0);
    double greenThreshold = computeThreshold(masterDark, 1);
    double blueThreshold = computeThreshold(masterDark, 2);

    typename BITMAP::type_t* data = masterDark->data();
    unsigned int rowSize = masterDark->bytesPerRow() / masterDark->channelSize();

    for (unsigned int y = 1; y < masterDark->height() - 1; ++y)
    {
        for (unsigned int x = 1; x < masterDark->width() - 1; ++x)
        {
            size_t offset = y * rowSize + x * 3;

            if ((data[offset] > redThreshold) || (data[offset+1] > greenThreshold) || (data[offset+2] > blueThreshold))
                hotPixels.push_back(point_t{ double(x), double(y) });
        }
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::detectHotPixels() requires(BITMAP::Channels == 1)
{
    const double hotFactor = 4.0;

    hotPixels.clear();

    const auto computeThreshold = [](BITMAP* bitmap) -> double
    {
        double median = computeMedian(bitmap);
        double average;
        double sigma = computeStandardDeviation(bitmap, average);
        return median + 16.0 * sigma;
    };

    double threshold = computeThreshold(masterDark);

    typename BITMAP::type_t* data = masterDark->data();
    unsigned int rowSize = masterDark->bytesPerRow() / masterDark->channelSize();

    for (unsigned int y = 1; y < masterDark->height() - 1; ++y)
    {
        for (unsigned int x = 1; x < masterDark->width() - 1; ++x)
        {
            size_t offset = y * rowSize + x * 3;

            if (data[offset] > threshold)
                hotPixels.push_back(point_t{ double(x), double(y) });
        }
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::removeHotPixels(BITMAP* bitmap) const requires(BITMAP::Channels == 3)
{
    typename BITMAP::type_t* data = bitmap->data();

    const unsigned int rowOffset = bitmap->width() * 3;

    for (const auto& point : hotPixels)
    {
        unsigned int x = (unsigned int) point.x;
        unsigned int y = (unsigned int) point.y;

        data[y * rowOffset + x * 3] = 0.0;
        data[y * rowOffset + x * 3 + 1] = 0.0;
        data[y * rowOffset + x * 3 + 2] = 0.0;
    }

    const auto interpolate = [rowOffset](typename BITMAP::type_t* ptr)
    {
        double sum = (*(ptr - 3) + *(ptr + 3) + *(ptr - rowOffset) + *(ptr + rowOffset)) * 3 +
                     (*(ptr - rowOffset - 3) + *(ptr - rowOffset + 3) +
                      *(ptr + rowOffset - 3) + *(ptr + rowOffset + 3)) * 2;

        *ptr = sum / 20.0;
    };

    for (const auto& point : hotPixels)
    {
        unsigned int x = (unsigned int) point.x;
        unsigned int y = (unsigned int) point.y;

        interpolate(&data[y * rowOffset + x * 3]);
        interpolate(&data[y * rowOffset + x * 3 + 1]);
        interpolate(&data[y * rowOffset + x * 3 + 2]);
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::removeHotPixels(BITMAP* bitmap) const requires(BITMAP::Channels == 1)
{
    typename BITMAP::type_t* data = bitmap->data();

    const unsigned int rowOffset = bitmap->width();

    for (const auto& point : hotPixels)
    {
        unsigned int x = (unsigned int) point.x;
        unsigned int y = (unsigned int) point.y;

        data[y * rowOffset + x] = 0.0;
        data[y * rowOffset + x + 1] = 0.0;
        data[y * rowOffset + x + 2] = 0.0;
    }

    const auto interpolate = [rowOffset](typename BITMAP::type_t* ptr)
    {
        double sum = (*(ptr - 1) + *(ptr + 1) + *(ptr - rowOffset) + *(ptr + rowOffset)) * 3 +
                     (*(ptr - rowOffset - 1) + *(ptr - rowOffset + 1) +
                      *(ptr + rowOffset - 1) + *(ptr + rowOffset + 1)) * 2;

        *ptr = sum / 20.0;
    };

    for (const auto& point : hotPixels)
    {
        unsigned int x = (unsigned int) point.x;
        unsigned int y = (unsigned int) point.y;

        interpolate(&data[y * rowOffset + x]);
        interpolate(&data[y * rowOffset + x + 1]);
        interpolate(&data[y * rowOffset + x + 2]);
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::clear()
{
    delete masterDark;
    masterDark = nullptr;

    hotPixels.clear();
    stacker.clear();

    lightFramesCalibrated = false;
    nbLightFramesCalibrated = 0;
    nbLightFramesUnusable = 0;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::clearMasterDark()
{
    clear();
    std::filesystem::remove(folder / MASTER_DARK);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::invalidateDarks()
{
    clearMasterDark();
    invalidateLights();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void Stacking<BITMAP>::invalidateLights()
{
    std::filesystem::path path = folder / "calibrated" / "lights";

    for (const auto& filename : lightFrames)
        std::filesystem::remove(path / getCalibratedFilename(filename));

    stacker.clear();

    lightFramesCalibrated = false;
    nbLightFramesCalibrated = 0;
    nbLightFramesUnusable = 0;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
const std::string Stacking<BITMAP>::getCalibratedFilename(const std::string& path)
{
    std::string filename = std::filesystem::path(path).filename().string();
    std::string extension = std::filesystem::path(path).extension().string();
    return filename.replace(filename.find(extension), extension.size(), ".fits");
};

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* Stacking<BITMAP>::loadBitmap(
    const std::filesystem::path& path, point_list_t* hotPixels, star_list_t* stars,
    Transformation* transformation
)
{
    Bitmap* bitmap = nullptr;

    std::filesystem::path filename = path;
    if (!filename.is_absolute())
        filename = folder / filename;

    if (filename.extension() == ".fits")
    {
        FITS fits;
        if (fits.open(filename))
        {
            bitmap = fits.readBitmap();

            if (hotPixels)
                *hotPixels = fits.readPoints("HOTPIXELS");

            if (stars)
                *stars = fits.readStars("STARS");

            if (transformation)
                *transformation = fits.readTransformation("TRANSFORMS");
        }
    }
    else
    {
        if (rawImage.open(filename))
        {
            if (rawImage.channels() == 3)
                bitmap = new UInt16ColorBitmap();
            else
                bitmap = new UInt16GrayBitmap();

            if (!rawImage.toBitmap(bitmap, true))
            {
                delete bitmap;
                bitmap = nullptr;
            }
        }

        if (hotPixels)
            hotPixels->clear();

        if (stars)
            stars->clear();

        if (transformation)
            *transformation = Transformation();
    }

    if (bitmap)
    {
        BITMAP* output = requiresFormat<BITMAP>(bitmap, RANGE_DEST, SPACE_SOURCE);
        if (output != bitmap)
            delete bitmap;

        return output;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool Stacking<BITMAP>::saveBitmap(
    BITMAP* bitmap, const std::filesystem::path& path, point_list_t* hotPixels,
    star_list_t* stars, Transformation* transformation
) const
{
    FITS fits;
    if (!fits.create(path))
        return false;

    if (!fits.write(bitmap))
        return false;

    if (hotPixels && !fits.write(*hotPixels, "HOTPIXELS"))
        return false;

    if (stars && !fits.write(*stars, size2d_t(bitmap->width(), bitmap->height()), "STARS"))
        return false;

    if (transformation && !fits.write(*transformation, "TRANSFORMS"))
        return false;

    return true;
}

}
}
