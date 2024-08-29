#include <catch.hpp>
#include <astrophoto-toolbox/data/fits.h>
#include <fstream>

using namespace astrophototoolbox;


TEST_CASE("Save 8-bits gray bitmap as FITS", "[FITS]")
{
    UInt8GrayBitmap bitmap(9, 3);

    uint8_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint8_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            if (x < 3)
                v = 0x00;
            else if (x < 6)
                v = 0x80;
            else
                v = 0xF0;

            v += y;

            data[x] = v;
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray8bits.fits"));
    REQUIRE(output.write(&bitmap));
    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "gray8bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt8GrayBitmap* bitmap3 = dynamic_cast<UInt8GrayBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint8_t* ptr = bitmap3->data();
    uint8_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save 16-bits gray bitmap as FITS", "[FITS]")
{
    UInt16GrayBitmap bitmap(9, 3);

    uint16_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint16_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            if (x < 3)
                v = 0x0000;
            else if (x < 6)
                v = 0x8800;
            else
                v = 0xFFF0;

            v += y;

            data[x] = v;
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray16bits.fits"));
    REQUIRE(output.write(&bitmap));
    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "gray16bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt16GrayBitmap* bitmap3 = dynamic_cast<UInt16GrayBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint16_t* ptr = bitmap3->data();
    uint16_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save 32-bits gray bitmap as FITS", "[FITS]")
{
    UInt32GrayBitmap bitmap(9, 3);

    uint32_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint32_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            if (x < 3)
                v = 0x00000000;
            else if (x < 6)
                v = 0x88888800;
            else
                v = 0xFFFFFFF0;

            v += y;

            data[x] = v;
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "gray32bits.fits"));
    REQUIRE(output.write(&bitmap));
    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "gray32bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt32GrayBitmap* bitmap3 = dynamic_cast<UInt32GrayBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint32_t* ptr = bitmap3->data();
    uint32_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save float gray bitmap as FITS", "[FITS]")
{
    FloatGrayBitmap bitmap(9, 3);

    float v = 0.0f;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        float* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            data[x] = v;
            v += 1.0f / 18.0f;
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "grayfloat.fits"));
    REQUIRE(output.write(&bitmap));
    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "grayfloat.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    FloatGrayBitmap* bitmap3 = dynamic_cast<FloatGrayBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    float* ptr = bitmap3->data();
    float* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save double gray bitmap as FITS", "[FITS]")
{
    DoubleGrayBitmap bitmap(9, 3);

    double v = 0.0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        double* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            data[x] = v;
            v += 1.0 / 18.0;
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "graydouble.fits"));
    REQUIRE(output.write(&bitmap));
    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "graydouble.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    DoubleGrayBitmap* bitmap3 = dynamic_cast<DoubleGrayBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    double* ptr = bitmap3->data();
    double* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save 8-bits color bitmap as FITS", "[FITS]")
{
    UInt8ColorBitmap bitmap(9, 3);

    uint8_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint8_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            for (unsigned int i = 0; i < bitmap.channels(); ++i)
            {
                if (x < 3)
                    v = 0x00;
                else if (x < 6)
                    v = 0x80;
                else
                    v = 0xF0;

                v = (v & 0xF0) >> (2 * i);
                v += y;

                data[x * 3 + i] = v;
            }
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "rgb8bits.fits"));
    REQUIRE(output.write(&bitmap));

    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "rgb8bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt8ColorBitmap* bitmap3 = dynamic_cast<UInt8ColorBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint8_t* ptr = bitmap3->data();
    uint8_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save 16-bits color bitmap as FITS", "[FITS]")
{
    UInt16ColorBitmap bitmap(9, 3);

    uint16_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint16_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            for (unsigned int i = 0; i < bitmap.channels(); ++i)
            {
                if (x < 3)
                    v = 0x0000;
                else if (x < 6)
                    v = 0x8880;
                else
                    v = 0xFFF0;

                v = (v & 0xFFF0) >> (4 * i);
                v += y;

                data[x * 3 + i] = v;
            }
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "rgb16bits.fits"));
    REQUIRE(output.write(&bitmap));

    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "rgb16bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt16ColorBitmap* bitmap3 = dynamic_cast<UInt16ColorBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint16_t* ptr = bitmap3->data();
    uint16_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save 32-bits color bitmap as FITS", "[FITS]")
{
    UInt32ColorBitmap bitmap(9, 3);

    uint32_t v = 0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        uint32_t* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            for (unsigned int i = 0; i < bitmap.channels(); ++i)
            {
                if (x < 3)
                    v = 0x00000000;
                else if (x < 6)
                    v = 0x88888880;
                else
                    v = 0xFFFFFFF0;

                v = (v & 0xFFFFF000) >> (4 * i);
                v += y;

                data[x * 3 + i] = v;
            }
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "rgb32bits.fits"));
    REQUIRE(output.write(&bitmap));

    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "rgb32bits.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    UInt32ColorBitmap* bitmap3 = dynamic_cast<UInt32ColorBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    uint32_t* ptr = bitmap3->data();
    uint32_t* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save float color bitmap as FITS", "[FITS]")
{
    FloatColorBitmap bitmap(9, 3);

    float v = 0.0f;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        float* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            for (unsigned int i = 0; i < bitmap.channels(); ++i)
            {
                data[x * 3 + i] = v;
                v += 1.0f / (18.0f * 3.0f);
            }
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "rgbfloat.fits"));
    REQUIRE(output.write(&bitmap));

    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "rgbfloat.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    FloatColorBitmap* bitmap3 = dynamic_cast<FloatColorBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    float* ptr = bitmap3->data();
    float* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}


TEST_CASE("Save double color bitmap as FITS", "[FITS]")
{
    DoubleColorBitmap bitmap(9, 3);

    double v = 0.0;
    for (unsigned int y = 0; y < bitmap.height(); ++y)
    {
        double* data = bitmap.data(y);

        for (unsigned int x = 0; x < bitmap.width(); ++x)
        {
            for (unsigned int i = 0; i < bitmap.channels(); ++i)
            {
                data[x * 3 + i] = v;
                v += 1.0 / (18.0 * 3.0);
            }
        }
    }

    FITS output;
    REQUIRE(output.create(TEMP_DIR "rgbdouble.fits"));
    REQUIRE(output.write(&bitmap));

    output.close();

    FITS input;
    REQUIRE(input.open(TEMP_DIR "rgbdouble.fits"));

    Bitmap* bitmap2 = input.readBitmap();
    REQUIRE(bitmap2);

    DoubleColorBitmap* bitmap3 = dynamic_cast<DoubleColorBitmap*>(bitmap2);
    REQUIRE(bitmap3);

    REQUIRE(bitmap3->width() == bitmap.width());
    REQUIRE(bitmap3->height() == bitmap.height());
    REQUIRE(bitmap3->size() == bitmap.size());

    double* ptr = bitmap3->data();
    double* ref = bitmap.data();

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * bitmap.channels(); ++i)
        REQUIRE(ptr[i] == ref[i]);

    delete bitmap3;
}
