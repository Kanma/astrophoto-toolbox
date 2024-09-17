/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/images/helpers.h>

using namespace astrophototoolbox;


TEST_CASE("Hot pixels removal", "[Bitmap helpers]")
{
    DoubleColorBitmap image(20, 10);

    *(image.data(5, 10)) = 1.0;
    *(image.data(10, 10) + 1) = 1.0;
    *(image.data(15, 10) + 2) = 1.0;

    removeHotPixels(&image);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.range() == RANGE_ONE);
    REQUIRE(image.space() == SPACE_LINEAR);

    double* data = image.data();

    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
        REQUIRE(data[i] == 0.0f);
}
