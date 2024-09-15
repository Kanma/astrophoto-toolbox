#include <catch.hpp>
#include <astrophoto-toolbox/images/pnm.h>
#include <fstream>

using namespace astrophototoolbox;


void checkFile(const std::string& filename)
{
    std::ifstream tested(TEMP_DIR + filename, std::ios_base::in | std::ios_base::binary);
    std::ifstream ref(DATA_DIR "images/" + filename, std::ios_base::in | std::ios_base::binary);

    REQUIRE(tested.is_open());
    REQUIRE(ref.is_open());

    uint8_t testedBuffer[10000];
    uint8_t refBuffer[10000];

    tested.seekg(0, std::ios_base::end);
    size_t testedSize = tested.tellg();
    tested.seekg(0, std::ios_base::beg);

    ref.seekg(0, std::ios_base::end);
    size_t refSize = ref.tellg();
    ref.seekg(0, std::ios_base::beg);

    REQUIRE(testedSize == refSize);

    tested.read((char*) testedBuffer, refSize);
    ref.read((char*) refBuffer, refSize);

    for (unsigned int i = 0; i < refSize; ++i)
        REQUIRE(testedBuffer[i] == refBuffer[i]);
}


TEST_CASE("Save 8-bits color bitmap", "[PNM]")
{
    UInt8ColorBitmap bitmap(9, 3);

    uint8_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint8_t* data = bitmap.data(y);

        for (unsigned int x = 0, i = 0; x < bitmap.width(); ++x, i += 3)
        {
            if (x < 3)
                v = 0;
            else if (x < 6)
                v = 128;
            else
                v = 255;

            data[i] = v;
        }
    }

    SECTION("as 8bits PPM")
    {
        REQUIRE(pnm::save(TEMP_DIR "rgb8bits.ppm", &bitmap));
        checkFile("rgb8bits.ppm");
    }

    SECTION("as 8bits PGM")
    {
        REQUIRE(pnm::save(TEMP_DIR "rgb8bits.pgm", &bitmap));
        checkFile("rgb8bits.pgm");
    }
}


TEST_CASE("Save 16-bits color bitmap", "[PNM]")
{
    UInt16ColorBitmap bitmap(9, 3);

    uint16_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint16_t* data = bitmap.data(y);

        for (unsigned int x = 0, i = 0; x < bitmap.width(); ++x, i += 3)
        {
            if (x < 3)
                v = 0;
            else if (x < 6)
                v = 0x8800;
            else
                v = 0xFFFF;

            data[i] = v;
        }
    }

    SECTION("as 16bits PPM")
    {
        REQUIRE(pnm::save(TEMP_DIR "rgb16bits.ppm", &bitmap));
        checkFile("rgb16bits.ppm");
    }

    SECTION("as 16bits PGM")
    {
        REQUIRE(pnm::save(TEMP_DIR "rgb16bits.pgm", &bitmap));
        checkFile("rgb16bits.pgm");
    }
}


TEST_CASE("Save 8-bits gray bitmap", "[PNM]")
{
    UInt8GrayBitmap bitmap(9, 3);

    uint8_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint8_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            if (x < 3)
                v = 0;
            else if (x < 6)
                v = 128;
            else
                v = 255;

            data[x] = v;
        }
    }

    SECTION("as 8bits PPM")
    {
        REQUIRE(pnm::save(TEMP_DIR "gray8bits.ppm", &bitmap));
        checkFile("gray8bits.ppm");
    }

    SECTION("as 8bits PGM")
    {
        REQUIRE(pnm::save(TEMP_DIR "gray8bits.pgm", &bitmap));
        checkFile("gray8bits.pgm");
    }
}


TEST_CASE("Save 16-bits gray bitmap", "[PNM]")
{
    UInt16GrayBitmap bitmap(9, 3);

    uint16_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint16_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            if (x < 3)
                v = 0;
            else if (x < 6)
                v = 0x8800;
            else
                v = 0xFFFF;

            data[x] = v;
        }
    }

    SECTION("as 16bits PPM")
    {
        REQUIRE(pnm::save(TEMP_DIR "gray16bits.ppm", &bitmap));
        checkFile("gray16bits.ppm");
    }

    SECTION("as 16bits PGM")
    {
        REQUIRE(pnm::save(TEMP_DIR "gray16bits.pgm", &bitmap));
        checkFile("gray16bits.pgm");
    }
}
