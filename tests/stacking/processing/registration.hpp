/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/processing/registration.h>
#include <astrophoto-toolbox/data/fits.h>
#include <fstream>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("(Stacking/Processing/Registration) Fail to register missing reference frame file", "[RegistrationProcessor]")
{
    RegistrationProcessor<UInt16ColorBitmap> processor;
    star_list_t stars = processor.processReference(DATA_DIR "missing.fits");
    REQUIRE(stars.empty());
}


TEST_CASE("(Stacking/Processing/Registration) Fail to register missing light frame file", "[RegistrationProcessor]")
{
    RegistrationProcessor<UInt16ColorBitmap> processor;
    std::tuple<star_list_t, Transformation> result = processor.process(DATA_DIR "missing.fits");
    REQUIRE(get<0>(result).empty());
}


TEST_CASE("(Stacking/Processing/Registration) Registration process", "[RegistrationProcessor]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "lightframes/light3.fits"));

    RegistrationProcessor<UInt16ColorBitmap> processor;

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
    point_t point, point2;

    star_list_t refStars = processor.processReference(
        TEMP_DIR "lightframes/light1.fits", -1, TEMP_DIR "lightframes/light1.fits"
    );
    REQUIRE(refStars.size() == 38);

    readFile(TEMP_DIR "lightframes/light1.fits", stars, transformation);
    REQUIRE(stars.size() == refStars.size());

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(200.0));
    REQUIRE(point.y == Approx(100.0));

    auto result = processor.process(
        TEMP_DIR "lightframes/light2.fits", TEMP_DIR "lightframes/light2.fits"
    );
    REQUIRE(get<0>(result).size() == 30);

    point = get<1>(result).transform(point_t(200, 100));
    REQUIRE(point.x == Approx(216.529).margin(0.001));
    REQUIRE(point.y == Approx(98.799).margin(0.001));

    readFile(TEMP_DIR "lightframes/light2.fits", stars, transformation);
    REQUIRE(stars.size() == get<0>(result).size());

    point2 = transformation.transform(point_t(200, 100));
    REQUIRE(point2.x == Approx(point.x).margin(0.001));
    REQUIRE(point2.y == Approx(point.y).margin(0.001));

    result = processor.process(
        TEMP_DIR "lightframes/light3.fits", TEMP_DIR "lightframes/light3.fits"
    );
    REQUIRE(get<0>(result).size() == 34);

    point = get<1>(result).transform(point_t(200, 100));
    REQUIRE(point.x == Approx(136.686).margin(0.001));
    REQUIRE(point.y == Approx(196.510).margin(0.001));

    readFile(TEMP_DIR "lightframes/light3.fits", stars, transformation);
    REQUIRE(stars.size() == get<0>(result).size());

    point2 = transformation.transform(point_t(200, 100));
    REQUIRE(point2.x == Approx(point.x).margin(0.001));
    REQUIRE(point2.y == Approx(point.y).margin(0.001));
}
