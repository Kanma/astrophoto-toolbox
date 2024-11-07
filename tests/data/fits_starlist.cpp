/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/data/fits.h>
#include "fits_helpers.h"
#include <fstream>

using namespace astrophototoolbox;


void checkTableHeader(
    const char* filename, int naxis, int naxis2, const size2d_t& imageSize,
    int extIndex, const std::string& extname
)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    char header[2881] = { 0 };
    std::string sheader;

    int counter = 0;
    while (!test.eof())
    {
        test.seekg(2880, std::ios_base::cur);
        test.read(header, 2880);

        sheader = header;

        if (sheader.starts_with("XTENSION"))
        {
            ++counter;
            if (counter == extIndex)
                break;
        }
    }

    REQUIRE(counter == extIndex);

    checkHeaderEntry(sheader, "XTENSION", "BINTABLE");
    checkHeaderEntry(sheader, "EXTNAME", extname);
    checkHeaderEntry(sheader, "NAXIS", naxis);
    checkHeaderEntry(sheader, "NAXIS2", naxis2);
    checkHeaderEntry(sheader, "TFIELDS", 5);
    checkHeaderEntry(sheader, "TTYPE1", "X");
    checkHeaderEntry(sheader, "TTYPE2", "Y");
    checkHeaderEntry(sheader, "TTYPE3", "INTENSITY");
    checkHeaderEntry(sheader, "TTYPE4", "QUALITY");
    checkHeaderEntry(sheader, "TTYPE5", "MEANRADIUS");

    checkHeaderEntry(sheader, "IMAGEW", (int) imageSize.width);
    checkHeaderEntry(sheader, "IMAGEH", (int) imageSize.height);
}


void checkANKeywords(const char* filename, int width, int height)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    char header[2881] = { 0 };
    test.read(header, 2880);

    std::string sheader(header);

    checkHeaderEntry(sheader, "IMAGEW", width);
    checkHeaderEntry(sheader, "IMAGEH", height);
    checkBoolHeaderEntry(sheader, "ANRUN", true);
    checkBoolHeaderEntry(sheader, "ANVERUNI", true);
    checkBoolHeaderEntry(sheader, "ANVERDUP", false);
    checkBoolHeaderEntry(sheader, "ANTWEAK", true);
    checkHeaderEntry(sheader, "ANTWEAKO", 2);
}


star_list_t getStarList()
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

    return list;
}


size2d_t getImageSize()
{
    return size2d_t(120, 60);
}


TEST_CASE("Save stars", "[FITS]")
{
    star_list_t list = getStarList();
    size2d_t imageSize = getImageSize();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "stars.fits"));
    REQUIRE(output.write(list, imageSize));
    output.close();

    checkBitmapHeader(TEMP_DIR "stars.fits", 0, 0, 0, 0, 0);
    checkTableHeader(TEMP_DIR "stars.fits", 2, 3, imageSize, 1, "STARS");
}


TEST_CASE("Save stars with name", "[FITS]")
{
    star_list_t list = getStarList();
    size2d_t imageSize = getImageSize();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "namedstars.fits"));
    REQUIRE(output.write(list, imageSize, nullptr, "SOURCES"));
    output.close();

    checkTableHeader(TEMP_DIR "namedstars.fits", 2, 3, imageSize, 1, "SOURCES");
}


TEST_CASE("Save stars multiple times", "[FITS]")
{
    star_list_t list = getStarList();
    size2d_t imageSize = getImageSize();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "modifiedstars.fits"));
    REQUIRE(output.write(list, imageSize));

    imageSize.width = 200;
    REQUIRE(!output.write(list, imageSize));
    REQUIRE(output.write(list, imageSize, nullptr, "STARS", true));

    output.close();

    checkTableHeader(TEMP_DIR "modifiedstars.fits", 2, 3, imageSize, 1, "STARS");
}


TEST_CASE("Save multiple stars", "[FITS]")
{
    star_list_t list = getStarList();
    size2d_t imageSize = getImageSize();
    size2d_t imageSize2 = getImageSize();

    imageSize2.width = 200;

    FITS output;
    REQUIRE(output.create(TEMP_DIR "multiplestars.fits"));
    REQUIRE(output.write(list, imageSize));
    REQUIRE(output.write(list, imageSize2, nullptr, "STARS2"));

    output.close();

    checkTableHeader(TEMP_DIR "multiplestars.fits", 2, 3, imageSize, 1, "STARS");
    checkTableHeader(TEMP_DIR "multiplestars.fits", 2, 3, imageSize2, 2, "STARS2");
}


TEST_CASE("Save astrometry.net keywords", "[FITS]")
{
    star_list_t list = getStarList();
    size2d_t imageSize = getImageSize();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "stars.axy"));
    REQUIRE(output.write(list, imageSize));
    REQUIRE(output.writeAstrometryNetKeywords(size2d_t(200, 100)));

    output.close();

    checkTableHeader(TEMP_DIR "stars.axy", 2, 3, imageSize, 1, "STARS");
    checkANKeywords(TEMP_DIR "stars.axy", 200, 100);
}


TEST_CASE("Retrieve infos about FITS file containing stars", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "stars/starlist.fits"));

    REQUIRE(input.nbHDUs() == 2);
    REQUIRE(input.nbImages() == 1);
    REQUIRE(input.nbTables() == 1);
}


TEST_CASE("Load stars", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "stars/starlist.fits"));

    size2d_t imageSize;
    star_list_t stars = input.readStars(0, &imageSize);

    REQUIRE(stars.size() == 3);

    REQUIRE(stars[0].position.x == Approx(100.0f));
    REQUIRE(stars[0].position.y == Approx(10.0f));

    REQUIRE(stars[1].position.x == Approx(80.0f));
    REQUIRE(stars[1].position.y == Approx(20.0f));

    REQUIRE(stars[2].position.x == Approx(10.0f));
    REQUIRE(stars[2].position.y == Approx(20.0f));

    REQUIRE(imageSize.width == 120);
    REQUIRE(imageSize.height == 60);
}


TEST_CASE("Fail to load inexistent stars", "[FITS]")
{
    SECTION("from image file")
    {
        FITS input;
        REQUIRE(input.open(DATA_DIR "images/color8bits.fits"));

        star_list_t stars = input.readStars(0);
        REQUIRE(stars.empty());
    }

    SECTION("using invalid index")
    {
        FITS input;
        REQUIRE(input.open(DATA_DIR "stars/starlist.fits"));

        star_list_t stars = input.readStars(1);
        REQUIRE(stars.empty());
    }
}
