#include "bitmap_helpers.h"


TEST_CASE("Float gray bitmap", "[Bitmap]")
{
    FloatGrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 4);
    REQUIRE(image.bytesPerRow() == 80);
    REQUIRE(image.size() == 800);

    float* data = image.data();
    REQUIRE(data);

    float* ptr = image.data(5);
    REQUIRE(ptr == data + 5 * 20);

    ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 + 10);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0.0f);

    float v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 1.0f;
    }

    SECTION("set from a buffer")
    {
        float buffer[10 * 8];

        float v = 255.0f;
        for (size_t i = 0; i < 10 * 8; ++i)
        {
            buffer[i] = v;
            v -= 1.0f;
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
        Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint8_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0 / 255.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint8_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0 / 65535.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0 / 4294967295.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, double")
    {
        Bitmap* image2 = createDoubleBitmap(false, 10, 8, 1.0, 255.0);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<float, double>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<float, double>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createFloatBitmap(true, 10, 8, 0.05f, 1.0f, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<float, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        SECTION("with scaling")
        {
            FloatGrayBitmap image2(image);
            checkBitmap<float, float>(&image2, 20, 10, 1, 4, 80, 800, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            FloatGrayBitmap image2(image, false);
            checkBitmap<float, float>(&image2, 20, 10, 1, 4, 80, 800, &image, 1.0);
        }
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<float, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0 / 65535.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<float, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }
}
