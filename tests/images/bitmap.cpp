#include <catch.hpp>
#include <astrophoto-toolbox/images/bitmap.h>

using namespace astrophototoolbox;

TEST_CASE("UInt8 color bitmap", "[Bitmap]")
{
    UInt8ColorBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.bytesPerPixel() == 3);
    REQUIRE(image.bytesPerRow() == 60);
    REQUIRE(image.size() == 600);

    auto info = image.info();
    REQUIRE(info.isoSpeed == 0);
    REQUIRE(info.shutterSpeed == Approx(0.0f));
    REQUIRE(info.aperture == Approx(0.0f));
    REQUIRE(info.focalLength == Approx(0.0f));

    uint8_t *data = image.data();
    REQUIRE(data);

    uint8_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 60 + 10 * 3);

    for (size_t i = 0; i < image.size(); ++i)
        REQUIRE(data[i] == 0);

    uint8_t v = 0;
    for (size_t i = 0; i < image.size(); ++i)
    {
        data[i] = v;
        ++v;
    }

    SECTION("set from a buffer")
    {
        uint8_t buffer[10 * 8 * 3];

        uint8_t v = 255;
        for (size_t i = 0; i < 10 * 8 * 3; ++i)
        {
            buffer[i] = v;
            --v;
        }

        image.set(buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        for (size_t i = 0; i < image.size(); ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt8ColorBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.size(); ++i)
        {
            data2[i] = v;
            --v;
        }

        auto& info2 = image2.info();
        info2.isoSpeed = 400;
        info2.shutterSpeed = 2.0f;
        info2.aperture = 4.5f;
        info2.focalLength = 20.0f;

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        auto info = image.info();
        REQUIRE(info.isoSpeed == 400);
        REQUIRE(info.shutterSpeed == Approx(2.0f));
        REQUIRE(info.aperture == Approx(4.5f));
        REQUIRE(info.focalLength == Approx(20.0f));

        data = image.data();
        for (size_t i = 0; i < image.size(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt8ColorBitmap image2(10, 8, 60);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.size(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width() * 3; ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        UInt16ColorBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == (data2[i] >> 8));
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        UInt32ColorBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == data2[i] >> 24);
    }

    SECTION("set from another color bitmap, float")
    {
        FloatColorBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0f / 255.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        uint8_t p = 255;
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i, --p)
            REQUIRE(data[i] == p);
    }

    SECTION("set from another color bitmap, double")
    {
        DoubleColorBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0 / 255.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        uint8_t p = 255;
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i, --p)
            REQUIRE(data[i] == p);
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt8GrayBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.size(); i += 3, ++j)
        {
            REQUIRE(data[i] == data2[j]);
            REQUIRE(data[i + 1] == data2[j]);
            REQUIRE(data[i + 2] == data2[j]);
        }
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt8GrayBitmap image2(10, 8, 60);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow(); i += 3, ++j)
            {
                REQUIRE(data[i] == data2[j]);
                REQUIRE(data[i + 1] == data2[j]);
                REQUIRE(data[i + 2] == data2[j]);
            }
        }
    }

    SECTION("creation from another color bitmap of the same type")
    {
        UInt8ColorBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image2.channels() == 3);
        REQUIRE(image2.bytesPerPixel() == 3);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint8_t* data2 = image2.data();
        for (size_t i = 0; i < image2.size(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        UInt16GrayBitmap image2(10, 8, 30);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 3);
        REQUIRE(image.bytesPerRow() == 30);
        REQUIRE(image.size() == 240);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width() * 3; i += 3, ++j)
            {
                uint8_t p = (data2[j] >> 8);
                REQUIRE(data[i] == p);
                REQUIRE(data[i + 1] == p);
                REQUIRE(data[i + 2] == p);
            }
        }
    }
}

