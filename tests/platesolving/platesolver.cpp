/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/platesolving/platesolver.h>
#include <astrophoto-toolbox/data/fits.h>
#include "../images/bitmap_helpers.h"

using namespace astrophototoolbox;
using namespace astrophototoolbox::platesolving;


TEST_CASE("No star at creation", "[PlateSolver]")
{
    PlateSolver solver;
    auto stars = solver.getStars();
    REQUIRE(stars.size() == 0);
}


TEST_CASE("Fail to detect stars without image", "[PlateSolver]")
{
    PlateSolver solver;
    REQUIRE(!solver.detectStars(nullptr));
}


TEST_CASE("Fail to detect stars in black image", "[PlateSolver]")
{
    FloatGrayBitmap bitmap(100, 100);

    PlateSolver solver;
    REQUIRE(solver.detectStars(&bitmap));
}


TEST_CASE("Star detection", "[PlateSolver]")
{
    FITS fits;
    fits.open(DATA_DIR "images/stars.fits");

    Bitmap* bitmap = fits.readBitmap();
    Bitmap* channel = bitmap->channel(0);

    PlateSolver solver;
    REQUIRE(solver.detectStars(channel));

    auto stars = solver.getStars();

    REQUIRE(stars.size() == 3);

    REQUIRE(stars[0].position.x == Approx(45.78783f));
    REQUIRE(stars[0].position.y == Approx(59.32189f));
    REQUIRE(stars[0].intensity == Approx(175.77803f));

    REQUIRE(stars[1].position.x == Approx(61.18722f));
    REQUIRE(stars[1].position.y == Approx(39.89201f));
    REQUIRE(stars[1].intensity == Approx(166.55237f));

    REQUIRE(stars[2].position.x == Approx(57.18385f));
    REQUIRE(stars[2].position.y == Approx(91.79505f));
    REQUIRE(stars[2].intensity == Approx(44.6794f));

    auto imageSize = solver.getImageSize();

    REQUIRE(imageSize.width == 120);
    REQUIRE(imageSize.height == 120);

    delete channel;
    delete bitmap;
}


TEST_CASE("Star uniformization", "[PlateSolver]")
{
    star_list_t list;

    star_t star;
    star.position.x = 100.0f;
    star.position.y = 10.0f;
    list.push_back(star);

    star.position.x = 80.0f;
    star.position.y = 20.0f;
    list.push_back(star);

    star.position.x = 10.0f;
    star.position.y = 20.0f;
    list.push_back(star);

    star.position.x = 20.0f;
    star.position.y = 15.0f;
    list.push_back(star);

    star.position.x = 100.0f;
    star.position.y = 50.0f;
    list.push_back(star);

    star.position.x = 80.0f;
    star.position.y = 40.0f;
    list.push_back(star);

    star.position.x = 10.0f;
    star.position.y = 45.0f;
    list.push_back(star);

    size2d_t imageSize(120, 60);

    PlateSolver solver;
    solver.setStars(list, imageSize);
    
    REQUIRE(solver.uniformize(4));

    auto stars = solver.getStars();

    REQUIRE(stars.size() == 7);

    REQUIRE(stars[0].position.x == Approx(100.0f));
    REQUIRE(stars[0].position.y == Approx(10.0));

    REQUIRE(stars[1].position.x == Approx(10.0f));
    REQUIRE(stars[1].position.y == Approx(20.0f));

    REQUIRE(stars[2].position.x == Approx(80.0f));
    REQUIRE(stars[2].position.y == Approx(20.0f));

    REQUIRE(stars[3].position.x == Approx(20.0f));
    REQUIRE(stars[3].position.y == Approx(15.0f));

    REQUIRE(stars[4].position.x == Approx(100.0f));
    REQUIRE(stars[4].position.y == Approx(50.0f));

    REQUIRE(stars[5].position.x == Approx(10.0f));
    REQUIRE(stars[5].position.y == Approx(45.0f));

    REQUIRE(stars[6].position.x == Approx(80.0f));
    REQUIRE(stars[6].position.y == Approx(40.0f));
}


