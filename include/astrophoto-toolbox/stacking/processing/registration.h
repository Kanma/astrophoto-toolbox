/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/stacking/starmatcher.h>
#include <filesystem>
#include <string>
#include <memory>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the registration-related operations (stars detection
    ///         and transformation from a reference frame computation)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class RegistrationProcessor
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Register the light frame file to use as the reference, and save the
        ///         list of detected stars at the given destination path
        ///
        /// If not specified, the luminancy threshold is estimated on the reference frame
        /// and used as-is on all the other frames.
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file (which can be the same as the light frame one).
        ///
        /// It is expected that the light frame has been properly processed.
        //--------------------------------------------------------------------------------
        star_list_t processReference(
            const std::string& lightFrame, int luminancyThreshold=-1,
            const std::string& destination = ""
        );

        //--------------------------------------------------------------------------------
        /// @brief  Register the light frame bitmap to use as the reference, and save the
        ///         list of detected stars at the given destination path
        ///
        /// If not specified, the luminancy threshold is estimated on the reference frame
        /// and used as-is on all the other frames.
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file.
        ///
        /// It is expected that the light frame has been properly processed.
        //--------------------------------------------------------------------------------
        star_list_t processReference(
            const std::shared_ptr<BITMAP>& lightFrame, int luminancyThreshold=-1,
            const std::string& destination = ""
        );

        //--------------------------------------------------------------------------------
        /// @brief  Register a light frame file, and save the list of detected stars and
        ///         the transformation from the reference frame at the given destination
        ///         path
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file (which can be the same as the light frame one).
        ///
        /// It is expected that the light frame has been properly processed.
        //--------------------------------------------------------------------------------
        std::tuple<star_list_t, Transformation> process(
            const std::string& lightFrame, const std::string& destination = ""
        );

        //--------------------------------------------------------------------------------
        /// @brief  Register a light frame file, and save the list of detected stars and
        ///         the transformation from the reference frame at the given destination
        ///         path
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file.
        ///
        /// It is expected that the light frame has been properly processed.
        //--------------------------------------------------------------------------------
        std::tuple<star_list_t, Transformation> process(
            const std::shared_ptr<BITMAP>& lightFrame, const std::string& destination = ""
        );


    private:
        Registration registration;
        int luminancyThreshold;
        StarMatcher matcher;
        star_list_t referenceStars;
    };

}
}


#include <astrophoto-toolbox/stacking/processing/registration.hpp>
