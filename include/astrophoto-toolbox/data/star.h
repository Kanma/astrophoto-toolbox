/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/data/point.h>
#include <set>
#include <vector>
#include <cmath>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Contains all the informations about a star in an image
    ///
    /// Note that since we support different methods to detect a star, all fields might
    /// not be valid. At the moment, only the position and intensity are really used and
    /// mandatory.
    //------------------------------------------------------------------------------------
    struct star_t
    {
        //_____ Attributes __________
    public:
        point_t position;
        double intensity = 0.0;

        // For stars detected by 'stacking::Registration'
        double quality = 0.0;
        double meanRadius = 0.0;


        //_____ Constructors (for convenience) __________
    public:
        star_t() = default;
 
        star_t(const double x, const double y)
        : position(x, y)
        {}

        star_t(const point_t position)
        : position(position)
        {}


        //_____ Additional operations __________
    public:
        inline bool contains(const point_t& point) const noexcept
        {
            return position.distance(point) <= meanRadius * (2.35 / 1.5);
        }


        inline bool contains(double x, double y) const noexcept
        {
            return contains(point_t(x, y));
        }


        //_____ Comparison operators (used to sort list of stars) __________
    public:
        // Stars are sorted by their position in the image by default
        friend constexpr auto operator<=>(const star_t& lhs, const star_t& rhs)
        {
            return lhs.position <=> rhs.position;
        }

        // // Two stars are equal if they are at the same position
        // friend constexpr auto operator==(const star_t& lhs, const star_t& rhs)
        // {
        //     return operator<=>(lhs, rhs) == 0;
        // }

        static constexpr bool compareIntensity(const star_t& star1, const star_t& star2)
        {
            if (star1.intensity > star2.intensity)
                return true;

            if (star1.intensity < star2.intensity)
                return false;

            return (star1.meanRadius > star2.meanRadius);
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a list of stars
    //------------------------------------------------------------------------------------
    typedef std::vector<star_t> star_list_t;


    //------------------------------------------------------------------------------------
    /// @brief  Represents a set of stars
    //------------------------------------------------------------------------------------
    typedef std::set<star_t> star_set_t;


    //------------------------------------------------------------------------------------
    /// @brief  Returns the overall quality of a list of stars (higher is better)
    //------------------------------------------------------------------------------------
    double computeQuality(const star_list_t& stars);


    //------------------------------------------------------------------------------------
    /// @brief  Returns the full width at half maximum (FWHM) of a list of stars (lower is
    ///         better)
    ///
    /// The FWHM is computed over the mean radius of the stars.
    //------------------------------------------------------------------------------------
    double computeFWHM(const star_list_t& stars);

}
