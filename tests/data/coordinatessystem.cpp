/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/data/coordinatessystem.h>

using namespace astrophototoolbox;


TEST_CASE("Coordinates System", "[CoordinatesSystem]")
{
    size2d_t imageSize(3906, 2602);
    Coordinates coords(101.072, -16.605);

    tan_t wcstan;
    wcstan.imagew = imageSize.width;
    wcstan.imageh = imageSize.height;
    wcstan.crval[0] = 101.34;
    wcstan.crval[1] = -16.796;
    wcstan.crpix[0] = 970.594;
    wcstan.crpix[1] = 1297.935;
    wcstan.cd[0][0] = -0.000261;
    wcstan.cd[0][1] = -0.000195;
    wcstan.cd[1][0] = 0.000195;
    wcstan.cd[1][1] = -0.000261;
    wcstan.sin = 0;

    CoordinatesSystem system(imageSize, coords, 1.1741, 143.270, wcstan);

    point_t position = system.convert(coords);
    REQUIRE(position.x == Approx(1952.67).margin(0.01));
    REQUIRE(position.y == Approx(1300.52).margin(0.01));

    coords = system.convert(position);
    REQUIRE(coords.getRA() == Approx(101.072));
    REQUIRE(coords.getDEC() == Approx(-16.605));

    position = system.convert(Coordinates(101.0, -16.605));
    REQUIRE(position.x == Approx(2122.139).margin(0.001));
    REQUIRE(position.y == Approx(1427.54).margin(0.001));

    coords = system.convert(position);
    REQUIRE(coords.getRA() == Approx(101.0));
    REQUIRE(coords.getDEC() == Approx(-16.605));

    position = system.convert(Coordinates(102.0, -16.605));
    REQUIRE(position.x == Approx(-235.683).margin(0.001));
    REQUIRE(position.y == Approx(-331.121).margin(0.001));

    coords = system.convert(position);
    REQUIRE(coords.getRA() == Approx(102.0));
    REQUIRE(coords.getDEC() == Approx(-16.605));

    position = system.convert(Coordinates(101.072, -16.0));
    REQUIRE(position.x == Approx(3066.225).margin(0.001));
    REQUIRE(position.y == Approx(-185.731).margin(0.001));

    coords = system.convert(position);
    REQUIRE(coords.getRA() == Approx(101.072));
    REQUIRE(coords.getDEC() == Approx(-16.0));

    position = system.convert(Coordinates(101.072, -17.0));
    REQUIRE(position.x == Approx(1225.696).margin(0.001));
    REQUIRE(position.y == Approx(2270.812).margin(0.001));

    coords = system.convert(position);
    REQUIRE(coords.getRA() == Approx(101.072));
    REQUIRE(coords.getDEC() == Approx(-17.0));

    REQUIRE(system.isInsideImage(Coordinates(101.072, -17.0)));
    REQUIRE(!system.isInsideImage(Coordinates(101.072, -20.0)));

    REQUIRE(system.isInsideImage(point_t(200, 1000)));
    REQUIRE(!system.isInsideImage(point_t(4000, 1000)));
}
