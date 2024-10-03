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
    /// @brief  Compute the average of a vector of values
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeAverage(const std::vector<T>& values)
    {
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }


    //------------------------------------------------------------------------------------
    /// @brief  Compute the standard deviation of a vector of values
    ///
    /// The average is also returned (as a parameter).
    //------------------------------------------------------------------------------------
    template <class T>
    inline double computeStandardDeviation(const std::vector<T>& values, double& average)
    {
        double result = 0.0;
        double squareDiff = 0.0;

        // Compute the average
        average = computeAverage(values);

        for (double val : values)
            squareDiff += std::pow(val - average, 2);

        if (values.size())
            result = sqrt(squareDiff / values.size());

        return result;
    }

    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of a vector of values
    //------------------------------------------------------------------------------------
    template<typename T>
    inline T computeMedian(const std::vector<T>& values, T maxValue = T(0))
    {
        if (maxValue == T(0))
            maxValue = *std::max_element(values.begin(), values.end());

        // Compute the histogram of the values
        histogram_t histogram;
        computeHistogram(values, histogram, maxValue);

        // Compute the median
        const size_t nbTotalValues = values.size() / 2 + values.size() % 2;
        size_t nbValues = 0;
        size_t index = 0;
        while (nbValues < nbTotalValues)
            nbValues += histogram[index++];

        return T(double(index) * maxValue / 65535.0);
    }

}
