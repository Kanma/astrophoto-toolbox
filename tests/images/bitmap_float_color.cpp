#include "bitmap_helpers.h"


TEST_CASE("Float color bitmap", "[Bitmap]")
{
    FloatColorBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.bytesPerPixel() == 12);
    REQUIRE(image.bytesPerRow() == 240);
    REQUIRE(image.size() == 2400);
    REQUIRE(image.range() == RANGE_ONE);
    REQUIRE(image.space() == SPACE_LINEAR);

    float* data = image.data();
    REQUIRE(data);

    float* ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 * 3 + 10 * 3);

    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
        REQUIRE(data[i] == 0.0f);

    float v = 0;
    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
    {
        data[i] = v;
        v += 1.0f;
    }

    SECTION("set from a buffer")
    {
        float buffer[10 * 8 * 3];

        float v = 255.0f;
        for (size_t i = 0; i < 10 * 8 * 3; ++i)
        {
            buffer[i] = v;
            v -= 1.0f;
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
        SECTION("with valid ranges, source range as 0-1")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65535.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        SECTION("with valid ranges, source range as 0-1")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 132);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65535.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 132, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000.0f, 65535.0f, 132, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1000.0f, 65535.0f, 132, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        for (range_t range : {RANGE_DEST, RANGE_ONE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
        }

        for (range_t range : {RANGE_UINT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
        }

        for (range_t range : {RANGE_USHORT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
        }

        for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        for (range_t range : {RANGE_DEST, RANGE_ONE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
        }

        for (range_t range : {RANGE_UINT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
        }

        for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
        }

        for (range_t range : {RANGE_BYTE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        for (range_t range : {RANGE_DEST, RANGE_ONE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
        }

        for (range_t range : {RANGE_SOURCE, RANGE_UINT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
        }

        for (range_t range : {RANGE_USHORT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
        }

        for (range_t range : {RANGE_BYTE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with valid ranges, source range as 0-1")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 65535.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type")
    {
        SECTION("with valid ranges, source range as 0-1")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65535.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000.0f, 65535.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        SECTION("with valid ranges, source range as 0-1")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f, 52);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65535.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000000.0f, 4294967295.0f, 52, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000.0f, 65535.0f, 52, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1000.0f, 65535.0f, 52, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_ONE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0, RANGE_ONE);
            }

            for (range_t range : {RANGE_UINT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0, RANGE_UINT);
            }

            for (range_t range : {RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }
    }

    SECTION("creation from another color bitmap of the same type")
    {
        for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_ONE})
        {
            FloatColorBitmap image2(image, range);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 1.0, RANGE_ONE);
        }

        for (range_t range : {RANGE_UINT})
        {
            FloatColorBitmap image2(image, range);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 4294967295.0, RANGE_UINT);
        }

        for (range_t range : {RANGE_USHORT})
        {
            FloatColorBitmap image2(image, range);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 65535.0, RANGE_USHORT);
        }

        for (range_t range : {RANGE_BYTE})
        {
            FloatColorBitmap image2(image, range);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 255.0, RANGE_BYTE);
        }
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 52);

        for (range_t range : {RANGE_DEST, RANGE_ONE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0, RANGE_ONE);
        }

        for (range_t range : {RANGE_UINT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0, RANGE_UINT);
        }

        for (range_t range : {RANGE_SOURCE, RANGE_USHORT})
        {
            REQUIRE(image.set(image2, range));
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_USHORT);
        }

        for (range_t range : {RANGE_BYTE})
        {
            REQUIRE(image.set(image2, range));
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 257.0, RANGE_BYTE);
        }

        delete image2;
    }

    SECTION("set from another sRGB color bitmap")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_sRGB);

        SECTION("destination: linear")
        {
            Bitmap* image3 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_LINEAR);

            REQUIRE(image.set(image2));
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image3, 1.0, RANGE_ONE, SPACE_LINEAR, 1e-6f);

            delete image3;
        }

        SECTION("destination: sRGB")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE, SPACE_sRGB);
        }

        delete image2;
    }

    SECTION("set from another linear color bitmap")
    {
        image.setSpace(SPACE_sRGB, false);

        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_LINEAR);

        SECTION("destination: sRGB")
        {
            Bitmap* image3 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_sRGB);

            REQUIRE(image.set(image2));
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image3, 1.0, RANGE_ONE, SPACE_sRGB);

            delete image3;
        }

        SECTION("destination: linear")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0, RANGE_ONE, SPACE_LINEAR);
        }

        delete image2;
    }

    SECTION("convert to sRGB")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_LINEAR);
        Bitmap* image3 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_sRGB);

        REQUIRE(image.set(image2));
        REQUIRE(image.setSpace(SPACE_sRGB));

        checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image3, 1.0, RANGE_ONE, SPACE_sRGB);

        delete image2;
        delete image3;
    }

    SECTION("convert to linear")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_ONE, SPACE_sRGB);
        Bitmap* image3 = createFloatBitmap(true, 10, 8,0.05f, 1.0f,  0, RANGE_ONE, SPACE_LINEAR);

        REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
        REQUIRE(image.setSpace(SPACE_LINEAR));
        checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image3, 1.0, RANGE_ONE, SPACE_LINEAR, 1e-6f);

        delete image2;
        delete image3;
    }
}
