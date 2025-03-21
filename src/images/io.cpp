/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/images/io.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/data/fits.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <assert.h>

#ifndef _WIN32
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifndef _WIN32
    #pragma clang diagnostic pop
#endif


using namespace astrophototoolbox;


namespace astrophototoolbox {
namespace io {

static RawImage rawImage;
static std::mutex rawImageMutex;

//-----------------------------------------------------------------------------

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

bool savePNM(
    const std::filesystem::path& filename, const std::filesystem::path& extension,
    bool convertColorSpace, Bitmap* bitmap
)
{
    std::ofstream output(filename, std::ios_base::out | std::ios_base::binary);
    if (!output.is_open())
        return false;

    space_t space = (convertColorSpace ? SPACE_sRGB : SPACE_SOURCE);

    if (extension == ".ppm")
    {
        if (bitmap->channelSize() == 1)
        {
            UInt8ColorBitmap* converted = requiresFormat<UInt8ColorBitmap>(bitmap, RANGE_DEST, space);

            savePPM8(output, converted);

            if (converted != bitmap)
                delete converted;

            return true;
        }
        else if (bitmap->channelSize() == 2)
        {
            UInt16ColorBitmap* converted = requiresFormat<UInt16ColorBitmap>(bitmap, RANGE_DEST, space);

            savePPM16(output, converted);

            if (converted != bitmap)
                delete converted;

            return true;
        }
    }
    else if (extension == ".pgm")
    {
        if (bitmap->channelSize() == 1)
        {
            UInt8GrayBitmap* converted = requiresFormat<UInt8GrayBitmap>(bitmap, RANGE_DEST, space);

            savePGM8(output, converted);

            if (converted != bitmap)
                delete converted;

            return true;
        }
        else if (bitmap->channelSize() == 2)
        {
            UInt16GrayBitmap* converted = requiresFormat<UInt16GrayBitmap>(bitmap, RANGE_DEST, space);

            savePGM16(output, converted);

            if (converted != bitmap)
                delete converted;

            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------

bool saveImage(
    const std::filesystem::path& filename, const std::filesystem::path& extension,
    int w, int h, int comp, const void* data, int stride_in_bytes
)
{
    int result = 0;

    if (extension == ".png")
        result = stbi_write_png(filename.string().c_str(), w, h, comp, data, stride_in_bytes);
    else if (extension == ".bmp")
        result = stbi_write_bmp(filename.string().c_str(), w, h, comp, data);
    else if (extension == ".tga")
        result = stbi_write_tga(filename.string().c_str(), w, h, comp, data);
    else if ((extension == ".jpg") || (extension == ".jpeg"))
        result = stbi_write_jpg(filename.string().c_str(), w, h, comp, data, 90);

    return (result != 0);
}

//-----------------------------------------------------------------------------

bool save(const std::filesystem::path& filename, Bitmap* bitmap, bool overwrite)
{
    assert(bitmap);

    if (std::filesystem::exists(filename))
    {
        if (!overwrite)
            return false;

        std::filesystem::remove(filename);
    }

    auto extension = filename.extension();

    if ((extension == ".ppm") || (extension == ".pgm"))
    {
        return savePNM(filename, extension, true, bitmap);
    }
    else if ((extension == ".png") || (extension == ".bmp") || (extension == ".tga") ||
             (extension == ".jpg") || (extension == ".jpeg"))
    {
        if (bitmap->channels() == 3)
        {
            UInt8ColorBitmap* converted = requiresFormat<UInt8ColorBitmap>(bitmap, RANGE_DEST, SPACE_sRGB);

            bool result = saveImage(
                filename, extension, converted->width(), converted->height(), 3,
                converted->data(), converted->bytesPerRow()
            );

            if (converted != bitmap)
                delete converted;

            return result;
        }
        else
        {
            UInt8GrayBitmap* converted = requiresFormat<UInt8GrayBitmap>(bitmap, RANGE_DEST, SPACE_sRGB);

            bool result = saveImage(
                filename, extension, converted->width(), converted->height(), 1,
                converted->data(), converted->bytesPerRow()
            );

            if (converted != bitmap)
                delete converted;

            return result;
        }
    }
    else if (extension == ".hdr")
    {
        if (bitmap->channels() == 3)
        {
            FloatColorBitmap* converted = requiresFormat<FloatColorBitmap>(bitmap, RANGE_DEST, SPACE_LINEAR);

            int result = stbi_write_hdr(
                filename.string().c_str(), converted->width(), converted->height(), 3, converted->data()
            );

            if (converted != bitmap)
                delete converted;

            return (result != 0);
        }
        else
        {
            FloatGrayBitmap* converted = requiresFormat<FloatGrayBitmap>(bitmap, RANGE_DEST, SPACE_LINEAR);

            int result = stbi_write_hdr(
                filename.string().c_str(), converted->width(), converted->height(), 1, converted->data()
            );

            if (converted != bitmap)
                delete converted;

            return (result != 0);
        }
    }
    else
    {
        astrophototoolbox::FITS output;

        if (!output.create(filename))
            return false;

        if (!output.write(bitmap))
        {
            output.close();
            return false;
        }

        output.close();

        return true;
    }
}

//-----------------------------------------------------------------------------

bool savePNM(
    const std::filesystem::path& filename, Bitmap* bitmap, bool convertColorSpace,
    bool overwrite
)
{
    assert(bitmap);

    if (std::filesystem::exists(filename))
    {
        if (!overwrite)
            return false;

        std::filesystem::remove(filename);
    }

    auto extension = filename.extension();

    if ((extension == ".ppm") || (extension == ".pgm"))
    {
        return savePNM(filename, extension, convertColorSpace, bitmap);
    }

    return false;
 }

//-----------------------------------------------------------------------------

Bitmap* load(const std::filesystem::path& filename, bool useCameraWhiteBalance, bool linear)
{
    Bitmap* bitmap = nullptr;

    if (!std::filesystem::exists(filename))
        return nullptr;

    auto extension = filename.extension();

    if ((extension == ".png") || (extension == ".bmp") || (extension == ".tga") ||
        (extension == ".jpg") || (extension == ".jpeg") || (extension == ".ppm") ||
        (extension == ".pgm"))
    {
        int width, height, channels;

        int is16bits = stbi_is_16_bit(filename.string().c_str());
        if (is16bits)
        {
            unsigned short* pixels = stbi_load_16(filename.string().c_str(), &width, &height, &channels, 0);
            if (!pixels)
                return nullptr;

            if (channels == 3)
                bitmap = new UInt16ColorBitmap(pixels, width, height);
            else if (channels == 1)
                bitmap = new UInt16GrayBitmap(pixels, width, height);

            stbi_image_free(pixels);
        }
        else
        {
            unsigned char* pixels = stbi_load(filename.string().c_str(), &width, &height, &channels, 0);
            if (!pixels)
                return nullptr;

            if (channels == 3)
                bitmap = new UInt8ColorBitmap(pixels, width, height);
            else if (channels == 1)
                bitmap = new UInt8GrayBitmap(pixels, width, height);

            stbi_image_free(pixels);
        }

        bitmap->setSpace(SPACE_sRGB, false);
    }
    else if (extension == ".hdr")
    {
        int width, height, channels;

        float* pixels = stbi_loadf(filename.string().c_str(), &width, &height, &channels, 0);
        if (!pixels)
            return nullptr;

        if (channels == 3)
            bitmap = new FloatColorBitmap(pixels, width, height);
        else if (channels == 1)
            bitmap = new FloatGrayBitmap(pixels, width, height);

        stbi_image_free(pixels);

        bitmap->setSpace(SPACE_LINEAR, false);
    }
    else if (FITS::isFITS(filename))
    {
        FITS fits;
        if (!fits.open(filename, true))
            return nullptr;

        bitmap = fits.readBitmap();
    }
    else
    {
        std::lock_guard<std::mutex> lock(rawImageMutex);

        if (!rawImage.open(filename))
            return nullptr;

        // Decode the RAW image
        if (rawImage.channels() == 3)
            bitmap = new UInt16ColorBitmap();
        else
            bitmap = new UInt8ColorBitmap();

        if (!rawImage.toBitmap(bitmap, useCameraWhiteBalance, linear))
        {
            delete bitmap;
            return nullptr;
        }
    }

    return bitmap;
}

}
}
