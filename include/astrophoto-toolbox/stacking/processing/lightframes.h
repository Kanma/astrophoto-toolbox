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
#include <astrophoto-toolbox/stacking/utils/backgroundcalibration.h>
#include <filesystem>
#include <string>
#include <memory>


namespace astrophototoolbox {
namespace stacking {
namespace processing {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the operations related to light frames (background
    ///         calibration, dark frame substraction, ...)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class LightFrameProcessor
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Set the master dark frame file to use
        ///
        /// It is expected that it is a FITS file containing a bitmap and a list of hot
        /// pixels.
        //--------------------------------------------------------------------------------
        bool setMasterDark(const std::string& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Set the master dark frame bitmap and list of hot pixels to use
        //--------------------------------------------------------------------------------
        inline void setMasterDark(
            const std::shared_ptr<BITMAP>& masterDark, const point_list_t& hotPixels
        )
        {
            this->masterDark = masterDark;
            this->hotPixels = hotPixels;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Set the parameters to use for background calibration
        ///
        /// This method is an aternative to calling 'process()' withe the'reference' flag
        /// set.
        //--------------------------------------------------------------------------------
        void setParameters(
            const utils::background_calibration_parameters_t& parameters
        );

        //--------------------------------------------------------------------------------
        /// @brief  Process a light frame file, and save it at the given destination path
        ///
        /// If 'reference' is set, the light frame becomes the reference one used by the
        /// background calibration algorithm for all the following frames. So be sure to
        /// pass your reference frame as the first one and set the flag, or use the
        /// 'setParameters()' method.
        //--------------------------------------------------------------------------------
        std::shared_ptr<BITMAP> process(
            const std::string& lightFrame, bool reference = false,
            const std::string& destination = ""
        );

        //--------------------------------------------------------------------------------
        /// @brief  Process a light frame bitmap, and save it at the given destination path
        ///
        /// If 'reference' is set, the light frame becomes the reference one used by the
        /// background calibration algorithm for all the following frames. So be sure to
        /// pass your reference frame as the first one and set the flag, or use the
        /// 'setParameters()' method.
        //--------------------------------------------------------------------------------
        std::shared_ptr<BITMAP> process(
            const std::shared_ptr<BITMAP>& lightFrame, bool reference = false,
            const std::string& destination = ""
        );


    private:
        void removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const requires(BITMAP::Channels == 3);
        void removeHotPixels(const std::shared_ptr<BITMAP>& bitmap) const requires(BITMAP::Channels == 1);


    private:
        utils::BackgroundCalibration<BITMAP> calibration;
        std::shared_ptr<BITMAP> masterDark;
        point_list_t hotPixels;
    };

}
}
}


#include <astrophoto-toolbox/stacking/processing/lightframes.hpp>
