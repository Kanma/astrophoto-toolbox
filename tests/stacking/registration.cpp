/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;


TEST_CASE("Registration of a bitmap", "[Registration]")
{
    RawImage image;
    FITS input;

    REQUIRE(image.open(DATA_DIR "downloads/starfield.CR2"));

    DoubleColorBitmap bitmap;
    REQUIRE(image.toBitmap(&bitmap));
    REQUIRE(bitmap.width() == 3906);
    REQUIRE(bitmap.height() == 2602);

    removeHotPixels(&bitmap);

    stacking::Registration registration;
    star_list_t stars;

    SECTION("with a fixed threshold")
    {
        stars = registration.registerBitmap(&bitmap);
        REQUIRE(stars.size() == 101);
        REQUIRE(registration.getLuminancyThreshold() == 10);

        REQUIRE(computeQuality(stars) == Approx(641.66933243));
        REQUIRE(computeFWHM(stars) == Approx(4.6888294344));

        REQUIRE(input.open(DATA_DIR "stars/starfield_stars_fixed.fits"));
    }

    SECTION("searching for a good threshold")
    {
        stars = registration.registerBitmap(&bitmap, -1);
        REQUIRE(stars.size() == 36);
        REQUIRE(registration.getLuminancyThreshold() == 47);

        REQUIRE(computeQuality(stars) == Approx(208.0239777654));
        REQUIRE(computeFWHM(stars) == Approx(5.5732949245));

        REQUIRE(input.open(DATA_DIR "stars/starfield_stars_search.fits"));
    }

    star_list_t ref = input.readStars();

    REQUIRE(stars.size() == ref.size());

    std::sort(stars.begin(), stars.end());
    std::sort(ref.begin(), ref.end());

    for (size_t i = 0; i < stars.size(); ++i)
    {
        REQUIRE(stars[i].position.x == Approx(ref[i].position.x).margin(0.001));
        REQUIRE(stars[i].position.y == Approx(ref[i].position.y).margin(0.001));
        REQUIRE(stars[i].intensity == Approx(ref[i].intensity).margin(0.001));
    }
}
