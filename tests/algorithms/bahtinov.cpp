/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/algorithms/bahtinov.h>
#include <astrophoto-toolbox/data/fits.h>
#include <string.h>

using namespace astrophototoolbox;


TEST_CASE("Find star in image with Bahtinov mask", "[Bahtinov]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/bahtinov_mask.fits"));

    Bitmap* bitmap = input.readBitmap();

    point_t position = findStarInBitmapWithBahtinovMask(bitmap);
    delete bitmap;

    REQUIRE(position.x == 106.0);
    REQUIRE(position.y == 190.5);
}


TEST_CASE("Find star in unfocused image with Bahtinov mask", "[Bahtinov]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/bahtinov_mask_unfocused.fits"));

    Bitmap* bitmap = input.readBitmap();

    point_t position = findStarInBitmapWithBahtinovMask(bitmap);
    delete bitmap;

    REQUIRE(position.x == 126.5);
    REQUIRE(position.y == 128.0);
}
