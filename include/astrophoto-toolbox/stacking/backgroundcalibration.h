/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform the background calibration over a group of images
    ///
    /// The background calibration consists in normalizing the background value of each
    /// picture before stacking it. The background value is defined as the median value of
    /// all the pixels of the picture.
    ///
    /// This class is a reimplementation of the relevant parts of DeepSkyStacker's
    /// 'BackgroundCalibration' class, adapted to astrophoto-toolbox needs.
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class BackgroundCalibration
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Set the bitmap to use as the reference during the calibration
        //--------------------------------------------------------------------------------
        void setReference(BITMAP* bitmap) requires(BITMAP::Channels == 3);

        //--------------------------------------------------------------------------------
        /// @brief  Set the bitmap to use as the reference during the calibration
        //--------------------------------------------------------------------------------
        void setReference(BITMAP* bitmap) requires(BITMAP::Channels == 1);

        //--------------------------------------------------------------------------------
        /// @brief  Apply background calibration to a bitmap
        ///
        /// Note that the reference must have been set!
        //--------------------------------------------------------------------------------
        void calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 3);

        //--------------------------------------------------------------------------------
        /// @brief  Apply background calibration to a bitmap
        ///
        /// Note that the reference must have been set!
        //--------------------------------------------------------------------------------
        void calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 1);


    private:
        void computeParameters(
            const BITMAP* bitmap,
            double& redBackground, double& greenBackground, double& blueBackground,
            double& redMax, double& greenMax, double& blueMax
        ) const requires(BITMAP::Channels == 3);

        void computeParameters(
            const BITMAP* bitmap, double& background, double& max
        ) const requires(BITMAP::Channels == 1);


    private:
        double targetRedBackground;
        double targetGreenBackground;
        double targetBlueBackground;

        double targetRedMax;
        double targetGreenMax;
        double targetBlueMax;
    };

}
}


#include <astrophoto-toolbox/stacking/backgroundcalibration.hpp>
