/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/stacking/processing/masterdark.h>
#include <astrophoto-toolbox/stacking/processing/lightframes.h>
#include <astrophoto-toolbox/stacking/processing/registration.h>
#include <astrophoto-toolbox/stacking/processing/stacking.h>
#include <filesystem>
#include <vector>
#include <string>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the stacking-related operations
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class Stacking
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Setup the stacking with a specific folder to work in
        ///
        /// During stacking, calibrated images, masters and temporary files will be stored
        /// in this folder.
        //--------------------------------------------------------------------------------
        void setup(const std::filesystem::path& folder);

        //--------------------------------------------------------------------------------
        /// @brief  Load the list of images from a configuration file ('stacking.txt' in
        ///         the working folder)
        //--------------------------------------------------------------------------------
        bool load();

        //--------------------------------------------------------------------------------
        /// @brief  Save the list of images into a configuration file ('stacking.txt' in
        ///         the working folder)
        //--------------------------------------------------------------------------------
        bool save();

        //--------------------------------------------------------------------------------
        /// @brief  Add a dark frame
        //--------------------------------------------------------------------------------
        bool addDarkFrame(const std::string& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of dark frames
        //--------------------------------------------------------------------------------
        inline size_t nbDarkFrames() const
        {
            return darkFrames.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Add a light frame
        //--------------------------------------------------------------------------------
        bool addLightFrame(const std::string& filename, bool reference = false);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of light frames
        //--------------------------------------------------------------------------------
        inline size_t nbLightFrames() const
        {
            return lightFrames.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Set the light frame to use as the reference during stacking
        ///
        /// Note: changing it invalidates all the light frames already processed, which
        /// will need to be reprocessed again.
        //--------------------------------------------------------------------------------
        inline void setReference(size_t index)
        {
            referenceFrame = index;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the index of the reference light frame
        //--------------------------------------------------------------------------------
        inline size_t getReference() const
        {
            return referenceFrame;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Stack the light frames
        ///
        /// This method will block until the stacking is done, which takes a while.
        //--------------------------------------------------------------------------------
        BITMAP* process(int luminancyThreshold = -1);


    private:
        const std::string getCalibratedFilename(const std::string& path);


    private:
        std::filesystem::path folder;

        std::vector<std::string> darkFrames;
        std::vector<std::string> lightFrames;

        size_t referenceFrame = -1;

        processing::MasterDarkGenerator<BITMAP> masterDarkGenerator;
        processing::LightFrameProcessor<BITMAP> lightFrameProcessor;
        processing::RegistrationProcessor<BITMAP> registrationProcessor;
        processing::FramesStacker<BITMAP> framesStacker;
    };

}
}


#include <astrophoto-toolbox/stacking/stacking.hpp>
