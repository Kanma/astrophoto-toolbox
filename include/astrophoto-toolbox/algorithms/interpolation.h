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
    /// @brief  Allows to interpolate a value expressed in a source range given a median
    ///         value into another one
    //------------------------------------------------------------------------------------
    class Interpolation
    {
        //_____ Construction / Destruction __________
    public:
        Interpolation(double x0, double x1, double x2, double y0, double y1, double y2);


        //_____ Methods __________
    public:
        double interpolate(double x) const;

        void interpolate(DoubleGrayBitmap* bitmap) const;


        //_____ Attributes __________
    private:
        double a, b, c;
        double min, max;
    };

}
