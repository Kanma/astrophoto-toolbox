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

#include <astrophoto-toolbox/stacking/backgroundcalibration.h>
#include <astrophoto-toolbox/algorithms/histogram.h>
#include <astrophoto-toolbox/algorithms/interpolation.h>
#include <astrophoto-toolbox/images/helpers.h>

using namespace astrophototoolbox;
using namespace stacking;


void BackgroundCalibration::setReference(DoubleColorBitmap* bitmap)
{
    computeParameters(
        bitmap,
        targetRedBackground, targetGreenBackground, targetBlueBackground,
        targetRedMax, targetGreenMax, targetBlueMax
    );
}

//-----------------------------------------------------------------------------

void BackgroundCalibration::calibrate(DoubleColorBitmap* bitmap) const
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
        double* data = bitmap->data(y);

        for (unsigned int x = 0, i = 0; x < bitmap->width(); ++x, i += 3)
        {
            data[i] = redInterpolation.interpolate(data[i]);
            data[i + 1] = greenInterpolation.interpolate(data[i + 1]);
            data[i + 2] = blueInterpolation.interpolate(data[i + 2]);
        }
    }
}

//-----------------------------------------------------------------------------

void BackgroundCalibration::computeParameters(
    DoubleColorBitmap* bitmap,
    double& redBackground, double& greenBackground, double& blueBackground,
    double& redMax, double& greenMax, double& blueMax
) const
{
    DoubleColorBitmap* converted = requiresFormat<DoubleColorBitmap>(
        bitmap, RANGE_ONE, SPACE_SOURCE
    );

    // Compute one histogram per channel
    histogram_t redHistogram;
    histogram_t greenHistogram;
    histogram_t blueHistogram;

    Bitmap* redChannel = converted->channel(0);
    computeHistogram(redChannel, redHistogram);

    Bitmap* greenChannel = converted->channel(1);
    computeHistogram(greenChannel, greenHistogram);

    Bitmap* blueChannel = converted->channel(2);
    computeHistogram(blueChannel, blueHistogram);

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

	redMax = findMax(redHistogram);
	greenMax = findMax(greenHistogram);
	blueMax = findMax(blueHistogram);

    // Compute the median of each channel
	const auto findMedian = [nbTotalValues = (bitmap->width() * bitmap->height()) / 2](const histogram_t& histogram) -> double
	{
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) / 65535.0;
	};

	redBackground = findMedian(redHistogram);
	greenBackground = findMedian(greenHistogram);
	blueBackground = findMedian(blueHistogram);

    delete redChannel;
    delete greenChannel;
    delete blueChannel;

    if (converted != bitmap)
        delete converted;
}
