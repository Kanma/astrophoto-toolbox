#include "bitmap_helpers.h"


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

    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
        REQUIRE(data[i] == 0);

    uint32_t v = 0;
    for (size_t i = 0; i < image.height() * image.width() * 3; ++i)
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
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 16843009.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint32_t, float>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint32_t, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05f, 1.0);
            image.set(image2);
            checkBitmap<uint32_t, double>(&image, 10, 8, 3, 12, 120, 960, image2, 4294967295.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint32_t, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8, 52);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint32_t, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another color bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt32ColorBitmap image2(image);
            checkBitmap<uint32_t, uint32_t>(&image2, 20, 10, 3, 12, 240, 2400, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt32ColorBitmap image2(image, false);
            checkBitmap<uint32_t, uint32_t>(&image2, 20, 10, 3, 12, 240, 2400, &image, 1.0);
        }
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 30);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint32_t, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 65537.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint32_t, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("retrieve individual channels")
    {
        for (unsigned int c = 0; c < 3; ++c)
        {
            Bitmap* channel = image.channel(c);
            checkBitmapIsChannel<uint32_t>(channel, 20, 10, c, 4, 80, 800, &image);
            delete channel;
        }
    }

    SECTION("fail to retrieve invalid channel")
    {
        REQUIRE(image.channel(3) == nullptr);
    }

    SECTION("set individual channels")
    {
        UInt32GrayBitmap channels[3];

        for (unsigned int c = 0; c < 3; ++c)
        {
            channels[c].resize(20, 10);
            uint32_t* data = channels[c].data();
            for (unsigned int i = 0; i < channels[c].width() * channels[c].height(); ++i)
                data[i] = c;

            REQUIRE(image.setChannel(c, &channels[c]));
        }

        for (unsigned int c = 0; c < 3; ++c)
            checkChannelOfBitmap<uint32_t>(&image, 20, 10, c, 12, 240, 2400, &channels[c]);
    }

    SECTION("fail to set invalid channel")
    {
        UInt32GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(4, &channel));
    }

    SECTION("fail to set null channel")
    {
        REQUIRE(!image.setChannel(0, nullptr));
    }

    SECTION("fail to set channel with different size")
    {
        UInt32GrayBitmap channel(10, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }

    SECTION("fail to set channel with different type")
    {
        UInt8GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }
}
