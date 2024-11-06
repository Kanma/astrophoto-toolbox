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
#include <string>


namespace astrophototoolbox {
namespace stacking {
namespace processing {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to stack processed light frames
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class FramesStacker
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Setup the stacker
        //--------------------------------------------------------------------------------
        void setup(
            unsigned int nbExpectedFrames,
            const std::filesystem::path& tempFolder = "",
            unsigned long maxFileSize = 50000000L
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add a bitmap to the list of bitmaps to be stacked
        ///
        /// It is expected that all the bitmaps have the same dimensions and range.
        ///
        /// Note: 'setup()' must have been called before this method.
        //--------------------------------------------------------------------------------
        bool addFrame(const std::string& lightFrame);

        //--------------------------------------------------------------------------------
        /// @brief  Add a bitmap to the list of bitmaps to be stacked
        ///
        /// It is expected that all the bitmaps have the same dimensions and range.
        ///
        /// Note: 'setup()' must have been called before this method.
        //--------------------------------------------------------------------------------
        bool addFrame(
            const std::shared_ptr<BITMAP>& lightFrame, const Transformation& transformation
        );

        //--------------------------------------------------------------------------------
        /// @brief  Computes the master dark frame from the provided list of dark frames,
        ///         and save it at the given destination path
        ///
        /// Several files need to be written during the processing, so the caller has to
        /// specify a temp folder to user (it will be created and destroyed by this
        /// method).
        //--------------------------------------------------------------------------------
        BITMAP* process(const std::string& destination = "");

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        ///
        /// Only useful in a multithreading scenario, where this method is called from a
        /// different thread than the one doing the processing.
        //--------------------------------------------------------------------------------
        void cancel();

        inline unsigned int nbFrames() const
        {
            return stacker.nbStackedBitmaps();
        }


    private:
        utils::BitmapStacker<BITMAP> stacker;
        rect_t outputRect;
    };

}
}
}


#include <astrophoto-toolbox/stacking/processing/stacking.hpp>
