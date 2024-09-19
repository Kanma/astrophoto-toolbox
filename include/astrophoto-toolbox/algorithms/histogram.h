/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represent a (simple) histogram of values
    //------------------------------------------------------------------------------------
    typedef std::vector<size_t> histogram_t;


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of a grayscale bitmap
    ///
    /// The histogram has 65536 bins.
    ///
    /// The bitmap is converted to grayscale if necessary.
    //------------------------------------------------------------------------------------
    void computeHistogram(Bitmap* bitmap, histogram_t& histogram);

}
