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

#include <astrophoto-toolbox/stacking/processing/utils.h>

namespace astrophototoolbox {
namespace stacking {
namespace processing {


template<class BITMAP>
BITMAP* MasterDarkGenerator<BITMAP>::compute(
    const std::vector<std::string>& darkFrames, const std::string& destination,
    const std::filesystem::path& tmpFolder
)
{
    if (std::filesystem::exists(destination))
        std::filesystem::remove(destination);

    if (darkFrames.empty())
        return nullptr;

    stacker.setup(darkFrames.size(), tmpFolder);

    for (const auto& filename : darkFrames)
    {
        if (!std::filesystem::exists(filename))
            continue;

        auto* bitmap = loadProcessedBitmap<BITMAP>(filename);
        if (bitmap)
        {
            stacker.addBitmap(bitmap);
            delete bitmap;
        }
    }

    if (stacker.nbStackedBitmaps() == 0)
    {
        stacker.clear();
        return nullptr;
    }

    BITMAP* masterDark = stacker.process();
    stacker.clear();

    std::filesystem::remove(tmpFolder);

    if (!masterDark)
        return nullptr;

    detectHotPixels(masterDark);

    if (!destination.empty() && !saveProcessedBitmap(masterDark, destination, &hotPixels))
    {
        delete masterDark;
        return nullptr;
    }

    return masterDark;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkGenerator<BITMAP>::cancel()
{
    stacker.cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkGenerator<BITMAP>::detectHotPixels(BITMAP* masterDark)
    requires(BITMAP::Channels == 3)
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
void MasterDarkGenerator<BITMAP>::detectHotPixels(BITMAP* masterDark)
    requires(BITMAP::Channels == 1)
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

}
}
}
