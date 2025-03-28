/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/data/transformation.h>
#include <algorithm>

using namespace astrophototoolbox;


astrophototoolbox::point_t Transformation::transform(const point_t& pt) const noexcept
{
    point_t result;
    double& x = result.x;
    double& y = result.y;

    double X = pt.x / xWidth;
    double Y = pt.y / yWidth;

    x = a0 + a1 * X + a2 * Y + a3 * X * Y;
    y = b0 + b1 * X + b2 * Y + b3 * X * Y;

    x *= xWidth;
    y *= yWidth;

    return result;
}

//-----------------------------------------------------------------------------

rect_t Transformation::transform(const rect_t& rect) const noexcept
{
    point_t p0 = transform(point_t(rect.left, rect.top));
    point_t p1 = transform(point_t(rect.right, rect.top));
    point_t p2 = transform(point_t(rect.left, rect.bottom));
    point_t p3 = transform(point_t(rect.right, rect.bottom));

    return rect_t(
        std::min({ p0.x, p1.x, p2.x, p3.x }),
        std::min({ p0.y, p1.y, p2.y, p3.y }),
        std::max({ p0.x, p1.x, p2.x, p3.x }),
        std::max({ p0.y, p1.y, p2.y, p3.y })
    );
}

//-----------------------------------------------------------------------------

double Transformation::angle(int width) const noexcept
{
    double angle;

    point_t pt1(0, 0);
    point_t pt2(width, 0);

    pt1 = transform(pt1);
    pt2 = transform(pt2);

    angle = atan2(pt2.y - pt1.y, pt2.x - pt1.x);

    return angle;
}

//-----------------------------------------------------------------------------

void Transformation::offsets(double& dX, double& dY) const noexcept
{
	dX = a0 * xWidth;
	dY = b0 * yWidth;
}
