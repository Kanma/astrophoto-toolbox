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

#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/images/io.h>
#include <sstream>
#include <fstream>

namespace astrophototoolbox {
namespace stacking {

template<class BITMAP>
BITMAP* loadProcessedBitmap(
    const std::filesystem::path& filename, point_list_t* hotPixels, star_list_t* stars,
    Transformation* transformation
)
{
    Bitmap* bitmap = nullptr;

    if (FITS::isFITS(filename))
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
        bitmap = io::load(filename, true);

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
bool saveProcessedBitmap(
    BITMAP* bitmap, const std::filesystem::path& path, point_list_t* hotPixels,
    star_list_t* stars, Transformation* transformation
)
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
