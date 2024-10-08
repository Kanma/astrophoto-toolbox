/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <map>
#include <cstring>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represent a (simple) histogram of values
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    typedef std::vector<size_t> histogram_t;

    typedef std::map<uint16_t, size_t> sparse_histogram_t;


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of an array of values
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    template<typename T>
    inline void computeHistogram(
        const T* values, size_t count, histogram_t& histogram, T maxValue = T(0),
        size_t offset = 1
    )
    {
        histogram.resize(size_t(std::numeric_limits<uint16_t>::max()) + 1);
        std::memset(histogram.data(), 0, histogram.size() * sizeof(size_t));

        if (maxValue == T(0))
        {
            for (size_t i = 0; i < count * offset; i += offset)
                maxValue = std::max(values[i], maxValue);
        }

        for (unsigned int i = 0; i < count; ++i)
        {
            ++histogram[uint16_t(double(*values) / maxValue * 65535.0)];
            values += offset;
        }
    }


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of an array of values
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    template<typename T>
    inline void computeHistogram(
        const T* values, size_t count, sparse_histogram_t& histogram, T maxValue = T(0),
        size_t offset = 1
    )
    {
        histogram.clear();

        if (maxValue == T(0))
        {
            for (size_t i = 0; i < count * offset; i += offset)
                maxValue = std::max(values[i], maxValue);
        }

        for (unsigned int i = 0; i < count; ++i)
        {
            uint16_t index = uint16_t(double(*values) / maxValue * 65535.0);

            if (histogram.contains(index))
                ++histogram[index];
            else
                histogram[index] = 1;

            values += offset;
        }
    }


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
        computeHistogram(values.data(), values.size(), histogram, maxValue);
    }


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of a vector of values
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    template<typename T>
    inline void computeHistogram(
        const std::vector<T>& values, sparse_histogram_t& histogram, T maxValue = T(0)
    )
    {
        computeHistogram(values.data(), values.size(), histogram, maxValue);
    }


    //------------------------------------------------------------------------------------
    /// @brief  Compute the histogram of one channel of a grayscale bitmap
    ///
    /// The histogram has 65536 bins.
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    inline void computeHistogram(
        const BITMAP* bitmap, histogram_t& histogram, size_t channel = 0
    )
    {
        return computeHistogram(
            ((typename BITMAP::type_t*) bitmap->ptr()) + channel,
            bitmap->width() * bitmap->height(), histogram,
            (typename BITMAP::type_t) bitmap->maxRangeValue(),
            BITMAP::Channels
        );
    }

}
