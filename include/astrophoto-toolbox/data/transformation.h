/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/data/point.h>


namespace astrophototoolbox
{

    class Transformation
    {
    public:
        double a0, a1, a2, a3;
        double b0, b1, b2, b3;
        double xWidth, yWidth;

    public:
    	point_t transform(const point_t& pt) const noexcept;
        double angle(int width) const noexcept;
        void offsets(double& dX, double& dY) const noexcept;
    };

}
