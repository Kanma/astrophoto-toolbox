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
    /// The type of file is determined from the extension of the provided filename.
    /// Supported extensions: .png, .bmp, .tga, .jpg, .jpeg, .hdr, .ppm, .pgm. Any other
    /// extension is considered as a FITS file.
    ///
    /// PPM and PGM files are saved as 16bits ones, unless the bitmap contains 8bits data.
    /// All other image formats (but FITS) are saved as 8bits.
    //------------------------------------------------------------------------------------
    bool save(const std::filesystem::path& filename, Bitmap* bitmap, bool overwrite = false);

}
}
