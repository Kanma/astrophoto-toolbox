/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/images/pnm.h>
#include <iostream>
#include <assert.h>

namespace astrophototoolbox {
namespace pnm {

void savePPM8(std::ofstream& stream, UInt8ColorBitmap* bitmap)
{
    assert(bitmap);

    stream << "P6" << std::endl
            << bitmap->width() << " " << bitmap->height() << std::endl
            << "255" << std::endl;

    uint8_t* data = bitmap->data();
    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        stream.write((char*) data, bitmap->width() * bitmap->bytesPerPixel());
        data += bitmap->bytesPerRow();
    }
}

//-----------------------------------------------------------------------------

void savePPM16(std::ofstream& stream, UInt16ColorBitmap* bitmap)
{
    assert(bitmap);

    stream << "P6" << std::endl
            << bitmap->width() << " " << bitmap->height() << std::endl
            << "65535" << std::endl;

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        const uint16_t* data = bitmap->data(y);

        for (unsigned int x = 0, i = 0; x < bitmap->width(); ++x, i += 3)
        {
            for (unsigned int c = 0; c < bitmap->channels(); ++c)
            {
                uint8_t v = data[i + c] >> 8;
                stream.write((char*) &v, sizeof(uint8_t));

                v = data[i + c] & 0xFF;
                stream.write((char*) &v, sizeof(uint8_t));
            }
        }
    }
}

//-----------------------------------------------------------------------------

void savePGM8(std::ofstream& stream, UInt8GrayBitmap* bitmap)
{
    assert(bitmap);

    stream << "P5" << std::endl
            << bitmap->width() << " " << bitmap->height() << std::endl
            << "255" << std::endl;

    uint8_t* data = bitmap->data();
    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        stream.write((char*) data, bitmap->width() * bitmap->bytesPerPixel());
        data += bitmap->bytesPerRow();
    }
}

//-----------------------------------------------------------------------------

void savePGM16(std::ofstream& stream, UInt16GrayBitmap* bitmap)
{
    assert(bitmap);

    stream << "P5" << std::endl
            << bitmap->width() << " " << bitmap->height() << std::endl
            << "65535" << std::endl;

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        const uint16_t* data = bitmap->data(y);

        for (unsigned int x = 0; x < bitmap->width(); ++x)
        {
            uint8_t v = data[x] >> 8;
            stream.write((char*) &v, sizeof(uint8_t));

            v = data[x] & 0xFF;
            stream.write((char*) &v, sizeof(uint8_t));
        }
    }
}

//-----------------------------------------------------------------------------

bool save(const std::string& filename, Bitmap* bitmap)
{
    std::ofstream output(filename, std::ios_base::out | std::ios_base::binary);
    if (!output.is_open())
        return false;

    if (filename.substr(filename.size() - 4, 4) == ".ppm")
    {
        if (!bitmap->isFloatingPoint() && (bitmap->channels() == 3) && (bitmap->channelSize() == 1))
        {
            savePPM8(output, dynamic_cast<UInt8ColorBitmap*>(bitmap));
        }
        else if (!bitmap->isFloatingPoint() && (bitmap->channels() == 3) && (bitmap->channelSize() == 2))
        {
            savePPM16(output, dynamic_cast<UInt16ColorBitmap*>(bitmap));
        }
        else if (!bitmap->isFloatingPoint() && (bitmap->channelSize() == 1))
        {
            auto converted = new UInt8ColorBitmap(bitmap);
            savePPM8(output, converted);
            delete converted;
        }
        else
        {
            auto converted = new UInt16ColorBitmap(bitmap);
            savePPM16(output, converted);
            delete converted;
        }
    }
    else if (filename.substr(filename.size() - 4, 4) == ".pgm")
    {
        if (!bitmap->isFloatingPoint() && (bitmap->channels() == 1) && (bitmap->channelSize() == 1))
        {
            savePGM8(output, dynamic_cast<UInt8GrayBitmap*>(bitmap));
        }
        else if (!bitmap->isFloatingPoint() && (bitmap->channels() == 1) && (bitmap->channelSize() == 2))
        {
            savePGM16(output, dynamic_cast<UInt16GrayBitmap*>(bitmap));
        }
        else if (!bitmap->isFloatingPoint() && (bitmap->channelSize() == 1))
        {
            auto converted = new UInt8GrayBitmap(bitmap);
            savePGM8(output, converted);
            delete converted;
        }
        else
        {
            auto converted = new UInt16GrayBitmap(bitmap);
            savePGM16(output, converted);
            delete converted;
        }
    }
    else
    {
        return false;
    }

    return true;
}

}
}
