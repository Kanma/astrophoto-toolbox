#include <catch.hpp>
#include <astrophoto-toolbox/images/raw.h>
#include <fstream>

using namespace astrophototoolbox;


TEST_CASE("Open missing file", "[RAW]")
{
    RawImage image;
    REQUIRE(!image.open("unknown.CR2"));
}


TEST_CASE("Open invalid file", "[RAW]")
{
    RawImage image;
    REQUIRE(!image.open(DATA_DIR "CMakeLists.txt"));
}


TEST_CASE("Open valid file", "[RAW]")
{
    RawImage image;

    REQUIRE(image.open(DATA_DIR "starfield.CR2"));

    SECTION("check the attributes")
    {
        REQUIRE(image.width() == 3906);
        REQUIRE(image.height() == 2602);
        REQUIRE(image.channels() == 3);

        REQUIRE(image.isoSpeed() == 800);
        REQUIRE(image.shutterSpeed() == Approx(30.0f));
        REQUIRE(image.aperture() == Approx(0.0f));
        REQUIRE(image.focalLength() == Approx(0.0f));
    }

    SECTION("conversion to uint8 bitmap")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(image.toBitmap(&bitmap));
        REQUIRE(bitmap.width() == 3906);
        REQUIRE(bitmap.height() == 2602);
    }

    SECTION("conversion to uint16 bitmap")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(image.toBitmap(&bitmap));
        REQUIRE(bitmap.width() == 3906);
        REQUIRE(bitmap.height() == 2602);
    }
}


TEST_CASE("Open null buffer", "[RAW]")
{
    RawImage image;
    REQUIRE(!image.open(nullptr, 1000));
}


TEST_CASE("Open invalid buffer", "[RAW]")
{
    RawImage image;
    uint8_t buffer[100];

    memset(buffer, 0, 100);

    REQUIRE(!image.open(buffer, 100));
}


TEST_CASE("Open valid buffer", "[RAW]")
{
    RawImage image;

    std::ifstream stream(
        DATA_DIR "starfield.CR2", std::ios_base::in | std::ios_base::binary
    );

    REQUIRE(stream.is_open());

    stream.seekg(0, std::ios_base::end);
    size_t size = stream.tellg();

    stream.seekg(0, std::ios_base::beg);

    uint8_t* buffer = new uint8_t[size];
    stream.read((char*) buffer, size);

    REQUIRE(image.open(buffer, size));

    delete[] buffer;

    REQUIRE(image.width() == 3906);
    REQUIRE(image.height() == 2602);
    REQUIRE(image.channels() == 3);

    REQUIRE(image.isoSpeed() == 800);
    REQUIRE(image.shutterSpeed() == Approx(30.0f));
    REQUIRE(image.aperture() == Approx(0.0f));
    REQUIRE(image.focalLength() == Approx(0.0f));
}
