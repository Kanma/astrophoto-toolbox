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
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 255.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint8_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 4294967295.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint32_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap, double")
    {
        Bitmap* image2 = createDoubleBitmap(true, 10, 8, 1.0, 255.0);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, double>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type")
    {
        Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f, 52);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<float, float>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another color bitmap of the same type")
    {
        SECTION("with scaling")
        {
            FloatColorBitmap image2(image);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            FloatColorBitmap image2(image, false);
            checkBitmap<float, float>(&image2, 20, 10, 3, 12, 240, 2400, &image, 1.0);
        }
    }

    SECTION("creation from another gray bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 30);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0 / 65535.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmapIdenticalChannels<float, uint16_t>(&image, 10, 8, 3, 12, 120, 960, image2, 1.0);
        }

        delete image2;
    }
}
