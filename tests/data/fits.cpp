#include <catch.hpp>
#include <astrophoto-toolbox/data/fits.h>
#include "../images/bitmap_helpers.h"
#include <fstream>
#include <string>

using namespace astrophototoolbox;


void checkHeaderEntry(const std::string& header, const std::string& key, int value)
{
    int pos = header.find((key + " ").c_str());
    REQUIRE(pos >= 0);

    char buffer[10] = { 0 };
    snprintf(buffer, 10, " %d ", value);

    pos = header.find(buffer, pos);
    REQUIRE(pos >= 0);
}


void checkHeader(const char* filename, int bitpix, int naxis, int naxis1, int naxis2, int naxis3 = 0)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    char header[2881] = { 0 };
    test.read(header, 2880);

    std::string sheader(header);

    checkHeaderEntry(sheader, "BITPIX", bitpix);
    checkHeaderEntry(sheader, "NAXIS", naxis);
    checkHeaderEntry(sheader, "NAXIS1", naxis1);
    checkHeaderEntry(sheader, "NAXIS2", naxis2);

    if (naxis3 != 0)
        checkHeaderEntry(sheader, "NAXIS3", naxis3);
}


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

    checkHeader(TEMP_DIR "gray8bits.fits", 8, 2, 32, 8);
    checkIntegralContent<uint8_t>(TEMP_DIR "gray8bits.fits", bitmap, 0);

    delete bitmap;
}


TEST_CASE("Save 16-bits gray bitmap as FITS", "[FITS]")
{
    Bitmap* bitmap = createUInt16Bitmap(false, 256, 8);

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray16bits.fits"));
    REQUIRE(output.write(bitmap));
    output.close();

    checkHeader(TEMP_DIR "gray16bits.fits", 16, 2, 256, 8);
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

    checkHeader(TEMP_DIR "gray32bits.fits", 32, 2, 256, 8);
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

    checkHeader(TEMP_DIR "grayfloat.fits", -32, 2, 100, 8);
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

    checkHeader(TEMP_DIR "graydouble.fits", -64, 2, 100, 8);
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

    checkHeader(TEMP_DIR "color8bits.fits", 8, 3, 32, 8, 3);

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

    checkHeader(TEMP_DIR "color16bits.fits", 16, 3, 256, 8, 3);

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

    checkHeader(TEMP_DIR "color32bits.fits", 32, 3, 256, 8, 3);

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

    checkHeader(TEMP_DIR "colorfloat.fits", -32, 3, 100, 8, 3);

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

    checkHeader(TEMP_DIR "colordouble.fits", -64, 3, 100, 8, 3);

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


TEST_CASE("Read 8-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "gray8bits.fits"));

    Bitmap* ref = createUInt8Bitmap(false, 32, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<uint8_t>(&bitmap, ref);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read 16-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "gray16bits.fits"));

    Bitmap* ref = createUInt16Bitmap(false, 256, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        checkIdenticalBitmaps<uint16_t>(&bitmap, ref);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read 32-bits gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "gray32bits.fits"));

    Bitmap* ref = createUInt32Bitmap(false, 256, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<uint32_t>(&bitmap, ref);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read float gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "grayfloat.fits"));

    Bitmap* ref = createFloatBitmap(false, 100, 8, 0.05f, 1.0f);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<float>(&bitmap, ref);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read double gray FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "graydouble.fits"));

    Bitmap* ref = createDoubleBitmap(false, 100, 8, 0.05, 1.0);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<double>(&bitmap, ref);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Read 8-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "color8bits.fits"));

    Bitmap* ref = createUInt8Bitmap(true, 32, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<uint8_t>(&bitmap, ref);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read 16-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "color16bits.fits"));

    Bitmap* ref = createUInt16Bitmap(true, 256, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<uint16_t>(&bitmap, ref);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read 32-bits color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "color32bits.fits"));

    Bitmap* ref = createUInt32Bitmap(true, 256, 8);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<uint32_t>(&bitmap, ref);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read float color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "colorfloat.fits"));

    Bitmap* ref = createFloatBitmap(true, 100, 8, 0.05f, 1.0f);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<float>(&bitmap, ref);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        DoubleColorBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    delete ref;
}


TEST_CASE("Read double color FITS image", "[FITS]")
{
    FITS input;
    REQUIRE(input.open(DATA_DIR "colordouble.fits"));

    Bitmap* ref = createDoubleBitmap(true, 100, 8, 0.05, 1.0);

    SECTION("as 8-bits gray image")
    {
        UInt8GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits gray image")
    {
        UInt16GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt16GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits gray image")
    {
        UInt32GrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt32GrayBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float gray image")
    {
        FloatGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        FloatGrayBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double gray image")
    {
        DoubleGrayBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        DoubleGrayBitmap ref2(ref);
        checkIdenticalBitmaps<double>(&bitmap, &ref2);
    }

    SECTION("as 8-bits color image")
    {
        UInt8ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));

        UInt8ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint8_t>(&bitmap, &ref2);
    }

    SECTION("as 16-bits color image")
    {
        UInt16ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt16ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint16_t>(&bitmap, &ref2);
    }

    SECTION("as 32-bits color image")
    {
        UInt32ColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        UInt32ColorBitmap ref2(ref);
        checkIdenticalBitmaps<uint32_t>(&bitmap, &ref2);
    }

    SECTION("as float color image")
    {
        FloatColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        
        FloatColorBitmap ref2(ref);
        checkIdenticalBitmaps<float>(&bitmap, &ref2);
    }

    SECTION("as double color image")
    {
        DoubleColorBitmap bitmap;
        REQUIRE(input.readBitmap(&bitmap));
        checkIdenticalBitmaps<double>(&bitmap, ref);
    }

    delete ref;
}
