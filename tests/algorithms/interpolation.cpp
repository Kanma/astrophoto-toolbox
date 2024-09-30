/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/algorithms/interpolation.h>

using namespace astrophototoolbox;


TEST_CASE("Interpolation (full range)", "[Interpolation]")
{
    Interpolation interpolation(0, 100, 255, 0, 150, 255);

    REQUIRE(interpolation.interpolate(0.0) == Approx(0.0));
    REQUIRE(interpolation.interpolate(100.0) == Approx(150.0));
    REQUIRE(interpolation.interpolate(255.0) == Approx(255.0));
}


TEST_CASE("Interpolation (reduced range)", "[Interpolation]")
{
    Interpolation interpolation(10, 100, 240, 20, 150, 230);

    REQUIRE(interpolation.interpolate(0.0) == Approx(20.0));
    REQUIRE(interpolation.interpolate(10.0) == Approx(20.0));
    REQUIRE(interpolation.interpolate(100.0) == Approx(150.0));
    REQUIRE(interpolation.interpolate(240.0) == Approx(230.0));
    REQUIRE(interpolation.interpolate(255.0) == Approx(230.0));
}
