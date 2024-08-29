#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <libraw/libraw.h>
#include <string>


namespace astrophototoolbox {
namespace pnm {

    //------------------------------------------------------------------------------------
    /// @brief  Save a Bitmap to a PPM or PGM file
    ///
    /// The type of file is determined from the extension of the provided filename.
    ///
    /// The file is a 16bits one, unless the bitmap contains 8bits data.
    //------------------------------------------------------------------------------------
    bool save(const std::string& filename, Bitmap* bitmap);

}
}
