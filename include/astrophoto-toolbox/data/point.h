/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <vector>
#include <cmath>

namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represents a 2D point
    //------------------------------------------------------------------------------------
     struct point_t
    {
        //_____ Attributes __________
    public:
        double x = 0.0;
        double y = 0.0;


        //_____ Constructors (for convenience) __________
    public:
        point_t() = default;

        point_t(double x, double y)
        : x(x), y(y)
        {}


        //_____ Additional operations __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Returns the distance to another point
        //--------------------------------------------------------------------------------
        double distance(const point_t& p) const
        {
            return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
        }


        //_____ Comparison operators __________
    public:
        friend constexpr auto operator<=>(const point_t& lhs, const point_t& rhs)
        {
            if (auto cmp = lhs.x <=> rhs.x; cmp != 0)
                return cmp;
            return lhs.y <=> rhs.y;
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a list of points
    //------------------------------------------------------------------------------------
    typedef std::vector<point_t> point_list_t;

}