TEST_CASE("UInt16 color bitmap", "[Bitmap]")
{
    UInt16ColorBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.bytesPerPixel() == 6);
    REQUIRE(image.bytesPerRow() == 120);
    REQUIRE(image.size() == 1200);

    uint16_t *data = image.data();
    REQUIRE(data);

    uint16_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 * 3 + 10 * 3);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0);

    uint16_t v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 0x100;
    }

    SECTION("set from a buffer")
    {
        uint16_t buffer[10 * 8 * 3];

        uint16_t v = 0xFF00;
        for (size_t i = 0; i < 10 * 8 * 3; ++i)
        {
            buffer[i] = v;
            v -= 0x100;
        }

        image.set((uint8_t*) buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt16ColorBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt16ColorBitmap image2(10, 8, 60);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width() * 3; ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        UInt8ColorBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 0xFF;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == (data2[i] << 8));
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        UInt32ColorBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == data2[i] >> 16);
    }

    SECTION("set from another color bitmap, float")
    {
        FloatColorBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0f / 65535.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == uint16_t(data2[i] * 65535.0f));
    }

    SECTION("set from another color bitmap, double")
    {
        DoubleColorBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0 / 65535.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == uint16_t(data2[i] * 65535.0));
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt16GrayBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.height() * image.width() * 3; i += 3, ++j)
        {
            REQUIRE(data[i] == data2[j]);
            REQUIRE(data[i + 1] == data2[j]);
            REQUIRE(data[i + 2] == data2[j]);
        }
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt16GrayBitmap image2(10, 8, 60);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow() / 2; i += 3, ++j)
            {
                REQUIRE(data[i] == data2[j]);
                REQUIRE(data[i + 1] == data2[j]);
                REQUIRE(data[i + 2] == data2[j]);
            }
        }
    }

    SECTION("creation from another color bitmap of the same type")
    {
        UInt16ColorBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image2.channels() == 3);
        REQUIRE(image2.bytesPerPixel() == 6);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint16_t *data2 = image2.data();
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another gray bitmap, 32bits integer, with extra row bytes")
    {
        UInt32GrayBitmap image2(10, 8, 52);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 6);
        REQUIRE(image.bytesPerRow() == 60);
        REQUIRE(image.size() == 480);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width() * 3; i += 3, ++j)
            {
                uint16_t p = (data2[j] >> 16);
                REQUIRE(data[i] == p);
                REQUIRE(data[i + 1] == p);
                REQUIRE(data[i + 2] == p);
            }
        }
    }
}

TEST_CASE("UInt32 color bitmap", "[Bitmap]")
{
    UInt32ColorBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.bytesPerPixel() == 12);
    REQUIRE(image.bytesPerRow() == 240);
    REQUIRE(image.size() == 2400);

    uint32_t *data = image.data();
    REQUIRE(data);

    uint32_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 * 3 + 10 * 3);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0);

    uint32_t v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 0x1000000;
    }

    SECTION("set from a buffer")
    {
        uint32_t buffer[10 * 8 * 3];

        uint32_t v = 0xFF00;
        for (size_t i = 0; i < 10 * 8 * 3; ++i)
        {
            buffer[i] = v;
            v -= 0x1000000;
        }

        image.set((uint8_t*) buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt32ColorBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt32ColorBitmap image2(10, 8, 140);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width() * 3; ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        UInt8ColorBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 0xFF;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == (data2[i] << 24));
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        UInt16ColorBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == data2[i] << 16);
    }

    SECTION("set from another color bitmap, float")
    {
        FloatColorBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0f / 4294967295.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == uint32_t(data2[i] * 4294967295.0f));
    }

    SECTION("set from another color bitmap, double")
    {
        DoubleColorBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 1.0 / 4294967295.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
            REQUIRE(data[i] == uint32_t(data2[i] * 4294967295.0));
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt32GrayBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.height() * image.width() * 3; i += 3, ++j)
        {
            REQUIRE(data[i] == data2[j]);
            REQUIRE(data[i + 1] == data2[j]);
            REQUIRE(data[i + 2] == data2[j]);
        }
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt32GrayBitmap image2(10, 8, 60);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow() / 4; i += 3, ++j)
            {
                REQUIRE(data[i] == data2[j]);
                REQUIRE(data[i + 1] == data2[j]);
                REQUIRE(data[i + 2] == data2[j]);
            }
        }
    }

    SECTION("creation from another color bitmap of the same type")
    {
        UInt32ColorBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image2.channels() == 3);
        REQUIRE(image2.bytesPerPixel() == 12);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint32_t *data2 = image2.data();
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        UInt16GrayBitmap image2(10, 8, 52);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 3);
        REQUIRE(image.bytesPerPixel() == 12);
        REQUIRE(image.bytesPerRow() == 120);
        REQUIRE(image.size() == 960);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width() * 3; i += 3, ++j)
            {
                uint32_t p = (data2[j] << 16);
                REQUIRE(data[i] == p);
                REQUIRE(data[i + 1] == p);
                REQUIRE(data[i + 2] == p);
            }
        }
    }
}

