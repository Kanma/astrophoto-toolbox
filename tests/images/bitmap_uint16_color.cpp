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
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 3, 6, 60, 480, image2, 257.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 65535.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint16_t, float>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0);
            image.set(image2);
            checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 65535.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint16_t, double>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 30);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint16_t, uint16_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another color bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt16ColorBitmap image2(image);
            checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 3, 6, 120, 1200, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt16ColorBitmap image2(image, false);
            checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 3, 6, 120, 1200, &image, 1.0);
        }
    }

    SECTION("creation from another gray bitmap, 32bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8, 52);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint16_t, uint32_t>(&image, 10, 8, 3, 6, 60, 480, image2, 1.0 / 65537.0);
        }

        delete image2;
    }

    SECTION("retrieve individual channels")
    {
        for (unsigned int c = 0; c < 3; ++c)
        {
            Bitmap* channel = image.channel(c);
            checkBitmapIsChannel<uint16_t>(channel, 20, 10, c, 2, 40, 400, &image);
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
}
