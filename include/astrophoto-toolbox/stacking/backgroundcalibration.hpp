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

#include <astrophoto-toolbox/algorithms/histogram.h>
#include <astrophoto-toolbox/algorithms/interpolation.h>


namespace astrophototoolbox {
namespace stacking {

template<class BITMAP>
void BackgroundCalibration<BITMAP>::setReference(BITMAP* bitmap) requires(BITMAP::Channels == 3)
{
    computeParameters(
        bitmap,
        targetRedBackground, targetGreenBackground, targetBlueBackground,
        targetRedMax, targetGreenMax, targetBlueMax
    );
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::setReference(BITMAP* bitmap) requires(BITMAP::Channels == 1)
{
    computeParameters(bitmap, targetRedBackground, targetRedMax);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 3)
{
    double srcRedBackground;
    double srcGreenBackground;
    double srcBlueBackground;

    double srcRedMax;
    double srcGreenMax;
    double srcBlueMax;

    computeParameters(
        bitmap,
        srcRedBackground, srcGreenBackground, srcBlueBackground,
        srcRedMax, srcGreenMax, srcBlueMax
    );

    Interpolation redInterpolation(
        0.0, srcRedBackground, srcRedMax,
        0.0, targetRedBackground, targetRedMax
    );

    Interpolation greenInterpolation(
        0.0, srcGreenBackground, srcGreenMax,
        0.0, targetGreenBackground, targetGreenMax
    );

    Interpolation blueInterpolation(
        0.0, srcBlueBackground, srcBlueMax,
        0.0, targetBlueBackground, targetBlueMax
    );

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        typename BITMAP::type_t* data = bitmap->data(y);

        for (unsigned int x = 0, i = 0; x < bitmap->width(); ++x, i += 3)
        {
            data[i] = redInterpolation.interpolate(data[i]);
            data[i + 1] = greenInterpolation.interpolate(data[i + 1]);
            data[i + 2] = blueInterpolation.interpolate(data[i + 2]);
        }
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 1)
{
    double srcBackground;
    double srcMax;

    computeParameters(bitmap, srcBackground, srcMax);

    Interpolation interpolation(
        0.0, srcBackground, srcMax,
        0.0, targetRedBackground, targetRedMax
    );

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        typename BITMAP::type_t* data = bitmap->data(y);

        for (unsigned int x = 0; x < bitmap->width(); ++x)
            data[x] = interpolation.interpolate(data[x]);
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::computeParameters(
    const BITMAP* bitmap,
    double& redBackground, double& greenBackground, double& blueBackground,
    double& redMax, double& greenMax, double& blueMax
) const requires(BITMAP::Channels == 3)
{
    // Compute one histogram per channel
    histogram_t redHistogram;
    histogram_t greenHistogram;
    histogram_t blueHistogram;

    computeHistogram(bitmap, redHistogram, 0);
    computeHistogram(bitmap, greenHistogram, 1);
    computeHistogram(bitmap, blueHistogram, 2);

    // Retrieve the maximum value of each channel
    const auto findMax = [](const histogram_t& histogram) -> double
    {
        size_t i = histogram.size() - 1;
        for (auto it = histogram.crbegin(); it != histogram.crend(); ++it, --i)
        {
            if (*it != 0)
                return static_cast<double>(i) / 65535.0;
        }

        return 0.0;
    };

    redMax = findMax(redHistogram) * bitmap->maxRangeValue();
    greenMax = findMax(greenHistogram) * bitmap->maxRangeValue();
    blueMax = findMax(blueHistogram) * bitmap->maxRangeValue();

    // Compute the median of each channel
    const auto findMedian = [nbTotalValues = (bitmap->width() * bitmap->height()) / 2](const histogram_t& histogram) -> double
    {
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) / 65535.0;
    };

    redBackground = findMedian(redHistogram) * bitmap->maxRangeValue();
    greenBackground = findMedian(greenHistogram) * bitmap->maxRangeValue();
    blueBackground = findMedian(blueHistogram) * bitmap->maxRangeValue();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::computeParameters(
    const BITMAP* bitmap, double& background, double& max
) const requires(BITMAP::Channels == 1)
{
    // Compute the histogram
    histogram_t histogram;

    computeHistogram(bitmap, histogram);

    // Retrieve the maximum value of the channel
    const auto findMax = [](const histogram_t& histogram) -> double
    {
        size_t i = histogram.size() - 1;
        for (auto it = histogram.crbegin(); it != histogram.crend(); ++it, --i)
        {
            if (*it != 0)
                return static_cast<double>(i) / 65535.0;
        }

        return 0.0;
    };

    max = findMax(histogram) * bitmap->maxRangeValue();

    // Compute the median of each channel
    const auto findMedian = [nbTotalValues = (bitmap->width() * bitmap->height()) / 2](const histogram_t& histogram) -> double
    {
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) / 65535.0;
    };

    background = findMedian(histogram) * bitmap->maxRangeValue();
}

}
}