TEST_CASE("UInt8 gray bitmap", "[Bitmap]")
{
    UInt8GrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 1);
    REQUIRE(image.bytesPerRow() == 20);
    REQUIRE(image.size() == 200);

    uint8_t *data = image.data();
    REQUIRE(data);

    uint8_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 + 10);

    for (size_t i = 0; i < image.size(); ++i)
        REQUIRE(data[i] == 0);

    uint8_t v = 0;
    for (size_t i = 0; i < image.size(); ++i)
    {
        data[i] = v;
        ++v;
    }

    SECTION("set from a buffer")
    {
        uint8_t buffer[10 * 8];

        uint8_t v = 255;
        for (size_t i = 0; i < 10 * 8; ++i)
        {
            buffer[i] = v;
            --v;
        }

        image.set(buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        for (size_t i = 0; i < image.size(); ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt8GrayBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.size(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        for (size_t i = 0; i < image.size(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt8GrayBitmap image2(10, 8, 60);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.size(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width(); ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        UInt16GrayBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == (data2[i] >> 8));
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        UInt32GrayBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == data2[i] >> 24);
    }

    SECTION("set from another gray bitmap, float")
    {
        FloatGrayBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0f / 255.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        uint8_t p = 255;
        for (size_t i = 0; i < image.height() * image.width(); ++i, --p)
            REQUIRE(data[i] == p);
    }

    SECTION("set from another color bitmap, double")
    {
        DoubleGrayBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0 / 255.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        uint8_t p = 255;
        for (size_t i = 0; i < image.height() * image.width(); ++i, --p)
            REQUIRE(data[i] == p);
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt8ColorBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.size(); ++i, j += 3)
            REQUIRE(data[i] == uint8_t(float(data2[j] + data2[j + 1] + data2[j + 2]) / 3));
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt8ColorBitmap image2(10, 8, 60);

        uint8_t* data2 = image2.data();
        uint8_t v = 255;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow(); ++i, j += 3)
                REQUIRE(data[i] == uint8_t(float(data2[j] + data2[j + 1] + data2[j + 2]) / 3));
        }
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        UInt8GrayBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image.channels() == 1);
        REQUIRE(image2.bytesPerPixel() == 1);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint8_t* data2 = image2.data();
        for (size_t i = 0; i < image2.size(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        UInt16ColorBitmap image2(10, 8, 72);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 1);
        REQUIRE(image.bytesPerRow() == 10);
        REQUIRE(image.size() == 80);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width(); ++i, j += 3)
                REQUIRE(data[i] == uint16_t(float(data2[j] + data2[j + 1] + data2[j + 2]) / 3) >> 8);
        }
    }
}

TEST_CASE("UInt16 gray bitmap", "[Bitmap]")
{
    UInt16GrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 2);
    REQUIRE(image.bytesPerRow() == 40);
    REQUIRE(image.size() == 400);

    uint16_t *data = image.data();
    REQUIRE(data);

    uint16_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 + 10);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0);

    uint16_t v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 0x100;
    }

    SECTION("set from a buffer")
    {
        uint16_t buffer[10 * 8];

        uint16_t v = 0xFF00;
        for (size_t i = 0; i < 10 * 8; ++i)
        {
            buffer[i] = v;
            v -= 0x100;
        }

        image.set((uint8_t*) buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt16GrayBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt16GrayBitmap image2(10, 8, 30);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width(); ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another gray bitmap, 8bits integer")
    {
        UInt8GrayBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 0xFF;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == (data2[i] << 8));
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        UInt32GrayBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == data2[i] >> 16);
    }

    SECTION("set from another gray bitmap, float")
    {
        FloatGrayBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0f / 65535.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == uint16_t(data2[i] * 65535.0f));
    }

    SECTION("set from another gray bitmap, double")
    {
        DoubleGrayBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0 / 65535.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == uint16_t(data2[i] * 65535.0));
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt16ColorBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.height() * image.width(); ++i, j += 3)
            REQUIRE(data[i] == uint16_t(float(data2[j] + data2[j + 1] + data2[j + 2]) / 3));
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt16ColorBitmap image2(10, 8, 60);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow() / 2; ++i, j += 3)
                REQUIRE(data[i] == uint16_t(float(data2[j] + data2[j + 1] + data2[j + 2]) / 3));
        }
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        UInt16GrayBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image2.channels() == 1);
        REQUIRE(image2.bytesPerPixel() == 2);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint16_t *data2 = image2.data();
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another color bitmap, 32bits integer, with extra row bytes")
    {
        UInt32ColorBitmap image2(10, 8, 132);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 2);
        REQUIRE(image.bytesPerRow() == 20);
        REQUIRE(image.size() == 160);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width(); ++i, j += 3)
                REQUIRE(data[i] == uint32_t((float(data2[j]) + float(data2[j + 1]) + float(data2[j + 2])) / 3) >> 16);
        }
    }
}

