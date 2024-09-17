/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "bitmap_helpers.h"


TEST_CASE("UInt16 color bitmap", "[Bitmap]")
{
    UInt16ColorBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 3);
    REQUIRE(image.bytesPerPixel() == 6);
    REQUIRE(image.bytesPerRow() == 120);
    REQUIRE(image.size() == 1200);
    REQUIRE(image.range() == RANGE_USHORT);
    REQUIRE(image.space() == SPACE_LINEAR);

    uint16_t *data = image.data();
    REQUIRE(data);

    uint16_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 * 3 + 10 * 3);

    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
        REQUIRE(data[i] == 0);

    uint16_t v = 0;
    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
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
        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        SECTION("with valid ranges, source range as uint")
        {
            Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createUInt32Bitmap(true, 10, 8, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createUInt32Bitmap(true, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap, float")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 65535.0, RANGE_USHORT);
            }

            delete image2;
 
            image2 = createFloatBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }
 
            delete image2;

            image2 = createFloatBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            delete image2;

            image2 = createFloatBitmap(true, 10, 8, 1.0f, 255.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(true, 10, 8, 256.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createFloatBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with valid ranges")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 65535.0, RANGE_USHORT);
            }

            delete image2;
 
            image2 = createDoubleBitmap(true, 10, 8, 1000.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }
 
            delete image2;

            image2 = createDoubleBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            delete image2;

            image2 = createDoubleBitmap(true, 10, 8, 1.0f, 255.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0f);

            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0f, 0, RANGE_BYTE);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createDoubleBitmap(true, 10, 8, 256.0f, 65535.0f, 0, RANGE_USHORT);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;

            image2 = createDoubleBitmap(true, 10, 8, 1000000.0f, 4294967295.0f, 0, RANGE_UINT);

            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type")
    {
        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 0, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        SECTION("with valid ranges, source range as ushort")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 32);

            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 257.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with valid ranges, source range as byte")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 32, RANGE_BYTE);

            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_SOURCE, RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_BYTE);
            }

            delete image2;
        }

        SECTION("with invalid ranges")
        {
            Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 32);

            for (range_t range : {RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));

            delete image2;
        }
    }

    SECTION("creation from another color bitmap of the same type")
    {
        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_SOURCE, RANGE_USHORT})
            {
                UInt16ColorBitmap image2(image, range);
                checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 3, 6, 120, 1200, &image, 1.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                UInt16ColorBitmap image2(image, range);
                checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 3, 6, 120, 1200, &image, 1.0 / 257.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_UINT, RANGE_ONE})
            {
                UInt16ColorBitmap image2(image, range);
                REQUIRE(image2.width() == 0);
            }
        }
    }

    SECTION("creation from another gray bitmap, 32bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8, 52);

        SECTION("with valid ranges")
        {
            for (range_t range : {RANGE_DEST, RANGE_USHORT})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0, RANGE_USHORT);
            }

            for (range_t range : {RANGE_BYTE})
            {
                REQUIRE(image.set(image2, range));
                checkBitmapIdenticalChannels<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 16843009.0, RANGE_BYTE);
            }
        }

        SECTION("with invalid ranges")
        {
            for (range_t range : {RANGE_SOURCE, RANGE_UINT, RANGE_ONE})
                REQUIRE(!image.set(image2, range));
        }

        delete image2;
    }

    SECTION("retrieve individual channels")
    {
        for (unsigned int c = 0; c < 3; ++c)
        {
            Bitmap* channel = image.channel(c);
            checkBitmapIsChannel<uint16_t>(channel, 20, 10, c, 2, 40, 400, &image, RANGE_USHORT);
            delete channel;
        }
    }

    SECTION("fail to retrieve invalid channel")
    {
        REQUIRE(image.channel(3) == nullptr);
    }

    SECTION("set individual channels")
    {
        UInt16GrayBitmap channels[3];

        for (unsigned int c = 0; c < 3; ++c)
        {
            channels[c].resize(20, 10);
            uint16_t* data = channels[c].data();
            for (unsigned int i = 0; i < channels[c].width() * channels[c].height(); ++i)
                data[i] = c;

            REQUIRE(image.setChannel(c, &channels[c]));
        }

        for (unsigned int c = 0; c < 3; ++c)
            checkChannelOfBitmap<uint16_t>(&image, 20, 10, c, 6, 120, 1200, &channels[c]);
    }

    SECTION("fail to set invalid channel")
    {
        UInt16GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(4, &channel));
    }

    SECTION("fail to set null channel")
    {
        REQUIRE(!image.setChannel(0, nullptr));
    }

    SECTION("fail to set channel with different size")
    {
        UInt16GrayBitmap channel(10, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }

    SECTION("fail to set channel with different type")
    {
        UInt8GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }

    SECTION("set from another sRGB color bitmap")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_sRGB);

        SECTION("destination: linear")
        {
            Bitmap* image3 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_LINEAR);

            REQUIRE(image.set(image2));
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image3, 1.0, RANGE_USHORT, SPACE_LINEAR, 2);

            delete image3;
        }

        SECTION("destination: sRGB")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT, SPACE_sRGB);
        }

        delete image2;
    }

    SECTION("set from another linear color bitmap")
    {
        image.setSpace(SPACE_sRGB, false);

        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_LINEAR);

        SECTION("destination: sRGB")
        {
            Bitmap* image3 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_sRGB);

            REQUIRE(image.set(image2));
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image3, 1.0, RANGE_USHORT, SPACE_sRGB);

            delete image3;
        }

        SECTION("destination: linear")
        {
            REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0, RANGE_USHORT, SPACE_LINEAR);
        }

        delete image2;
    }

    SECTION("convert to sRGB")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_LINEAR);
        Bitmap* image3 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_sRGB);

        REQUIRE(image.set(image2));
        REQUIRE(image.setSpace(SPACE_sRGB));

        checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image3, 1.0, RANGE_USHORT, SPACE_sRGB);

        delete image2;
        delete image3;
    }

    SECTION("convert to linear")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_sRGB);
        Bitmap* image3 = createUInt16Bitmap(true, 10, 8, 0, RANGE_USHORT, SPACE_LINEAR);

        REQUIRE(image.set(image2, RANGE_DEST, SPACE_SOURCE));
        REQUIRE(image.setSpace(SPACE_LINEAR));
        checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image3, 1.0, RANGE_USHORT, SPACE_LINEAR, 2);

        delete image2;
        delete image3;
    }
}
