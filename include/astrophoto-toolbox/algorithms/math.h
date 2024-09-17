/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

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

}
