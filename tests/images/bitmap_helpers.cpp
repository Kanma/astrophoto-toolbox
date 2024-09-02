#include "bitmap_helpers.h"

using namespace astrophototoolbox;


void setBitmapInfo(Bitmap* bitmap)
{
    auto& info = bitmap->info();
    info.isoSpeed = 400;
    info.shutterSpeed = 2.0f;
    info.aperture = 4.5f;
    info.focalLength = 20.0f;
}


Bitmap* createUInt8Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow
)
{
    Bitmap* bitmap;

    if (color)
        bitmap = new UInt8ColorBitmap();
    else
        bitmap = new UInt8GrayBitmap();

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    uint8_t v = 0;
    for (unsigned int y = 0; y < height; ++y)
    {
        uint8_t* data = bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow(); ++x)
        {
            data[x] = v;
            ++v;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createUInt16Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new UInt16ColorBitmap();
    else
        bitmap = new UInt16GrayBitmap();

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    uint16_t v = 0;
    for (unsigned int y = 0; y < height; ++y)
    {
        uint16_t* data = (uint16_t*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 2; ++x)
        {
            data[x] = v;
            v += 0x100;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createUInt32Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new UInt32ColorBitmap();
    else
        bitmap = new UInt32GrayBitmap();

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    uint32_t v = 0;
    for (unsigned int y = 0; y < height; ++y)
    {
        uint32_t* data = (uint32_t*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 4; ++x)
        {
            data[x] = v;
            v += 0x1000000;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createFloatBitmap(
    bool color, unsigned int width, unsigned int height, float increment,
    float maxValue, unsigned int bytesPerRow
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new FloatColorBitmap();
    else
        bitmap = new FloatGrayBitmap();

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    float v = 0.0f;
    for (unsigned int y = 0; y < height; ++y)
    {
        float* data = (float*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 4; ++x)
        {
            data[x] = v;
            v += increment;
            if (v > maxValue)
                v = 0.0f;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createDoubleBitmap(
    bool color, unsigned int width, unsigned int height, double increment,
    double maxValue, unsigned int bytesPerRow
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new DoubleColorBitmap();
    else
        bitmap = new DoubleGrayBitmap();

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    double v = 0.0;
    for (unsigned int y = 0; y < height; ++y)
    {
        double* data = (double*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 8; ++x)
        {
            data[x] = v;
            v += increment;
            if (v > maxValue)
                v = 0.0;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


void checkBitmapInfo(Bitmap* bitmap, Bitmap* ref)
{
    auto info1 = bitmap->info();
    auto info2 = ref->info();
    REQUIRE(info1.isoSpeed == info2.isoSpeed);
    REQUIRE(info1.shutterSpeed == Approx(info2.shutterSpeed));
    REQUIRE(info1.aperture == Approx(info2.aperture));
    REQUIRE(info1.focalLength == Approx(info2.focalLength));
}
