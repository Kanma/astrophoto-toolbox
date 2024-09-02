#include "bitmap_helpers.h"


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
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8, 30);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 1, 2, 20, 160, image2, 257.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint8_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 32bits integer")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0 / 65537.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint16_t, uint32_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0 / 65537.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint16_t, float>(&image, 10, 8, 1, 2, 20, 160, image2, 65535.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint16_t, float>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another gray bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 0.05f, 1.0);
            image.set(image2);
            checkBitmap<uint16_t, double>(&image, 10, 8, 1, 2, 20, 160, image2, 65535.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint16_t, double>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint16_t, uint16_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt16GrayBitmap image2(image);
            checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 1, 2, 40, 400, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt16GrayBitmap image2(image, false);
            checkBitmap<uint16_t, uint16_t>(&image2, 20, 10, 1, 2, 40, 400, &image, 1.0);
        }
    }

    SECTION("creation from another color bitmap, 32bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint16_t, uint32_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0 / 65537.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint16_t, uint32_t>(&image, 10, 8, 1, 2, 20, 160, image2, 1.0 / 65537.0);
        }

        delete image2;
    }
}
