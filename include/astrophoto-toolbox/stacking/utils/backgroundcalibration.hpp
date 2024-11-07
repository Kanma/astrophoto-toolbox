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
namespace utils {

template<class BITMAP>
void BackgroundCalibration<BITMAP>::setReference(BITMAP* bitmap)
{
    computeParameters(bitmap, parameters);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 3)
{
    background_calibration_parameters_t src;

    computeParameters(bitmap, src);

    Interpolation redInterpolation(
        0.0, src.redBackground, src.redMax,
        0.0, parameters.redBackground, parameters.redMax
    );

    Interpolation greenInterpolation(
        0.0, src.greenBackground, src.greenMax,
        0.0, parameters.greenBackground, parameters.greenMax
    );

    Interpolation blueInterpolation(
        0.0, src.blueBackground, src.blueMax,
        0.0, parameters.blueBackground, parameters.blueMax
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
    background_calibration_parameters_t src;

    computeParameters(bitmap, src);

    Interpolation interpolation(
        0.0, src.redBackground, src.redMax,
        0.0, parameters.redBackground, parameters.redMax
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
    const BITMAP* bitmap, background_calibration_parameters_t& parameters
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

    parameters.redMax = findMax(redHistogram) * bitmap->maxRangeValue();
    parameters.greenMax = findMax(greenHistogram) * bitmap->maxRangeValue();
    parameters.blueMax = findMax(blueHistogram) * bitmap->maxRangeValue();

    // Compute the median of each channel
    const auto findMedian = [nbTotalValues = (bitmap->width() * bitmap->height()) / 2](const histogram_t& histogram) -> double
    {
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) / 65535.0;
    };

    parameters.redBackground = findMedian(redHistogram) * bitmap->maxRangeValue();
    parameters.greenBackground = findMedian(greenHistogram) * bitmap->maxRangeValue();
    parameters.blueBackground = findMedian(blueHistogram) * bitmap->maxRangeValue();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BackgroundCalibration<BITMAP>::computeParameters(
    const BITMAP* bitmap, background_calibration_parameters_t& parameters
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

    parameters.redMax = findMax(histogram) * bitmap->maxRangeValue();

    // Compute the median of each channel
    const auto findMedian = [nbTotalValues = (bitmap->width() * bitmap->height()) / 2](const histogram_t& histogram) -> double
    {
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) / 65535.0;
    };

    parameters.redBackground = findMedian(histogram) * bitmap->maxRangeValue();
}

}
}
}
