#include "bitmap_helpers.h"


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
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8, 60);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 257.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 257.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 16843009.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 16843009.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint8_t, float>(&image, 10, 8, 3, 3, 30, 240, image2, 255.0f);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(true, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint8_t, float>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0f);
            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 0.05, 1.0);
            image.set(image2);
            checkBitmap<uint8_t, double>(&image, 10, 8, 3, 3, 30, 240, image2, 255.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint8_t, double>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another gray bitmap of the same type")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 60);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint8_t, uint8_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another color bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt8ColorBitmap image2(image);
            checkBitmap<uint8_t, uint8_t>(&image2, 20, 10, 3, 3, 60, 600, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt8ColorBitmap image2(image, false);
            checkBitmap<uint8_t, uint8_t>(&image2, 20, 10, 3, 3, 60, 600, &image, 1.0);
        }
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 30);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<uint8_t, uint16_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 257.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<uint8_t, uint16_t>(&image, 10, 8, 3, 3, 30, 240, image2, 1.0 / 257.0);
        }

        delete image2;
    }

    SECTION("retrieve individual channels")
    {
        for (unsigned int c = 0; c < 3; ++c)
        {
            Bitmap* channel = image.channel(c);
            checkBitmapIsChannel<uint8_t>(channel, 20, 10, c, 1, 20, 200, &image);
            delete channel;
        }
    }

    SECTION("fail to retrieve invalid channel")
    {
        REQUIRE(image.channel(3) == nullptr);
    }

    SECTION("set individual channels")
    {
        UInt8GrayBitmap channels[3];

        for (unsigned int c = 0; c < 3; ++c)
        {
            channels[c].resize(20, 10);
            uint8_t* data = channels[c].ptr();
            for (unsigned int i = 0; i < channels[c].size(); ++i)
                data[i] = c;

            REQUIRE(image.setChannel(c, &channels[c]));
        }

        for (unsigned int c = 0; c < 3; ++c)
            checkChannelOfBitmap<uint8_t>(&image, 20, 10, c, 3, 60, 600, &channels[c]);
    }

    SECTION("fail to set invalid channel")
    {
        UInt8GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(4, &channel));
    }

    SECTION("fail to set null channel")
    {
        REQUIRE(!image.setChannel(0, nullptr));
    }

    SECTION("fail to set channel with different size")
    {
        UInt8GrayBitmap channel(10, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }

    SECTION("fail to set channel with different type")
    {
        UInt16GrayBitmap channel(20, 10);
        REQUIRE(!image.setChannel(0, &channel));
    }
}
