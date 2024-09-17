/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

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
