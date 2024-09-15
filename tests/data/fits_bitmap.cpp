#include <catch.hpp>
#include <astrophoto-toolbox/data/fits.h>
#include "../images/bitmap_helpers.h"
#include "fits_helpers.h"
#include <fstream>

using namespace astrophototoolbox;


template<typename T>
void checkIntegralContent(const char* filename, Bitmap* ref, uint64_t bzero, unsigned int channel = 0)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    test.seekg(2880 + channel * ref->width() * ref->height() * sizeof(T), std::ios_base::beg);

    T* ptr = (T*) ref->ptr();
    for (unsigned int y = 0; y < ref->height(); ++y)
    {
        for (unsigned int x = 0; x < ref->width(); ++x)
        {
            T v = 0;
            uint8_t c;
            for (unsigned int i = 0; i < sizeof(T); ++i)
            {
                test.read((char*) &c, sizeof(uint8_t));
                v |= T(c) << ((sizeof(T) - 1 - i) * 8);
            }

            v += bzero;

            REQUIRE(v == *ptr);
            ++ptr;
        }
    }
}


template<typename T, typename T2>
void checkFloatContent(const char* filename, Bitmap* ref, unsigned int channel = 0)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    test.seekg(2880 + channel * ref->width() * ref->height() * sizeof(T), std::ios_base::beg);

    T* ptr = (T*) ref->ptr();
    for (unsigned int y = 0; y < ref->height(); ++y)
    {
        for (unsigned int x = 0; x < ref->width(); ++x)
        {
            T2 v = 0;
            uint8_t c;
            for (unsigned int i = 0; i < sizeof(T); ++i)
            {
                test.read((char*) &c, sizeof(uint8_t));
                v |= T2(c) << ((sizeof(T) - 1 - i) * 8);
            }

            T* f = (T*) &v;

            REQUIRE(*f == *ptr);
            ++ptr;
        }
    }
}


TEST_CASE("Save 8-bits gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt8Bitmap(false, 32, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray8bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "gray8bits.fits", 8, 255.0, 2, 32, 8);
    checkIntegralContent<uint8_t>(TEMP_DIR "gray8bits.fits", bitmap, 0);

    delete bitmap;
}


TEST_CASE("Save 8-bits gray bitmap as FITS, with name", "[FITS]")
{
    Bitmap* bitmap = createUInt8Bitmap(false, 32, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "namedgray8bits.fits"));
    REQUIRE(output.write(bitmap, "image"));
    output.close();

    checkBitmapHeader(TEMP_DIR "namedgray8bits.fits", 8, 255.0, 2, 32, 8, 0, "image");
    checkIntegralContent<uint8_t>(TEMP_DIR "namedgray8bits.fits", bitmap, 0);

    delete bitmap;
}


TEST_CASE("Save 16-bits gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt16Bitmap(false, 256, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray16bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "gray16bits.fits", 16, 65535.0, 2, 256, 8);
    checkIntegralContent<uint16_t>(TEMP_DIR "gray16bits.fits", bitmap, 0x8000);

    delete bitmap;
}


TEST_CASE("Save 32-bits gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt32Bitmap(false, 256, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray32bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "gray32bits.fits", 32, 4294967295.0, 2, 256, 8);
    checkIntegralContent<uint32_t>(TEMP_DIR "gray32bits.fits", bitmap, 0x80000000);

    delete bitmap;
}


TEST_CASE("Save float gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createFloatBitmap(false, 100, 8, 0.05f, 1.0f);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "grayfloat.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "grayfloat.fits", -32, 1.0, 2, 100, 8);
    checkFloatContent<float, uint32_t>(TEMP_DIR "grayfloat.fits", bitmap);

    delete bitmap;
}


TEST_CASE("Save double gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createDoubleBitmap(false, 100, 8, 0.05, 1.0);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "graydouble.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "graydouble.fits", -64, 1.0, 2, 100, 8);
    checkFloatContent<double, uint64_t>(TEMP_DIR "graydouble.fits", bitmap);

    delete bitmap;
}


TEST_CASE("Save 8-bits color bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt8Bitmap(true, 32, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "color8bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "color8bits.fits", 8, 255.0, 3, 32, 8, 3);

    Bitmap* channel = bitmap->channel(0);
    checkIntegralContent<uint8_t>(TEMP_DIR "color8bits.fits", channel, 0, 0);
    delete channel;

    channel = bitmap->channel(1);
    checkIntegralContent<uint8_t>(TEMP_DIR "color8bits.fits", channel, 0, 1);
    delete channel;

    channel = bitmap->channel(2);
    checkIntegralContent<uint8_t>(TEMP_DIR "color8bits.fits", channel, 0, 2);
    delete channel;

    delete bitmap;
}


TEST_CASE("Save 16-bits color bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt16Bitmap(true, 256, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "color16bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "color16bits.fits", 16, 65535.0, 3, 256, 8, 3);

    Bitmap* channel = bitmap->channel(0);
    checkIntegralContent<uint16_t>(TEMP_DIR "color16bits.fits", channel, 0x8000, 0);
    delete channel;

    channel = bitmap->channel(1);
    checkIntegralContent<uint16_t>(TEMP_DIR "color16bits.fits", channel, 0x8000, 1);
    delete channel;

    channel = bitmap->channel(2);
    checkIntegralContent<uint16_t>(TEMP_DIR "color16bits.fits", channel, 0x8000, 2);
    delete channel;

    delete bitmap;
}


