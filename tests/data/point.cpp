/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/data/point.h>

using namespace astrophototoolbox;


TEST_CASE("Null point", "[Point]")
{
    point_t point;
    REQUIRE(point.isNull());
}


TEST_CASE("Valid point", "[Point]")
{
    point_t point(100, 50);

    SECTION("distance on x")
    {
        REQUIRE(!point.isNull());
        REQUIRE(point.x == Approx(100.0));
        REQUIRE(point.y == Approx(50.0));
    }

    SECTION("distance on x")
    {
        REQUIRE(point.distance(point_t(200, 50)) == Approx(100.0));
    }

    SECTION("distance on y")
    {
        REQUIRE(point.distance(point_t(100, 0)) == Approx(50.0));
    }

    SECTION("distance")
    {
        REQUIRE(point.distance(point_t(0, 100)) == Approx(111.803).margin(0.001));
    }
}
