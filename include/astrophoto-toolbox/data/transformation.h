/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/data/point.h>
#include <astrophoto-toolbox/data/rect.h>


namespace astrophototoolbox
{

    class Transformation
    {
    public:
        double a0 = 0.0;
        double a1 = 1.0;
        double a2 = 0.0;
        double a3 = 0.0;
        double b0 = 0.0;
        double b1 = 0.0;
        double b2 = 1.0;
        double b3 = 0.0;
        double xWidth = 1.0;
        double yWidth = 1.0;

    public:
        double angle(int width) const noexcept;
        void offsets(double& dX, double& dY) const noexcept;

        point_t transform(const point_t& pt) const noexcept;

        rect_t transform(const rect_t& rect) const noexcept;

        template<class BITMAP>
            requires(BITMAP::Channels == 3)
        BITMAP* transform(BITMAP* bitmap) const noexcept
        {
            double median = computeMedian(bitmap);

            BITMAP* target = new BITMAP(bitmap->width(), bitmap->height());

            for (unsigned int y = 0; y < bitmap->height(); ++y)
            {
                for (unsigned int x = 0; x < bitmap->width(); ++x)
                {
                    point_t p = transform(point_t(x, y));

                    int xx = int(p.x);
                    int yy = int(p.y);

                    if ((xx >= 0) && (xx < target->width()) && (yy >= 0) && (yy < target->height()))
                    {
                        typename BITMAP::type_t* src = (typename BITMAP::type_t*) bitmap->ptr(x, y);
                        typename BITMAP::type_t* dest = target->data(xx, yy);
                        dest[0] = src[0];
                        dest[1] = src[1];
                        dest[2] = src[2];
                    }
                }
            }

            return target;
        }
    };

}
