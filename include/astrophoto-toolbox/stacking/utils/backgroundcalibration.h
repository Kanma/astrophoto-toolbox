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
namespace utils {

    struct background_calibration_parameters_t
    {
        double redBackground;
        double greenBackground;
        double blueBackground;

        double redMax;
        double greenMax;
        double blueMax;
    };


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
        ///
        /// Parameters will be computed form the bitmap.
        //--------------------------------------------------------------------------------
        void setReference(BITMAP* bitmap);

        //--------------------------------------------------------------------------------
        /// @brief  Set the parameters to use during the calibration
        //--------------------------------------------------------------------------------
        inline void setParameters(const background_calibration_parameters_t& parameters)
        {
            this->parameters = parameters;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the parameters used during the calibration
        //--------------------------------------------------------------------------------
        inline background_calibration_parameters_t getParameters() const
        {
            return parameters;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Apply background calibration to a bitmap
        ///
        /// Note that either the reference bitmap or the parameters must have been set!
        //--------------------------------------------------------------------------------
        void calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 3);

        //--------------------------------------------------------------------------------
        /// @brief  Apply background calibration to a bitmap
        ///
        /// Note that either the reference bitmap or the parameters must have been set!
        //--------------------------------------------------------------------------------
        void calibrate(BITMAP* bitmap) const requires(BITMAP::Channels == 1);


    private:
        void computeParameters(
            const BITMAP* bitmap, background_calibration_parameters_t& parameters
        ) const requires(BITMAP::Channels == 3);

        void computeParameters(
            const BITMAP* bitmap, background_calibration_parameters_t& parameters
        ) const requires(BITMAP::Channels == 1);


    private:
        background_calibration_parameters_t parameters;
    };

}
}
}


#include <astrophoto-toolbox/stacking/utils/backgroundcalibration.hpp>
