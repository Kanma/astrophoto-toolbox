#pragma once

namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Contains informations about the capture of a bitmap
    //------------------------------------------------------------------------------------
    struct bitmap_info_t {
        unsigned int isoSpeed = 0;
        float shutterSpeed = 0.0f;
        float aperture = 0.0f;
        float focalLength = 0.0f;
    };

}
