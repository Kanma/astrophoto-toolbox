#include <catch.hpp>
#include <astrophoto-toolbox/astrometry/astrometry.h>
#include <astrophoto-toolbox/data/fits.h>
#include "../images/bitmap_helpers.h"


TEST_CASE("No star at creation", "[Astrometry]")
{
    Astrometry astrometry;
    auto stars = astrometry.getStarList();
    REQUIRE(stars.size() == 0);
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

    auto stars = astrometry.getStarList();

    REQUIRE(stars.size() == 3);

    REQUIRE(stars[0].x == Approx(45.78783f));
    REQUIRE(stars[0].y == Approx(59.32189f));
    REQUIRE(stars[0].flux == Approx(175.77803f));
    REQUIRE(stars[0].background == Approx(48.87514f));

    REQUIRE(stars[1].x == Approx(61.18722f));
    REQUIRE(stars[1].y == Approx(39.89201f));
    REQUIRE(stars[1].flux == Approx(166.55237f));
    REQUIRE(stars[1].background == Approx(48.88011f));

    REQUIRE(stars[2].x == Approx(57.18385f));
    REQUIRE(stars[2].y == Approx(91.79505f));
    REQUIRE(stars[2].flux == Approx(44.6794f));
    REQUIRE(stars[2].background == Approx(48.83362f));

    auto info = astrometry.getDetectionInfo();

    REQUIRE(info.imageWidth == 120);
    REQUIRE(info.imageHeight == 120);
    REQUIRE(info.estimatedSourceVariance == Approx(13.72017f));

    delete channel;
    delete bitmap;
}


TEST_CASE("Star uniformization", "[Astrometry]")
{
    star_list_t list;

    star_t star;
    star.x = 100.0f;
    star.y = 10.0f;
    list.push_back(star);

    star.x = 80.0f;
    star.y = 20.0f;
    list.push_back(star);

    star.x = 10.0f;
    star.y = 20.0f;
    list.push_back(star);

    star.x = 20.0f;
    star.y = 15.0f;
    list.push_back(star);

    star.x = 100.0f;
    star.y = 50.0f;
    list.push_back(star);

    star.x = 80.0f;
    star.y = 40.0f;
    list.push_back(star);

    star.x = 10.0f;
    star.y = 45.0f;
    list.push_back(star);

    star_detection_info_t info;
    info.imageWidth = 120;
    info.imageHeight = 60;

    Astrometry astrometry;
    astrometry.setStarList(list, info);
    
    REQUIRE(astrometry.uniformize(4));

    auto stars = astrometry.getStarList();

    REQUIRE(stars.size() == 7);

    REQUIRE(stars[0].x == Approx(100.0f));
    REQUIRE(stars[0].y == Approx(10.0));

    REQUIRE(stars[1].x == Approx(10.0f));
    REQUIRE(stars[1].y == Approx(20.0f));

    REQUIRE(stars[2].x == Approx(80.0f));
    REQUIRE(stars[2].y == Approx(20.0f));

    REQUIRE(stars[3].x == Approx(20.0f));
    REQUIRE(stars[3].y == Approx(15.0f));

    REQUIRE(stars[4].x == Approx(100.0f));
    REQUIRE(stars[4].y == Approx(50.0f));

    REQUIRE(stars[5].x == Approx(10.0f));
    REQUIRE(stars[5].y == Approx(45.0f));

    REQUIRE(stars[6].x == Approx(80.0f));
    REQUIRE(stars[6].y == Approx(40.0f));
}


TEST_CASE("Star list cut", "[Astrometry]")
{
    star_list_t list;

    star_t star;
    for (unsigned int i = 0; i < 20; ++i)
    {
        star.x = float(i);
        star.y = float(i * 2);
        list.push_back(star);
    }

    star_detection_info_t info;
    info.imageWidth = 120;
    info.imageHeight = 60;

    Astrometry astrometry;
    astrometry.setStarList(list, info);
    
    astrometry.cut(10);

    auto stars = astrometry.getStarList();

    REQUIRE(stars.size() == 10);

    for (unsigned int i = 0; i < 10; ++i)
    {
        REQUIRE(stars[i].x == Approx(list[i].x));
        REQUIRE(stars[i].y == Approx(list[i].y));
    }
}
