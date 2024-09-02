#include "bitmap_helpers.h"


TEST_CASE("UInt32 gray bitmap", "[Bitmap]")
{
    UInt32GrayBitmap image(20, 10);

    REQUIRE(image.width() == 20);
    REQUIRE(image.height() == 10);
    REQUIRE(image.channels() == 1);
    REQUIRE(image.bytesPerPixel() == 4);
    REQUIRE(image.bytesPerRow() == 80);
    REQUIRE(image.size() == 800);

    uint32_t *data = image.data();
    REQUIRE(data);

    uint32_t *ptr = image.data(10, 5);
    REQUIRE(ptr == data + 5 * 20 + 10);

    for (size_t i = 0; i < image.height() * image.width(); ++i)
        REQUIRE(data[i] == 0);

    uint32_t v = 0;
    for (size_t i = 0; i < image.height() * image.width(); ++i)
    {
        data[i] = v;
        v += 0x1000000;
    }

    SECTION("set from a buffer")
    {
        uint32_t buffer[10 * 8];

        uint32_t v = 0xFF00;
        for (size_t i = 0; i < 10 * 8; ++i)
        {
            buffer[i] = v;
            v -= 0x1000000;
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
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(false, 10, 8, 52);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 8bits integer")
    {
        Bitmap* image2 = createUInt8Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint8_t>(&image, 10, 8, 1, 4, 40, 320, image2, 16843009.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint8_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, 16bits integer")
    {
        Bitmap* image2 = createUInt16Bitmap(false, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmap<uint32_t, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 65537.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmap<uint32_t, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another gray bitmap, float")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 0.05f, 1.0f);
            image.set(image2);
            checkBitmap<uint32_t, float>(&image, 10, 8, 1, 4, 40, 320, image2, 4294967295.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createFloatBitmap(false, 10, 8, 1.0f, 255.0f);
            image.set(image2, false);
            checkBitmap<uint32_t, float>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another gray bitmap, double")
    {
        SECTION("with scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 0.05f, 1.0);
            image.set(image2);
            checkBitmap<uint32_t, double>(&image, 10, 8, 1, 4, 40, 320, image2, 4294967295.0);
            delete image2;
        }

        SECTION("without scaling")
        {
            Bitmap* image2 = createDoubleBitmap(false, 10, 8, 1.0, 255.0);
            image.set(image2, false);
            checkBitmap<uint32_t, double>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
            delete image2;
        }
    }

    SECTION("set from another color bitmap of the same type")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("set from another color bitmap of the same type, with extra row bytes")
    {
        Bitmap* image2 = createUInt32Bitmap(true, 10, 8, 132);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint32_t, uint32_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }

    SECTION("creation from another gray bitmap of the same type")
    {
        SECTION("with scaling")
        {
            UInt32GrayBitmap image2(image);
            checkBitmap<uint32_t, uint32_t>(&image2, 20, 10, 1, 4, 80, 800, &image, 1.0);
        }

        SECTION("without scaling (should have no effect)")
        {
            UInt32GrayBitmap image2(image, false);
            checkBitmap<uint32_t, uint32_t>(&image2, 20, 10, 1, 4, 80, 800, &image, 1.0);
        }
    }

    SECTION("creation from another color bitmap, 16bits integer, with extra row bytes")
    {
        Bitmap* image2 = createUInt16Bitmap(true, 10, 8, 72);

        SECTION("with scaling")
        {
            image.set(image2);
            checkBitmapMeanChannel<uint32_t, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 65537.0);
        }

        SECTION("without scaling")
        {
            image.set(image2, false);
            checkBitmapMeanChannel<uint32_t, uint16_t>(&image, 10, 8, 1, 4, 40, 320, image2, 1.0);
        }

        delete image2;
    }
}
