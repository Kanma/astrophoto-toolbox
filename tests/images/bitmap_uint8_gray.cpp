/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "bitmap_helpers.h"


TEST_CASE("UInt8 gray bitmap", "[Bitmap]")
{
    UInt8GrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 1);
    REQUIRE(image.bytesPerRow() == 20);
    REQUIRE(image.size() == 200);
    REQUIRE(image.range() == RANGE_BYTE);
    REQUIRE(image.space() == SPACE_LINEAR);

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
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 20);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;

            image2 = createUInt16Bitmap(false, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createUInt16Bitmap(false, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;

            image2 = createUInt32Bitmap(false, 10, 8, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;

            image2 = createUInt32Bitmap(false, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createUInt32Bitmap(false, 10, 80, 0, RANGE_USHORT);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createUInt32Bitmap(false, 10, 80, 0, RANGE_BYTE);

            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
         }
    }

    SECTION("set from another gray bitmap, float")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
 
            image2 = createFloatBitmap(false, 10, 8, 1.0f, 255.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
 
            delete image2;

            image2 = createFloatBitmap(false, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0, RANGE_BYTE);
            }
 
            delete image2;

            image2 = createFloatBitmap(false, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }
 
            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(false, 10, 8, 256.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(false, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 0.05, 1.0);

            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, double>(&image, 10, 8, 1, 1, 10, 80, image2, 255.0, RANGE_BYTE);
            }

            delete image2;
 
            image2 = createDoubleBitmap(false, 10, 8, 1.0, 255.0, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint8_t, double>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
 
            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 0.05, 1.0);

            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createDoubleBitmap(false, 10, 8, 0.05, 1.0, 0, RANGE_BYTE);

            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8, 42);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_BYTE})
            {
                UInt8GrayBitmap image2(image, range);
                checkBitmap<uint8_t, uint8_t>(&image2, 20, 10, 1, 1, 20, 200, &image, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_USHORT, RANGE_UINT, RANGE_ONE})
            {
                UInt8GrayBitmap image2(image, range);
                REQUIRE(image2.width() == 0);
            }
        }
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapMeanChannel<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_SOURCE, RANGE_USHORT, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("fail to change range")
    {
        REQUIRE(!image.setRange(RANGE_USHORT));
    }

    SECTION("set from another sRGB gray bitmap")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 0, SPACE_sRGB);

        SECTION("destination: linear")
        {
            Bitmap* image3 = createUInt8Bitmap(false, 10, 8, 0, SPACE_LINEAR);

            REQUIRE(image.set(image2));
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image3, 1.0, RANGE_BYTE, SPACE_LINEAR, 1);

            delete image3;
        }

        SECTION("destination: sRGB")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE, SPACE_sRGB, 0);
        }

        delete image2;
    }

    SECTION("set from another linear gray bitmap")
    {
        image.setSpace(SPACE_sRGB, false);

        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 0, SPACE_LINEAR);

        SECTION("destination: sRGB")
        {
            Bitmap* image3 = createUInt8Bitmap(false, 10, 8, 0, SPACE_sRGB);

            REQUIRE(image.set(image2));
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image3, 1.0, RANGE_BYTE, SPACE_sRGB, 1);

            delete image3;
        }

        SECTION("destination: linear")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0, RANGE_BYTE, SPACE_LINEAR, 0);
        }

        delete image2;
    }

    SECTION("convert to sRGB")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 0, SPACE_LINEAR);
        Bitmap* image3 = createUInt8Bitmap(false, 10, 8, 0, SPACE_sRGB);

        REQUIRE(image.set(image2));
        REQUIRE(image.setSpace(SPACE_sRGB));

        checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image3, 1.0, RANGE_BYTE, SPACE_sRGB);

        delete image2;
        delete image3;
    }

    SECTION("convert to linear")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 0, SPACE_sRGB);
        Bitmap* image3 = createUInt8Bitmap(false, 10, 8, 0, SPACE_LINEAR);

        REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
        REQUIRE(image.setSpace(SPACE_LINEAR));
        checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image3, 1.0, RANGE_BYTE, SPACE_LINEAR, 1);

        delete image2;
        delete image3;
    }
}