TEST_CASE("Star list cut", "[PlateSolver]")
{
    star_list_t list;

    star_t star;
    for (unsigned int i = 0; i < 20; ++i)
    {
        star.position.x = float(i);
        star.position.y = float(i * 2);
        list.push_back(star);
    }

    size2d_t imageSize(120, 60);

    PlateSolver solver;
    solver.setStars(list, imageSize);
    
    solver.cut(10);

    auto stars = solver.getStars();

    REQUIRE(stars.size() == 10);

    for (unsigned int i = 0; i < 10; ++i)
    {
        REQUIRE(stars[i].position.x == Approx(list[i].position.x));
        REQUIRE(stars[i].position.y == Approx(list[i].position.y));
    }
}


TEST_CASE("Fail to load index files", "[PlateSolver]")
{
    SECTION("from folder without index file")
    {
        PlateSolver solver;
        REQUIRE(!solver.loadIndexes(TEMP_DIR "empty"));
    }

    SECTION("from inexistent folder")
    {
        PlateSolver solver;
        REQUIRE(!solver.loadIndexes(DATA_DIR "unknown"));
    }
}


TEST_CASE("Plate solving", "[PlateSolver]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "stars/starfield.axy"));

    size2d_t imageSize;
    star_list_t stars = input.readStars(0, &imageSize);

    REQUIRE(!stars.empty());

    PlateSolver solver;
    solver.setStars(stars, imageSize);

    REQUIRE(solver.loadIndexes(DATA_DIR "downloads/indexes"));

    REQUIRE(solver.solve(0.5, 2.0));

    Coordinates coordinates = solver.getCoordinates();
    REQUIRE(coordinates.getRA() == Approx(282.654));
    REQUIRE(coordinates.getDEC() == Approx(-12.9414));

    REQUIRE(solver.getPixelSize() == Approx(1.173130749).margin(0.001));

    REQUIRE(solver.getRightAscensionOrientation() == Approx(-109.18136).margin(0.001));
    REQUIRE(solver.getDeclinationOrientation() == Approx(160.81863).margin(0.001));

    CoordinatesSystem system = solver.getCoordinatesSystem();

    point_t position = system.convert(coordinates);
    REQUIRE(position.x == Approx(1953.0));
    REQUIRE(position.y == Approx(1301.0));

    coordinates = system.convert(position);
    REQUIRE(coordinates.getRA() == Approx(282.654));
    REQUIRE(coordinates.getDEC() == Approx(-12.9414));

    position = system.convert(Coordinates(282.0, -12.9414));
    REQUIRE(position.x == Approx(2596.401).margin(0.001));
    REQUIRE(position.y == Approx(-545.413).margin(0.001));

    coordinates = system.convert(position);
    REQUIRE(coordinates.getRA() == Approx(282.0));
    REQUIRE(coordinates.getDEC() == Approx(-12.9414));

    position = system.convert(Coordinates(283.0, -12.9414));
    REQUIRE(position.x == Approx(1614.181).margin(0.001));
    REQUIRE(position.y == Approx(2279.537).margin(0.001));

    coordinates = system.convert(position);
    REQUIRE(coordinates.getRA() == Approx(283.0));
    REQUIRE(coordinates.getDEC() == Approx(-12.9414));

    position = system.convert(Coordinates(282.654, -13.0));
    REQUIRE(position.x == Approx(2122.674).margin(0.001));
    REQUIRE(position.y == Approx(1360.608).margin(0.001));

    coordinates = system.convert(position);
    REQUIRE(coordinates.getRA() == Approx(282.654));
    REQUIRE(coordinates.getDEC() == Approx(-13.0));

    position = system.convert(Coordinates(282.654, -12.0));
    REQUIRE(position.x == Approx(-776.716).margin(0.001));
    REQUIRE(position.y == Approx(354.465).margin(0.001));

    coordinates = system.convert(position);
    REQUIRE(coordinates.getRA() == Approx(282.654));
    REQUIRE(coordinates.getDEC() == Approx(-12.0));
}


TEST_CASE("Fail to do plate solving without index files", "[PlateSolver]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "stars/starfield.axy"));

    size2d_t imageSize;
    star_list_t stars = input.readStars(0, &imageSize);

    REQUIRE(!stars.empty());

    PlateSolver solver;
    solver.setStars(stars, imageSize);

    REQUIRE(!solver.solve(0.5, 2.0));

    Coordinates coordinates = solver.getCoordinates();
    REQUIRE(coordinates.isNull());
}
