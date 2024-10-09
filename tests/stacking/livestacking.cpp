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


TEST_CASE("Live stacking initialisation", "[Stacking]")
{
    REQUIRE(!std::filesystem::exists(TEMP_DIR "livestacking"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "livestacking");

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(!stacking.hasMasterDark());
    REQUIRE(!stacking.getMasterDark());

    REQUIRE(stacking.save());

    REQUIRE(stacking.computeMasterDark());

    REQUIRE(stacking.hasMasterDark());

    REQUIRE(!stacking.process());
}


TEST_CASE("Light stacking first frame", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacking.txt"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "livestacking");

    REQUIRE(stacking.load());

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits", true);

    REQUIRE(stacking.nbLightFrames() == 1);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);

    REQUIRE(stacking.save());

    Bitmap* stacked = stacking.process();
    REQUIRE(stacked);
    delete stacked;

    star_list_t stars;
    Transformation transformation;
    point_t point;

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/calibrated/lights/light1.fits"));

    readFile(TEMP_DIR "livestacking/calibrated/lights/light1.fits", stars, transformation);
    REQUIRE(stars.size() == 38);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(200.0));
    REQUIRE(point.y == Approx(100.0));
}


TEST_CASE("Light stacking second frame", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacking.txt"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/calibrated/lights/light1.fits"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "livestacking");

    REQUIRE(stacking.load());

    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");

    REQUIRE(stacking.nbLightFrames() == 2);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);

    REQUIRE(stacking.save());

    Bitmap* stacked = stacking.process();
    REQUIRE(stacked);
    delete stacked;

    star_list_t stars;
    Transformation transformation;
    point_t point;

    readFile(TEMP_DIR "livestacking/calibrated/lights/light2.fits", stars, transformation);
    REQUIRE(stars.size() == 30);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(216.529).margin(0.001));
    REQUIRE(point.y == Approx(98.799).margin(0.001));
}
