#include <catch.hpp>
#include <astrophoto-toolbox/astrometry/astrometry.h>
#include <astrophoto-toolbox/data/fits.h>
#include "../images/bitmap_helpers.h"


TEST_CASE("No star at creation", "[Astrometry]")
{
    Astrometry astrometry;
    auto list = astrometry.getStarList();
    REQUIRE(list.stars.size() == 0);
}


TEST_CASE("Fail to detect stars without image", "[Astrometry]")
{
    Astrometry astrometry;
    REQUIRE(!astrometry.detectStars(nullptr));
}


TEST_CASE("Fail to detect stars in black image", "[Astrometry]")
{
    FloatGrayBitmap bitmap(100, 100);

    Astrometry astrometry;
    REQUIRE(astrometry.detectStars(&bitmap));
}


TEST_CASE("Star detection", "[Astrometry]")
{
    FITS fits;
    fits.open(DATA_DIR "stars.fits");

    Bitmap* bitmap = fits.readBitmap();
    Bitmap* channel = bitmap->channel(0);

    Astrometry astrometry;
    REQUIRE(astrometry.detectStars(channel));

    auto list = astrometry.getStarList();

    REQUIRE(list.stars.size() == 3);

    REQUIRE(list.stars[0].x == Approx(45.78783f));
    REQUIRE(list.stars[0].y == Approx(59.32189f));
    REQUIRE(list.stars[0].flux == Approx(45174.957f));
    REQUIRE(list.stars[0].background == Approx(12560.91016f));

    REQUIRE(list.stars[1].x == Approx(61.18722f));
    REQUIRE(list.stars[1].y == Approx(39.89201f));
    REQUIRE(list.stars[1].flux == Approx(42803.957f));
    REQUIRE(list.stars[1].background == Approx(12562.1875f));

    REQUIRE(list.stars[2].x == Approx(57.18385f));
    REQUIRE(list.stars[2].y == Approx(91.79505f));
    REQUIRE(list.stars[2].flux == Approx(11482.608f));
    REQUIRE(list.stars[2].background == Approx(12550.24121f));

    REQUIRE(list.estimatedSourceVariance == Approx(3526.0835f));

    delete channel;
    delete bitmap;
}
