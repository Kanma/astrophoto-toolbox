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
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow,
    space_t space
)
{
    Bitmap* bitmap;

    if (color)
        bitmap = new UInt8ColorBitmap(space);
    else
        bitmap = new UInt8GrayBitmap(space);

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
            if (space == SPACE_LINEAR)
                data[x] = v;
            else
                data[x] = ((1.0 + 0.055) * pow(double(v) / 255.0, 1.0 / 2.4) - 0.055) * 255.0;

            ++v;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createUInt16Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow,
    range_t range, space_t space
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new UInt16ColorBitmap(range, space);
    else
        bitmap = new UInt16GrayBitmap(range, space);

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    uint16_t increment = (range == RANGE_USHORT ? 0x100 : 0x01);
    uint16_t maxValue = (range == RANGE_USHORT ? 0xFFFF : 0xFF);

    uint16_t v = 0;
    for (unsigned int y = 0; y < height; ++y)
    {
        uint16_t* data = (uint16_t*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 2; ++x)
        {
            if (space == SPACE_LINEAR)
                data[x] = v;
            else if (double(v) / 65535.0 <= 0.0031308)
                data[x] = double(v) * 12.92;
            else
                data[x] = ((1.0 + 0.055) * pow(double(v) / 65535.0, 1.0 / 2.4) - 0.055) * 65535.0;

            v += increment;
            if (v > maxValue)
                v = 0;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createUInt32Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow,
    range_t range, space_t space
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new UInt32ColorBitmap(range, space);
    else
        bitmap = new UInt32GrayBitmap(range, space);

    if (bytesPerRow > 0)
        bitmap->resize(width, height, bytesPerRow);
    else
        bitmap->resize(width, height);

    uint32_t increment = (range == RANGE_UINT ? 0x1000000 : (range == RANGE_USHORT ? 0x100 : 0x01));
    uint32_t maxValue = (range == RANGE_UINT ? 0xFFFFFFFF : (range == RANGE_USHORT ? 0xFFFF : 0xFF));

    uint32_t v = 0;
    for (unsigned int y = 0; y < height; ++y)
    {
        uint32_t* data = (uint32_t*) bitmap->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / 4; ++x)
        {
            if (space == SPACE_LINEAR)
                data[x] = v;
            else if (double(v) / 4294967295.0 <= 0.0031308)
                data[x] = double(v) * 12.92;
            else
                data[x] = ((1.0 + 0.055) * pow(double(v) / 4294967295.0, 1.0 / 2.4) - 0.055) * 4294967295.0;

            v += increment;
            if (v > maxValue)
                v = 0;
        }
    }

    setBitmapInfo(bitmap);

    return bitmap;
}


Bitmap* createFloatBitmap(
    bool color, unsigned int width, unsigned int height, float increment,
    float maxValue, unsigned int bytesPerRow, range_t range, space_t space
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new FloatColorBitmap(range, space);
    else
        bitmap = new FloatGrayBitmap(range, space);

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
            if (space == SPACE_LINEAR)
                data[x] = v;
            else if (double(v) <= 0.0031308)
                data[x] = double(v) * 12.92;
            else
                data[x] = ((1.0 + 0.055) * pow(double(v), 1.0 / 2.4) - 0.055);

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
    double maxValue, unsigned int bytesPerRow, range_t range
)
{
    Bitmap* bitmap;
    
    if (color)
        bitmap = new DoubleColorBitmap(range);
    else
        bitmap = new DoubleGrayBitmap(range);

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
