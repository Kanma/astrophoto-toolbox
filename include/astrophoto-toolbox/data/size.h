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
    /// @brief  Represents a 2D size
    //------------------------------------------------------------------------------------
    struct size2d_t
    {
        //_____ Attributes __________
    public:
        int width = 0;
        int height = 0;


        //_____ Constructors (for convenience) __________
    public:
        size2d_t() = default;

        size2d_t(int width, int height)
        : width(width), height(height)
        {}
    };

}
