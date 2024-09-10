#pragma once

#include <catch.hpp>
#include <astrophoto-toolbox/images/bitmap.h>

using namespace astrophototoolbox;


void setBitmapInfo(Bitmap* bitmap);

Bitmap* createUInt8Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow = 0,
    space_t space = SPACE_LINEAR
);

Bitmap* createUInt16Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow = 0,
    range_t range = RANGE_USHORT, space_t space = SPACE_LINEAR
);

Bitmap* createUInt32Bitmap(
    bool color, unsigned int width, unsigned int height, unsigned int bytesPerRow = 0,
    range_t range = RANGE_UINT, space_t space = SPACE_LINEAR
);

Bitmap* createFloatBitmap(
    bool color, unsigned int width, unsigned int height, float increment,
    float maxValue, unsigned int bytesPerRow = 0, range_t range = RANGE_ONE,
    space_t space = SPACE_LINEAR
);

Bitmap* createDoubleBitmap(
    bool color, unsigned int width, unsigned int height, double increment,
    double maxValue, unsigned int bytesPerRow = 0, range_t range = RANGE_ONE
);

void checkBitmapInfo(Bitmap* bitmap, Bitmap* ref);


template<typename T1, typename T2>
void checkBitmap(
    Bitmap* bitmap, unsigned int width, unsigned int height, unsigned int channels,
    unsigned int bytesPerPixel, unsigned int bytesPerRow, unsigned int size, Bitmap* ref,
    double factor, range_t range, space_t space = SPACE_LINEAR, T1 tolerance = 0
)
{
    REQUIRE(bitmap->width() == width);
    REQUIRE(bitmap->height() == height);
    REQUIRE(bitmap->channels() == channels);
    REQUIRE(bitmap->bytesPerPixel() == bytesPerPixel);
    REQUIRE(bitmap->bytesPerRow() == bytesPerRow);
    REQUIRE(bitmap->size() == size);
    REQUIRE(bitmap->range() == range);
    REQUIRE(bitmap->space() == space);

    for (unsigned int y = 0; y < height; ++y)
    {
        T1* data = (T1*) bitmap->ptr(y);
        T2* data2 = (T2*) ref->ptr(y);

        for (unsigned int x = 0; x < bitmap->bytesPerRow() / sizeof(T1); ++x)
        {
            if (tolerance == 0)
            {
                REQUIRE(data[x] == T1(double(data2[x]) * factor));
            }
            else
            {
                if (data[x] - tolerance < data[x])
                    REQUIRE(data[x] >= T1(double(data2[x]) * factor) - tolerance);

                if (data[x] + tolerance > data[x])
                    REQUIRE(data[x] <= T1(double(data2[x]) * factor) + tolerance);
            }
        }
    }

    checkBitmapInfo(bitmap, ref);
}


template<typename T>
void checkIdenticalBitmaps(Bitmap* bitmap, Bitmap* ref)
{
    checkBitmap<T, T>(
        bitmap, ref->width(), ref->height(), ref->channels(), ref->bytesPerPixel(), 
        ref->bytesPerRow(), ref->size(), ref, 1.0, ref->range()
    );
}


template<typename T1, typename T2>
void checkBitmapIdenticalChannels(
    Bitmap* bitmap, unsigned int width, unsigned int height, unsigned int channels,
    unsigned int bytesPerPixel, unsigned int bytesPerRow, unsigned int size, Bitmap* ref,
    double factor, range_t range
)
{
    REQUIRE(bitmap->width() == width);
    REQUIRE(bitmap->height() == height);
    REQUIRE(bitmap->channels() == channels);
    REQUIRE(bitmap->bytesPerPixel() == bytesPerPixel);
    REQUIRE(bitmap->bytesPerRow() == bytesPerRow);
    REQUIRE(bitmap->size() == size);
    REQUIRE(bitmap->range() == range);

    for (unsigned int y = 0; y < height; ++y)
    {
        T1* data = (T1*) bitmap->ptr(y);
        T2* data2 = (T2*) ref->ptr(y);

        for (unsigned int x = 0, x2 = 0; x < bitmap->bytesPerRow() / (channels * sizeof(T1)); ++x, x2 += 3)
        {
            for (unsigned int c = 0; c < channels; ++c)
                REQUIRE(data[x2 + c] == T1(double(data2[x]) * factor));
        }
    }

    checkBitmapInfo(bitmap, ref);
}


