/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <filesystem>


namespace astrophototoolbox {
namespace io {

    //------------------------------------------------------------------------------------
    /// @brief  Save a Bitmap to a file
    ///
    /// The format of the file is determined from the extension of the provided filename.
    /// Supported extensions: .png, .bmp, .tga, .jpg, .jpeg, .hdr, .ppm, .pgm. Any other
    /// extension is considered as a FITS file.
    ///
    /// PPM and PGM files are saved as 16bits ones, unless the bitmap contains 8bits data.
    /// All other image formats (but FITS and .hdr) are saved as 8bits.
    ///
    /// Bitmaps are written to FITS files without any change to their color space. This
    /// format is the better choice if you want to process your image in another software
    /// (no compression, no loss, exactly the same pixel values your software
    //// manipulated).
    ///
    /// For PNG, BMP, TGA, JPG and HDR, the color space is converted if necessary, in
    /// order for the files to look correct when opened in other softwares. Thus, those
    /// formats are not to be used if you want to continue processing the images in
    /// another software).
    ///
    /// The PPM and PGM formats can be used to do both: ensure that the image is correctly
    /// displayed (the default) OR save the pixel values as-is to further open them in
    /// another software (it is way easier to parse a PNM file than a FITS one).
    /// To do the letter, use the 'savePNM()' function instead.
    //------------------------------------------------------------------------------------
    bool save(const std::filesystem::path& filename, Bitmap* bitmap, bool overwrite = false);


    //------------------------------------------------------------------------------------
    /// @brief  Save a Bitmap to a PNM file
    ///
    /// The format of the file is determined from the extension of the provided filename.
    /// Supported extensions: .ppm and .pgm.
    ///
    /// PPM and PGM files are saved as 16bits ones, unless the bitmap contains 8bits data.
    ///
    /// The 'convertColorSpace' parameter allows to deactivate any color space conversion
    /// necessary to produce correct-looking images in other software. See the
    /// documentation of 'save()' for more details.
    //------------------------------------------------------------------------------------
    bool savePNM(
        const std::filesystem::path& filename, Bitmap* bitmap, bool convertColorSpace = true,
        bool overwrite = false
    );


    //------------------------------------------------------------------------------------
    /// @brief  Load a Bitmap from a file
    ///
    /// Supported extensions: .png, .bmp, .tga, .jpg, .jpeg, .hdr, .ppm, .pgm. Any other
    /// extension is first inspected to determine if it is a FITS file, and if not we try
    /// to load it as a RAW image.
    ///
    /// The 'useCameraWhiteBalance' and 'linear' parameters are only relevant for RAW
    /// files.
    ///
    /// See also 'FITS::readBitmap()' and 'RawImage::toBitmap()', which are more relevant
    /// if you already know the format of the file.
    //------------------------------------------------------------------------------------
    Bitmap* load(
        const std::filesystem::path& filename, bool useCameraWhiteBalance = false,
        bool linear = true
    );

}
}
