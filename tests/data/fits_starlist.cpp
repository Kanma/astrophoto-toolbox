#include <catch.hpp>
#include <astrophoto-toolbox/data/fits.h>
#include "fits_helpers.h"
#include <fstream>

using namespace astrophototoolbox;


void checkTableHeader(
    const char* filename, int naxis, int naxis2, const star_detection_info_t& info,
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
    checkHeaderEntry(sheader, "TFIELDS", 4);
    checkHeaderEntry(sheader, "TTYPE1", "X");
    checkHeaderEntry(sheader, "TTYPE2", "Y");
    checkHeaderEntry(sheader, "TTYPE3", "FLUX");
    checkHeaderEntry(sheader, "TTYPE4", "BACKGROUND");

    checkHeaderEntry(sheader, "IMAGEW", (int) info.imageWidth);
    checkHeaderEntry(sheader, "IMAGEH", (int) info.imageHeight);
    checkHeaderEntry(sheader, "ESTSIGMA", info.estimatedSourceVariance);
    checkHeaderEntry(sheader, "DPSF", info.gaussianPsfWidth);
    checkHeaderEntry(sheader, "PLIM", info.significanceLimit);
    checkHeaderEntry(sheader, "DLIM", info.distanceLimit);
    checkHeaderEntry(sheader, "SADDLE", info.saddleDifference);
    checkHeaderEntry(sheader, "MAXPER", info.maxNbPeaksPerObject);
    checkHeaderEntry(sheader, "MAXPEAKS", info.maxNbPeaksTotal);
    checkHeaderEntry(sheader, "MAXSIZE", info.maxSize);
    checkHeaderEntry(sheader, "HALFBOX", info.slidingSkyWindowHalfSize);
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
    star.x = 100.0f;
    star.y = 10.0f;
    list.push_back(star);

    star.x = 80.0f;
    star.y = 20.0f;
    list.push_back(star);

    star.x = 10.0f;
    star.y = 20.0f;
    list.push_back(star);

    return list;
}


star_detection_info_t getDetectionInfo()
{
    star_detection_info_t info;

    info.imageWidth = 120;
    info.imageHeight = 60;
    info.estimatedSourceVariance = 1.0f;
    info.gaussianPsfWidth = 2.0f;
    info.significanceLimit = 3.0f;
    info.distanceLimit = 4.0f;
    info.saddleDifference = 5.0f;
    info.maxNbPeaksPerObject = 10;
    info.maxNbPeaksPerObject = 10;
    info.maxNbPeaksTotal = 20;
    info.maxSize = 30;
    info.slidingSkyWindowHalfSize = 40;

    return info;
}


TEST_CASE("Save stars", "[FITS]")
{
    star_list_t list = getStarList();
    star_detection_info_t info = getDetectionInfo();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "stars.fits"));
    REQUIRE(output.write(list, info));
    output.close();

    checkBitmapHeader(TEMP_DIR "stars.fits", 0, 0, 0, 0, 0);
    checkTableHeader(TEMP_DIR "stars.fits", 2, 3, info, 1, "STARS");
}


TEST_CASE("Save stars with name", "[FITS]")
{
    star_list_t list = getStarList();
    star_detection_info_t info = getDetectionInfo();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "namedstars.fits"));
    REQUIRE(output.write(list, info, "SOURCES"));
    output.close();

    checkTableHeader(TEMP_DIR "namedstars.fits", 2, 3, info, 1, "SOURCES");
}


TEST_CASE("Save stars multiple times", "[FITS]")
{
    star_list_t list = getStarList();
    star_detection_info_t info = getDetectionInfo();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "modifiedstars.fits"));
    REQUIRE(output.write(list, info));

    info.imageWidth = 200;
    REQUIRE(!output.write(list, info));
    REQUIRE(output.write(list, info, "STARS", true));

    output.close();

    checkTableHeader(TEMP_DIR "modifiedstars.fits", 2, 3, info, 1, "STARS");
}


TEST_CASE("Save multiple stars", "[FITS]")
{
    star_list_t list = getStarList();
    star_detection_info_t info = getDetectionInfo();
    star_detection_info_t info2 = getDetectionInfo();

    info2.imageWidth = 200;

    FITS output;
    REQUIRE(output.create(TEMP_DIR "multiplestars.fits"));
    REQUIRE(output.write(list, info));
    REQUIRE(output.write(list, info2, "STARS2"));

    output.close();

    checkTableHeader(TEMP_DIR "multiplestars.fits", 2, 3, info, 1, "STARS");
    checkTableHeader(TEMP_DIR "multiplestars.fits", 2, 3, info2, 2, "STARS2");
}


TEST_CASE("Save astrometry.net keywords", "[FITS]")
{
    star_list_t list = getStarList();
    star_detection_info_t info = getDetectionInfo();

    FITS output;
    REQUIRE(output.create(TEMP_DIR "stars.axy"));
    REQUIRE(output.write(list, info));
    REQUIRE(output.writeAstrometryNetKeywords(200, 100));

    output.close();

    checkTableHeader(TEMP_DIR "stars.axy", 2, 3, info, 1, "STARS");
    checkANKeywords(TEMP_DIR "stars.axy", 200, 100);
}


TEST_CASE("Retrieve infos about FITS file containing stars", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "starlist.fits"));

    REQUIRE(input.nbHDUs() == 2);
    REQUIRE(input.nbImages() == 1);
    REQUIRE(input.nbTables() == 1);
}


TEST_CASE("Load stars", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "starlist.fits"));

    star_detection_info_t info;
    star_list_t stars = input.readStarList(0, &info);

    REQUIRE(stars.size() == 3);

    REQUIRE(stars[0].x == Approx(100.0f));
    REQUIRE(stars[0].y == Approx(10.0f));

    REQUIRE(stars[1].x == Approx(80.0f));
    REQUIRE(stars[1].y == Approx(20.0f));

    REQUIRE(stars[2].x == Approx(10.0f));
    REQUIRE(stars[2].y == Approx(20.0f));

    REQUIRE(info.imageWidth == 120);
    REQUIRE(info.imageHeight == 60);
    REQUIRE(info.estimatedSourceVariance == Approx(1.0f));
    REQUIRE(info.gaussianPsfWidth == Approx(2.0f));
    REQUIRE(info.significanceLimit == Approx(3.0f));
    REQUIRE(info.distanceLimit == Approx(4.0f));
    REQUIRE(info.saddleDifference == Approx(5.0f));
    REQUIRE(info.maxNbPeaksPerObject == 10);
    REQUIRE(info.maxNbPeaksPerObject == 10);
    REQUIRE(info.maxNbPeaksTotal == 20);
    REQUIRE(info.maxSize == 30);
    REQUIRE(info.slidingSkyWindowHalfSize == 40);
}


TEST_CASE("Fail to load inexistent stars", "[FITS]")
{
    SECTION("from image file")
    {
        FITS input;
        REQUIRE(input.open(DATA_DIR "color8bits.fits"));

        star_list_t stars = input.readStarList(0);
        REQUIRE(stars.empty());
    }

    SECTION("using invalid index")
    {
        FITS input;
        REQUIRE(input.open(DATA_DIR "starlist.fits"));

        star_list_t stars = input.readStarList(1);
        REQUIRE(stars.empty());
    }
}