template<typename T1, typename T2>
void checkBitmapMeanChannel(
    Bitmap* bitmap, unsigned int width, unsigned int height, unsigned int channels,
    unsigned int bytesPerPixel, unsigned int bytesPerRow, unsigned int size, Bitmap* ref,
    double factor, range_t range
)
{
    REQUIRE(bitmap->width() == width);
    REQUIRE(bitmap->height() == height);
    REQUIRE(bitmap->channels() == channels);
    REQUIRE(bitmap->bytesPerPixel() == bytesPerPixel);
    REQUIRE(bitmap->bytesPerRow() == bytesPerRow);
    REQUIRE(bitmap->size() == size);
    REQUIRE(bitmap->range() == range);

    for (unsigned int y = 0; y < height; ++y)
    {
        T1* data = (T1*) bitmap->ptr(y);
        T2* data2 = (T2*) ref->ptr(y);

        for (unsigned int x = 0, x2 = 0; x < bitmap->bytesPerRow() / (channels * sizeof(T1)); ++x, x2 += 3)
        {
            double mean = 0.0;

            for (unsigned int c = 0; c < ref->channels(); ++c)
                mean += double(data2[x2 + c]);

            mean =  mean / ref->channels();

            REQUIRE(data[x] == T1(mean * factor));
        }
    }

    checkBitmapInfo(bitmap, ref);
}


template<typename T>
void checkBitmapIsChannel(
    Bitmap* bitmap, unsigned int width, unsigned int height, unsigned int channel,
    unsigned int bytesPerPixel, unsigned int bytesPerRow, unsigned int size, Bitmap* ref,
    range_t range
)
{
    REQUIRE(bitmap->width() == width);
    REQUIRE(bitmap->height() == height);
    REQUIRE(bitmap->channels() == 1);
    REQUIRE(bitmap->bytesPerPixel() == bytesPerPixel);
    REQUIRE(bitmap->bytesPerRow() == bytesPerRow);
    REQUIRE(bitmap->size() == size);
    REQUIRE(bitmap->range() == range);

    for (unsigned int y = 0; y < height; ++y)
    {
        T* data = (T*) bitmap->ptr(y);
        T* data2 = (T*) ref->ptr(y);

        for (unsigned int x = 0, x2 = 0; x < width; ++x, x2 += 3)
            REQUIRE(data[x] == data2[x2 + channel]);
    }

    checkBitmapInfo(bitmap, ref);
}


template<typename T>
void checkChannelOfBitmap(
    Bitmap* bitmap, unsigned int width, unsigned int height, unsigned int channel,
    unsigned int bytesPerPixel, unsigned int bytesPerRow, unsigned int size, Bitmap* ref
)
{
    REQUIRE(bitmap->width() == width);
    REQUIRE(bitmap->height() == height);
    REQUIRE(bitmap->channels() == 3);
    REQUIRE(bitmap->bytesPerPixel() == bytesPerPixel);
    REQUIRE(bitmap->bytesPerRow() == bytesPerRow);
    REQUIRE(bitmap->size() == size);

    for (unsigned int y = 0; y < height; ++y)
    {
        T* data = (T*) bitmap->ptr(y);
        T* data2 = (T*) ref->ptr(y);

        for (unsigned int x = 0, x2 = 0; x < width; ++x, x2 += 3)
            REQUIRE(data[x2 + channel] == data2[x]);
    }

    checkBitmapInfo(bitmap, ref);
}
