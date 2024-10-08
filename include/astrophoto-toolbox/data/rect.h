/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represents a 2D rect
    //------------------------------------------------------------------------------------
    struct rect_t
    {
        //_____ Attributes __________
    public:
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;


        //_____ Constructors (for convenience) __________
    public:
        rect_t() {}

        rect_t(int left, int top, int right, int bottom)
        : left(left), top(top), right(right), bottom(bottom)
        {}


        //_____ Additional operations __________
    public:
        inline int width() const
        {
            return right - left;
        }

        inline int height() const
        {
            return bottom - top;
        }

        inline rect_t intersection(const rect_t& other)
        {
            return rect_t(
                std::max(left, other.left),
                std::max(top, other.top),
                std::min(right, other.right),
                std::min(bottom, other.bottom)
            );
        }
    };

}
