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
#include <astrophoto-toolbox/stacking/backgroundcalibration.h>
#include <filesystem>
#include <string>
#include <memory>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the stacking-related operations
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class LightFrameProcessor
    {
    public:
        bool setMasterDark(const std::string& filename);

        inline void setMasterDark(
            const std::shared_ptr<BITMAP>& masterDark, const point_list_t& hotPixels
        )
        {
            this->masterDark = masterDark;
            this->hotPixels = hotPixels;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Computes the master dark frame from the provided list of dark frames,
        ///         and save it at the given destination path
        ///
        /// Several files need to be written during the processing, so the caller has to
        /// specify a temp folder to user (it will be created and destroyed by this
        /// method).
        //--------------------------------------------------------------------------------
        std::shared_ptr<BITMAP> process(
            const std::string& lightFrame, bool reference = false,
            const std::string& destination = ""
        );

        std::shared_ptr<BITMAP> process(
            const std::shared_ptr<BITMAP>& lightFrame, bool reference = false,
            const std::string& destination = ""
        );


    private:
        void removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const requires(BITMAP::Channels == 3);
        void removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const requires(BITMAP::Channels == 1);


    private:
        BackgroundCalibration<BITMAP> calibration;
        std::shared_ptr<BITMAP> masterDark;
        point_list_t hotPixels;
    };

}
}


#include <astrophoto-toolbox/stacking/processing/lightframes.hpp>
