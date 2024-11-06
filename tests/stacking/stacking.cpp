/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/stacking.h>
#include <astrophoto-toolbox/data/fits.h>
#include <fstream>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("(Stacking) Initialisation", "[Stacking]")
{
    std::filesystem::remove_all(TEMP_DIR "stacking");

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.nbDarkFrames() == 0);
    REQUIRE(stacking.nbLightFrames() == 0);

    REQUIRE(!stacking.process());
}


TEST_CASE("(Stacking) Master dark frame computation", "[Stacking]")
{
    std::filesystem::remove_all(TEMP_DIR "stacking");

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    REQUIRE(stacking.nbDarkFrames() == 3);

    REQUIRE(!stacking.process());

    std::filesystem::path path(TEMP_DIR "stacking/master_dark.fits");
    REQUIRE(std::filesystem::exists(path));

    FITS fits;
    REQUIRE(fits.open(path));

    Bitmap* bitmap = fits.readBitmap();
    REQUIRE(bitmap);
    delete bitmap;

    point_list_t hotPixels = fits.readPoints();
    REQUIRE(hotPixels.size() == 274);
}


TEST_CASE("(Stacking) Stacking", "[Stacking]")
{
    std::filesystem::remove_all(TEMP_DIR "stacking");

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits", true);
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(stacking.nbLightFrames() == 3);

    Bitmap* bitmap = stacking.process();
    REQUIRE(bitmap);
    delete bitmap;

    const auto readFile = [](const std::string& filename, star_list_t& stars, Transformation& transformation)
    {
        std::filesystem::path path(filename);
        REQUIRE(std::filesystem::exists(path));

        FITS fits;
        REQUIRE(fits.open(path));

        Bitmap* bitmap = fits.readBitmap();
        REQUIRE(bitmap);
        delete bitmap;

        stars = fits.readStars();
        transformation = fits.readTransformation();
    };

    star_list_t stars;
    Transformation transformation;
    point_t point;

    readFile(TEMP_DIR "stacking/calibrated/lightframes/light1.fits", stars, transformation);
    REQUIRE(stars.size() == 38);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(200.0));
    REQUIRE(point.y == Approx(100.0));

    readFile(TEMP_DIR "stacking/calibrated/lightframes/light2.fits", stars, transformation);
    REQUIRE(stars.size() == 30);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(216.529).margin(0.001));
    REQUIRE(point.y == Approx(98.799).margin(0.001));

    readFile(TEMP_DIR "stacking/calibrated/lightframes/light3.fits", stars, transformation);
    REQUIRE(stars.size() == 34);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(136.686).margin(0.001));
    REQUIRE(point.y == Approx(196.510).margin(0.001));
}


TEST_CASE("(Stacking) Config file", "[Stacking]")
{
    std::filesystem::remove_all(TEMP_DIR "stacking");

    {
        Stacking<UInt16ColorBitmap> stacking;

        stacking.setup(TEMP_DIR "stacking");

        stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
        stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
        stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

        stacking.addLightFrame(DATA_DIR "downloads/light1.fits", true);
        stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
        stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

        REQUIRE(stacking.save());
    }

    {
        std::filesystem::path config(TEMP_DIR "stacking/stacking.txt");
        REQUIRE(std::filesystem::exists(config));

        std::ifstream input(config, std::ios::in);
        std::string line;

        REQUIRE(std::getline(input, line));
        REQUIRE(line == "DARKFRAMES");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/dark1.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/dark2.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/dark3.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == "---");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == "LIGHTFRAMES");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/light1.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/light2.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == DATA_DIR "downloads/light3.fits");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == "REF 0");

        REQUIRE(std::getline(input, line));
        REQUIRE(line == "---");

        REQUIRE(!std::getline(input, line));
    }

    {
        Stacking<UInt16ColorBitmap> stacking;

        stacking.setup(TEMP_DIR "stacking");

        REQUIRE(stacking.load());
        REQUIRE(stacking.nbDarkFrames() == 3);
        REQUIRE(stacking.nbLightFrames() == 3);
    }
}
