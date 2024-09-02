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

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8, 20);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 16843009.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint8_t, uint32_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 16843009.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 255.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint8_t, float>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 0.05, 1.0);
            image.set(image2);
            checkBitmap<uint8_t, double>(&image, 10, 8, 1, 1, 10, 80, image2, 255.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint8_t, double>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt8Bitmap(true, 10, 8, 42);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint8_t, uint8_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt8GrayBitmap image2(image);
            checkBitmap<uint8_t, uint8_t>(&image2, 20, 10, 1, 1, 20, 200, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt8GrayBitmap image2(image, false);
            checkBitmap<uint8_t, uint8_t>(&image2, 20, 10, 1, 1, 20, 200, &image, 1.0);
        }
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint8_t, uint16_t>(&image, 10, 8, 1, 1, 10, 80, image2, 1.0 / 257.0);
        }

        delete image2;
    }
}
