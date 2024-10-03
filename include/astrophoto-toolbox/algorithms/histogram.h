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


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of a vector of values
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    template<typename T>
    inline void computeHistogram(
        const std::vector<T>& values, histogram_t& histogram, T maxValue = T(0)
    )
    {
        histogram.resize(size_t(std::numeric_limits<uint16_t>::max()) + 1);
        memset(histogram.data(), 0, histogram.size() * sizeof(uint16_t));

        if (maxValue == T(0))
            maxValue = *std::max_element(values.begin(), values.end());

        const T* ptr = values.data();
        for (unsigned int i = 0; i < values.size(); ++i)
        {
            ++histogram[uint16_t(double(*ptr) / maxValue * 65535.0)];
            ++ptr;
        }
    }

}
