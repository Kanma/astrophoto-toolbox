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

    REQUIRE(stars[0].position.x == Approx(45.78783f));
    REQUIRE(stars[0].position.y == Approx(59.32189f));
    REQUIRE(stars[0].intensity == Approx(175.77803f));

    REQUIRE(stars[1].position.x == Approx(61.18722f));
    REQUIRE(stars[1].position.y == Approx(39.89201f));
    REQUIRE(stars[1].intensity == Approx(166.55237f));

    REQUIRE(stars[2].position.x == Approx(57.18385f));
    REQUIRE(stars[2].position.y == Approx(91.79505f));
    REQUIRE(stars[2].intensity == Approx(44.6794f));

    auto imageSize = astrometry.getImageSize();

    REQUIRE(imageSize.width == 120);
    REQUIRE(imageSize.height == 120);

    delete channel;
    delete bitmap;
}


TEST_CASE("Star uniformization", "[Astrometry]")
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

    Astrometry astrometry;
    astrometry.setStarList(list, imageSize);
    
    REQUIRE(astrometry.uniformize(4));

    auto stars = astrometry.getStarList();

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


TEST_CASE("Star list cut", "[Astrometry]")
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

    Astrometry astrometry;
    astrometry.setStarList(list, imageSize);
    
    astrometry.cut(10);

    auto stars = astrometry.getStarList();

    REQUIRE(stars.size() == 10);

    for (unsigned int i = 0; i < 10; ++i)
    {
        REQUIRE(stars[i].position.x == Approx(list[i].position.x));
        REQUIRE(stars[i].position.y == Approx(list[i].position.y));
    }
}


TEST_CASE("Fail to load index files", "[Astrometry]")
{
    SECTION("from folder without index file")
    {
        Astrometry astrometry;
        REQUIRE(!astrometry.loadIndexes(TEMP_DIR "empty"));
    }

    SECTION("from inexistent folder")
    {
        Astrometry astrometry;
        REQUIRE(!astrometry.loadIndexes(DATA_DIR "unknown"));
    }
}


TEST_CASE("Plate solving", "[Astrometry]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "starfield.axy"));

    size2d_t imageSize;
    star_list_t stars = input.readStars(0, &imageSize);

    REQUIRE(!stars.empty());

    Astrometry astrometry;
    astrometry.setStarList(stars, imageSize);

    REQUIRE(astrometry.loadIndexes(DATA_DIR "downloads"));

    REQUIRE(astrometry.solve(0.5, 2.0));

    Coordinates coordinates = astrometry.getCoordinates();
    REQUIRE(coordinates.getRA() == Approx(282.654));
    REQUIRE(coordinates.getDEC() == Approx(-12.9414));

    REQUIRE(astrometry.getPixelSize() == Approx(1.1736878371));
}


TEST_CASE("Fail to do plate solving without index files", "[Astrometry]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "starfield.axy"));

    size2d_t imageSize;
    star_list_t stars = input.readStars(0, &imageSize);

    REQUIRE(!stars.empty());

    Astrometry astrometry;
    astrometry.setStarList(stars, imageSize);

    REQUIRE(!astrometry.solve(0.5, 2.0));

    Coordinates coordinates = astrometry.getCoordinates();
    REQUIRE(coordinates.isNull());
}
