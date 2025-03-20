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
#include <astrophoto-toolbox/stacking/utils/bitmapstacker.h>
#include <filesystem>


namespace astrophototoolbox {
namespace stacking {
namespace processing {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to generate a master dark frame (and list of hot pixels) from a
    ///         list of dark frames
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class MasterDarkGenerator
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Computes the master dark frame from the provided list of dark frames,
        ///         and save it at the given destination path
        ///
        /// Several files need to be written during the processing, so the caller has to
        /// specify a temp folder to user (it will be created and destroyed by this
        /// method).
        //--------------------------------------------------------------------------------
        BITMAP* compute(
            const std::vector<std::filesystem::path>& darkFrames,
            const std::filesystem::path& destination,
            const std::filesystem::path& tmpFolder
        );

        //--------------------------------------------------------------------------------
        /// @brief  Returns the positions of the hot pixels
        //--------------------------------------------------------------------------------
        inline const point_list_t& getHotPixels() const
        {
            return hotPixels;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        ///
        /// Only useful in a multithreading scenario, where this method is called from a
        /// different thread than the one doing the processing.
        //--------------------------------------------------------------------------------
        void cancel();


    private:
        void detectHotPixels(BITMAP* masterDark) requires(BITMAP::Channels == 3);
        void detectHotPixels(BITMAP* masterDark) requires(BITMAP::Channels == 1);


    private:
        utils::BitmapStacker<BITMAP> stacker;
        point_list_t hotPixels;
        bool cancelled = false;
    };

}
}
}

#include <astrophoto-toolbox/stacking/processing/masterdark.hpp>
