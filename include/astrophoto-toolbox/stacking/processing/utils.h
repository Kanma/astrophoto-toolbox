/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/point.h>
#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/data/transformation.h>
#include <filesystem>


namespace astrophototoolbox {
namespace stacking {

    template<class BITMAP>
    BITMAP* loadProcessedBitmap(
        const std::filesystem::path& filename, point_list_t* hotPixels = nullptr,
        star_list_t* stars = nullptr, Transformation* transformation = nullptr
    );

    template<class BITMAP>
    bool saveProcessedBitmap(
        BITMAP* bitmap, const std::filesystem::path& path,
        point_list_t* hotPixels = nullptr, star_list_t* stars = nullptr,
        Transformation* transformation = nullptr
    );

}
}


#include <astrophoto-toolbox/stacking/processing/utils.hpp>
