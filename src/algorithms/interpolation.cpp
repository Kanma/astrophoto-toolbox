/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/algorithms/interpolation.h>
#include <algorithm>


namespace astrophototoolbox {

/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Interpolation::Interpolation(double x0, double x1, double x2, double y0, double y1, double y2)
{
    double t1 = (x0 * y0 - x1 * y1) * (y0 - y2) - (x0 * y0 - x2 * y2) * (y0 - y1);
    double t2 = (x0 - x1) * (y0 - y2) - (x0 - x2) * (y0 - y1);
    double t3 = y0 - y1;

    if (t1)
        b = t2 / t1;
    else
        b = 0;

    if (t3)
        c = ((x0 - x1) - b * (x0 * y0 - x1 * y1)) / t3;
    else
        c = 0;

    a = (b * x0 + c) * y0 - x0;

    min = std::min(std::min(y0, y1), y2);
    max = std::max(std::max(y0, y1), y2);
}


/************************************** METHODS ****************************************/

double Interpolation::interpolate(double x) const
{
    if (b || c)
        return std::max(std::min((x + a) / (b * x + c), max), min);
    else
        return std::max(std::min(x + a, max), min);
}

//-----------------------------------------------------------------------------

void Interpolation::interpolate(DoubleGrayBitmap* bitmap) const
{
    double* data = bitmap->data();
    for (unsigned int i = 0; i < bitmap->size() / sizeof(double); ++i)
        *data = interpolate(*data);

}

}
