/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/algorithms/bahtinov.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <string.h>


namespace astrophototoolbox {

point_t findStarInBitmapWithBahtinovMask(Bitmap* bitmap, double* radius)
{
    DoubleGrayBitmap* luminance = computeLuminanceBitmap(bitmap);

    double* data = luminance->data();

    std::vector<double> vmax(luminance->height());
    std::vector<double> hmax(luminance->width());

    memset(hmax.data(), 0, luminance->width() * sizeof(double));

    for (unsigned int y = 0; y < luminance->height(); ++y)
    {
        double max = 0.0;

        for (unsigned int x = 0; x < luminance->width(); ++x)
        {
            max = std::max(max, *data);
            hmax[x] = std::max(hmax[x], *data);
            ++data;
        }

        vmax[y] = max;
    }

    int y1 = 0;
    int y2 = luminance->height() - 1;

    double max1 = vmax[0];
    double max2 = vmax[y2];

    for (unsigned int y = 1, ry = y2 - 1; y < luminance->height(); ++y, --ry)
    {
        if (vmax[y] > max1)
        {
            y1 = y;
            max1 = vmax[y];
        }

        if (vmax[ry] > max2)
        {
            y2 = ry;
            max2 = vmax[ry];
        }
    }

    int x1 = 0;
    int x2 = luminance->width() - 1;

    max1 = hmax[0];
    max2 = hmax[x2];

    for (unsigned int x = 1, rx = x2 - 1; x < luminance->width(); ++x, --rx)
    {
        if (hmax[x] > max1)
        {
            x1 = x;
            max1 = hmax[x];
        }

        if (hmax[rx] > max2)
        {
            x2 = rx;
            max2 = hmax[rx];
        }
    }

    if (luminance != bitmap)
        delete luminance;

    point_t center((x1 + x2) / 2.0, (y1 + y2) / 2.0);

    if (radius)
        *radius = std::max(x2 - center.x, y2 - center.y);

    return center;
}

}