TEST_CASE("Save 32-bits color bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt32Bitmap(true, 256, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "color32bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "color32bits.fits", 32, 4294967295.0, 3, 256, 8, 3);

    Bitmap* channel = bitmap->channel(0);
    checkIntegralContent<uint32_t>(TEMP_DIR "color32bits.fits", channel, 0x80000000, 0);
    delete channel;

    channel = bitmap->channel(1);
    checkIntegralContent<uint32_t>(TEMP_DIR "color32bits.fits", channel, 0x80000000, 1);
    delete channel;

    channel = bitmap->channel(2);
    checkIntegralContent<uint32_t>(TEMP_DIR "color32bits.fits", channel, 0x80000000, 2);
    delete channel;

    delete bitmap;
}


TEST_CASE("Save float color bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createFloatBitmap(true, 100, 8, 0.05f, 1.0f);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "colorfloat.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "colorfloat.fits", -32, 1.0, 3, 100, 8, 3);

    Bitmap* channel = bitmap->channel(0);
    checkFloatContent<float, uint32_t>(TEMP_DIR "colorfloat.fits", channel, 0);
    delete channel;

    channel = bitmap->channel(1);
    checkFloatContent<float, uint32_t>(TEMP_DIR "colorfloat.fits", channel, 1);
    delete channel;

    channel = bitmap->channel(2);
    checkFloatContent<float, uint32_t>(TEMP_DIR "colorfloat.fits", channel, 2);
    delete channel;

    delete bitmap;
}


TEST_CASE("Save double color bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createDoubleBitmap(true, 100, 8, 0.05, 1.0);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "colordouble.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkBitmapHeader(TEMP_DIR "colordouble.fits", -64, 1.0, 3, 100, 8, 3);

    Bitmap* channel = bitmap->channel(0);
    checkFloatContent<double, uint64_t>(TEMP_DIR "colordouble.fits", channel, 0);
    delete channel;

    channel = bitmap->channel(1);
    checkFloatContent<double, uint64_t>(TEMP_DIR "colordouble.fits", channel, 1);
    delete channel;

    channel = bitmap->channel(2);
    checkFloatContent<double, uint64_t>(TEMP_DIR "colordouble.fits", channel, 2);
    delete channel;

    delete bitmap;
}


TEST_CASE("Retrieve infos about 8-bits gray FITS image file", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray8bits.fits"));

    REQUIRE(input.nbHDUs() == 1);
    REQUIRE(input.nbImages() == 1);
    REQUIRE(input.nbTables() == 0);
}


TEST_CASE("Read 8-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray8bits.fits"));

    Bitmap* ref = createUInt8Bitmap(false, 32, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt8GrayBitmap*>(bitmap));

    checkIdenticalBitmaps<uint8_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read 16-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray16bits.fits"));

    Bitmap* ref = createUInt16Bitmap(false, 256, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt16GrayBitmap*>(bitmap));

    checkIdenticalBitmaps<uint16_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read 32-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray32bits.fits"));

    Bitmap* ref = createUInt32Bitmap(false, 256, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt32GrayBitmap*>(bitmap));

    checkIdenticalBitmaps<uint32_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read float gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/grayfloat.fits"));

    Bitmap* ref = createFloatBitmap(false, 100, 8, 0.05f, 1.0f);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<FloatGrayBitmap*>(bitmap));

    checkIdenticalBitmaps<float>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read double gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/graydouble.fits"));

    Bitmap* ref = createDoubleBitmap(false, 100, 8, 0.05, 1.0);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<DoubleGrayBitmap*>(bitmap));

    checkIdenticalBitmaps<double>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read 8-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/color8bits.fits"));

    Bitmap* ref = createUInt8Bitmap(true, 32, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt8ColorBitmap*>(bitmap));

    checkIdenticalBitmaps<uint8_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read 16-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/color16bits.fits"));

    Bitmap* ref = createUInt16Bitmap(true, 256, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt16ColorBitmap*>(bitmap));

    checkIdenticalBitmaps<uint16_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read 32-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/color32bits.fits"));

    Bitmap* ref = createUInt32Bitmap(true, 256, 8);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<UInt32ColorBitmap*>(bitmap));

    checkIdenticalBitmaps<uint32_t>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read float color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/colorfloat.fits"));

    Bitmap* ref = createFloatBitmap(true, 100, 8, 0.05f, 1.0f);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<FloatColorBitmap*>(bitmap));

    checkIdenticalBitmaps<float>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read double color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/colordouble.fits"));

    Bitmap* ref = createDoubleBitmap(true, 100, 8, 0.05, 1.0);

    Bitmap* bitmap = input.readBitmap();
    REQUIRE(bitmap);
    REQUIRE(dynamic_cast<DoubleColorBitmap*>(bitmap));

    checkIdenticalBitmaps<double>(bitmap, ref);

    delete ref;
    delete bitmap;
}


TEST_CASE("Read FITS image with invalid index", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray8bits.fits"));

    Bitmap* bitmap = input.readBitmap(10);
    REQUIRE(!bitmap);
}


TEST_CASE("Read FITS image with valid name", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/namedgray8bits.fits"));

    Bitmap* bitmap = input.readBitmap("image");
    REQUIRE(bitmap);
    delete bitmap;
}


TEST_CASE("Read FITS image with invalid name", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "images/gray8bits.fits"));

    Bitmap* bitmap = input.readBitmap("unknown");
    REQUIRE(!bitmap);
}
