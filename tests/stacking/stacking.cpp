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


TEST_CASE("Stacking initialisation", "[Stacking]")
{
    Stacking<UInt16ColorBitmap> stacking;

    REQUIRE(stacking.nbDarkFrames() == 0);
    REQUIRE(!stacking.hasMasterDark());
    REQUIRE(!stacking.getMasterDark());

    REQUIRE(stacking.nbLightFrames() == 0);
    REQUIRE(!stacking.hasReference());

    REQUIRE(!stacking.process());
}


TEST_CASE("Dark frames addition", "[Stacking]")
{
    REQUIRE(!std::filesystem::exists(TEMP_DIR "stacking"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(!stacking.hasMasterDark());
    REQUIRE(!stacking.getMasterDark());

    REQUIRE(stacking.save());

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

    REQUIRE(!std::getline(input, line));
}


TEST_CASE("Dark frames loading", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(!stacking.hasMasterDark());
    REQUIRE(!stacking.getMasterDark());
}


TEST_CASE("Master dark frame computation", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "stacking/master_dark.fits"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(!stacking.hasMasterDark());
    REQUIRE(!stacking.getMasterDark());

    REQUIRE(stacking.computeMasterDark());

    REQUIRE(stacking.hasMasterDark());

    auto* masterDark = stacking.getMasterDark();
    REQUIRE(masterDark);

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


TEST_CASE("Dark frames loading, with master present", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/master_dark.fits"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbDarkFrames() == 3);
    REQUIRE(stacking.hasMasterDark());
    REQUIRE(stacking.getMasterDark());
}


TEST_CASE("Light frames addition", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits", true);
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.nbLightFrames() == 3);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);

    REQUIRE(stacking.save());

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


TEST_CASE("Light frames loading", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbLightFrames() == 3);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);
}


TEST_CASE("Light frames calibration", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/master_dark.fits"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbLightFrames() == 3);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);

    REQUIRE(stacking.processLightFrames());

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

    readFile(TEMP_DIR "stacking/calibrated/lights/light1.fits", stars, transformation);
    REQUIRE(stars.size() == 38);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(200.0));
    REQUIRE(point.y == Approx(100.0));

    readFile(TEMP_DIR "stacking/calibrated/lights/light2.fits", stars, transformation);
    REQUIRE(stars.size() == 30);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(216.529).margin(0.001));
    REQUIRE(point.y == Approx(98.799).margin(0.001));

    readFile(TEMP_DIR "stacking/calibrated/lights/light3.fits", stars, transformation);
    REQUIRE(stars.size() == 34);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(136.686).margin(0.001));
    REQUIRE(point.y == Approx(196.510).margin(0.001));
}


TEST_CASE("Light frames stacking", "[Stacking]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/stacking.txt"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "stacking/calibrated/lights/light1.fits"));

    Stacking<UInt16ColorBitmap> stacking;

    stacking.setup(TEMP_DIR "stacking");

    REQUIRE(stacking.load());

    REQUIRE(stacking.nbLightFrames() == 3);
    REQUIRE(stacking.hasReference());
    REQUIRE(stacking.getReference() == 0);

    Bitmap* bitmap = stacking.process();
    REQUIRE(bitmap);
    delete bitmap;
}
