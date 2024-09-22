/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/point.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Find the center of the star in a bitmap where a Bahtinov mask is used
    ///
    /// The center and the radius are roughly estimated using the maximum luminancy values
    /// in the image, which is assumed to be mostly black with a group of luminous pixels
    /// around the star.
    //------------------------------------------------------------------------------------
    point_t findStarInBitmapWithBahtinovMask(Bitmap* bitmap, double* radius = nullptr);

}
