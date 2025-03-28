/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <libraw/libraw.h>
#include <filesystem>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Container for a RAW image
    ///
    /// Used to convert a RAW image to a Bitmap.
    //------------------------------------------------------------------------------------
    class RawImage
    {
        //_____ Construction / Destruction __________
    public:
        RawImage();
        ~RawImage();


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Open a RAW image from a file
        //--------------------------------------------------------------------------------
        bool open(const std::filesystem::path& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Open a RAW image from a memory buffer
        //--------------------------------------------------------------------------------
        bool open(void* buffer, size_t size);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the width of the image, in pixels
        //--------------------------------------------------------------------------------
        unsigned int width() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the height of the image, in pixels
        //--------------------------------------------------------------------------------
        unsigned int height() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of channels of the image
        //--------------------------------------------------------------------------------
        uint8_t channels() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the ISO speed of the image
        //--------------------------------------------------------------------------------
        unsigned int isoSpeed() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the shutter speed of the image
        //--------------------------------------------------------------------------------
        float shutterSpeed() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the aperture of the image
        //--------------------------------------------------------------------------------
        float aperture() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the focal length of the image
        //--------------------------------------------------------------------------------
        float focalLength() const;

        //--------------------------------------------------------------------------------
        /// @brief  Convert the RAW image to a bitmap
        //--------------------------------------------------------------------------------
        bool toBitmap(
            Bitmap* bitmap, bool useCameraWhiteBalance = false, bool linear = true
        );


        //_____ Attributes __________
    private:
        LibRaw _processor;
    };

}