TEST_CASE("UInt32 gray bitmap", "[Bitmap]")
{
    UInt32GrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 4);
    REQUIRE(image.bytesPerRow() == 80);
    REQUIRE(image.size() == 800);

    uint32_t *data = image.data();
    REQUIRE(data);

    uint32_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 + 10);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0);

    uint32_t v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 0x1000000;
    }

    SECTION("set from a buffer")
    {
        uint32_t buffer[10 * 8];

        uint32_t v = 0xFF00;
        for (size_t i = 0; i < 10 * 8; ++i)
        {
            buffer[i] = v;
            v -= 0x1000000;
        }

        image.set((uint8_t*) buffer, 10, 8);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == buffer[i]);
    }

    SECTION("set from another gray bitmap of the same type")
    {
        UInt32GrayBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        UInt32GrayBitmap image2(10, 8, 52);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0; i < image.width(); ++i)
                REQUIRE(data[i] == data2[i]);
        }
    }

    SECTION("set from another gray bitmap, 8bits integer")
    {
        UInt8GrayBitmap image2(10, 8);

        uint8_t* data2 = image2.data();
        uint8_t v = 0xFF;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            --v;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == (data2[i] << 24));
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        UInt16GrayBitmap image2(10, 8);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == data2[i] << 16);
    }

    SECTION("set from another gray bitmap, float")
    {
        FloatGrayBitmap image2(10, 8);

        float *data2 = image2.data();
        float v = 1.0f;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0f / 4294967295.0f;
            if (v < 0.0f)
                v = 1.0f;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == uint32_t(data2[i] * 4294967295.0f));
    }

    SECTION("set from another gray bitmap, double")
    {
        DoubleGrayBitmap image2(10, 8);

        double *data2 = image2.data();
        double v = 1.0;
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
        {
            data2[i] = v;
            v -= 1.0 / 4294967295.0;
            if (v < 0.0)
                v = 1.0;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0; i < image.height() * image.width(); ++i)
            REQUIRE(data[i] == uint32_t(data2[i] * 4294967295.0));
    }

    SECTION("set from another color bitmap of the same type")
    {
        UInt32ColorBitmap image2(10, 8);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.width() * 3; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        data = image.data();
        for (size_t i = 0, j = 0; i < image.height() * image.width(); ++i, j += 3)
            REQUIRE(data[i] == uint32_t((float(data2[j]) + float(data2[j + 1]) + float(data2[j + 2])) / 3));
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        UInt32ColorBitmap image2(10, 8, 132);

        uint32_t *data2 = image2.data();
        uint32_t v = 0xFF000000;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 4; ++i)
        {
            data2[i] = v;
            v -= 0x1000000;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.bytesPerRow() / 4; ++i, j += 3)
                REQUIRE(data[i] == uint32_t(double(uint64_t(data2[j]) + uint64_t(data2[j + 1]) + uint64_t(data2[j + 2])) / 3.0));
        }
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        UInt32GrayBitmap image2(image);

        REQUIRE(image2.width() == image.width());
        REQUIRE(image2.height() == image.height());
        REQUIRE(image2.channels() == 1);
        REQUIRE(image2.bytesPerPixel() == 4);
        REQUIRE(image2.bytesPerRow() == image.bytesPerRow());
        REQUIRE(image2.size() == image.size());

        data = image.data();
        uint32_t *data2 = image2.data();
        for (size_t i = 0; i < image2.height() * image2.width(); ++i)
            REQUIRE(data[i] == data2[i]);
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        UInt16ColorBitmap image2(10, 8, 70);

        uint16_t *data2 = image2.data();
        uint16_t v = 0xFF00;
        for (size_t i = 0; i < image2.height() * image2.bytesPerRow() / 2; ++i)
        {
            data2[i] = v;
            v -= 0x100;
        }

        image.set(&image2);

        REQUIRE(image.width() == 10);
        REQUIRE(image.height() == 8);
        REQUIRE(image.channels() == 1);
        REQUIRE(image.bytesPerPixel() == 4);
        REQUIRE(image.bytesPerRow() == 40);
        REQUIRE(image.size() == 320);

        for (size_t y = 0; y < image.height(); ++y)
        {
            data = image.data(y);
            data2 = image2.data(y);

            for (size_t i = 0, j = 0; i < image.width(); ++i, j += 3)
                REQUIRE(data[i] == uint32_t(double((uint64_t(data2[j]) + uint64_t(data2[j + 1]) + uint64_t(data2[j + 2])) << 16) / 3.0));
        }
    }
}
