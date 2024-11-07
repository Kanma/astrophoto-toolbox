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
#include <astrophoto-toolbox/images/helpers.h>

namespace astrophototoolbox {
namespace stacking {
namespace processing {


template<class BITMAP>
bool LightFrameProcessor<BITMAP>::setMasterDark(const std::string& filename)
{
    masterDark.reset();

    BITMAP* bitmap = loadProcessedBitmap<BITMAP>(filename, &hotPixels);
    if (!bitmap)
        return false;

    masterDark.reset(bitmap);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameProcessor<BITMAP>::setParameters(
    const utils::background_calibration_parameters_t& parameters
)
{
    calibration.setParameters(parameters);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
std::shared_ptr<BITMAP> LightFrameProcessor<BITMAP>::process(
    const std::string& lightFrame, bool reference, const std::string& destination
)
{
    BITMAP* bitmap = loadProcessedBitmap<BITMAP>(lightFrame);
    if (!bitmap)
        return nullptr;

    return process(std::make_shared<BITMAP>(bitmap), reference, destination);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
std::shared_ptr<BITMAP> LightFrameProcessor<BITMAP>::process(
    const std::shared_ptr<BITMAP>& lightFrame, bool reference, const std::string& destination
)
{
    if (masterDark)
    {
        substract(lightFrame.get(), masterDark.get());
        removeHotPixels(lightFrame);
    }

    if (reference)
        calibration.setReference(lightFrame.get());
    else
        calibration.calibrate(lightFrame.get());

    if (!destination.empty())
    {
        auto bgcalibParameters = calibration.getParameters();

        if (!saveProcessedBitmap(lightFrame.get(), destination, nullptr, nullptr, nullptr,
                                 reference ? &bgcalibParameters : nullptr))
        {
            return nullptr;
        }
    }

    return lightFrame;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameProcessor<BITMAP>::removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const
    requires(BITMAP::Channels == 3)
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
void LightFrameProcessor<BITMAP>::removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const
    requires(BITMAP::Channels == 1)
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

}
}
}
