/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/algorithms/histogram.h>
#include <numeric>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Compute the average of an array of values
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeAverage(const T* values, size_t count, size_t offset = 1)
    {
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i)
        {
            sum += *values;
            values += offset;
        }

        return sum / count;
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the average of a vector of values
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeAverage(const std::vector<T>& values)
    {
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the standard deviation of an array of values
    ///
    /// The average is also returned (as a parameter).
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeStandardDeviation(
        const T* values, size_t count, double& average, size_t offset = 1
    )
    {
        double squareDiff = 0.0;

        // Compute the average
        average = computeAverage(values, count, offset);

        for (size_t i = 0; i < count; ++i)
        {
            squareDiff += (*values - average) * (*values - average);
            values += offset;
        }

        return sqrt(squareDiff / count);
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the standard deviation of a vector of values
    ///
    /// The average is also returned (as a parameter).
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeStandardDeviation(const std::vector<T>& values, double& average)
    {
        return computeStandardDeviation(values.data(), values.size(), average);
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of an array of values, using an existing histogram to
    ///         cache values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(
        const T* values, size_t count, histogram_t& histogram, T maxValue = T(0),
        size_t offset = 1
    )
    {
        if (maxValue == T(0))
        {
            for (size_t i = 0; i < count; ++i)
                maxValue = std::max(values[i], maxValue);
        }

        // Compute the histogram of the values
        computeHistogram(values, count, histogram, maxValue, offset);

        // Compute the median
        const size_t nbTotalValues = count / 2 + count % 2;
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return double(index) * maxValue / 65535.0;
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of an array of values, using an existing histogram to
    ///         cache values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(
        const T* values, size_t count, sparse_histogram_t& histogram, T maxValue = T(0),
        size_t offset = 1
    )
    {
        if (maxValue == T(0))
        {
            for (size_t i = 0; i < count; ++i)
                maxValue = std::max(values[i], maxValue);
        }

        // Compute the histogram of the values
        computeHistogram(values, count, histogram, maxValue, offset);

        // Compute the median
        const size_t nbTotalValues = count / 2 + count % 2;
        size_t nbValues = 0;
        size_t index = 0;
        for (auto it = histogram.cbegin(); (nbValues < nbTotalValues) && (it != histogram.end()); ++it)
        {
            nbValues += it->second;
            index = it->first;
        }

        return double(index) * maxValue / 65535.0;
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of an array of values, using an existing histogram to
    ///         cache values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(
        const T* values, size_t count, T maxValue = T(0), size_t offset = 1
    )
    {
        sparse_histogram_t histogram;
        return computeMedian(values, count, histogram, maxValue, offset);
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of a vector of values, using an existing histogram to
    ///         cache values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(
        const std::vector<T>& values, histogram_t& histogram, T maxValue = T(0)
    )
    {
        return computeMedian(values.data(), values.size(), histogram, maxValue);
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of a vector of values, using an existing histogram to
    ///         cache values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(
        const std::vector<T>& values, sparse_histogram_t& histogram, T maxValue = T(0)
    )
    {
        return computeMedian(values.data(), values.size(), histogram, maxValue);
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of a vector of values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline double computeMedian(const std::vector<T>& values, T maxValue = T(0))
    {
        sparse_histogram_t histogram;
        return computeMedian(values, histogram, maxValue);
    }

}
