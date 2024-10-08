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
#include <astrophoto-toolbox/stacking/bitmapstacker.h>
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
        ~Stacking();


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
        bool save() const;

        //--------------------------------------------------------------------------------
        /// @brief  Add a dark frame
        //--------------------------------------------------------------------------------
        bool addDarkFrame(const std::string& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Computes the master dark frame from all the added dark frames
        //--------------------------------------------------------------------------------
        bool computeMasterDark();

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if the master dark frame exists
        //--------------------------------------------------------------------------------
        bool hasMasterDark() const;

        //--------------------------------------------------------------------------------
        /// @brief  Retrieves the master dark frame
        //--------------------------------------------------------------------------------
        BITMAP* getMasterDark();

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
        /// @brief  Process the light frames, to be ready to be stacked
        ///
        /// Only the images not already done are processed, so you can repeatedly call
        /// this method during live stacking.
        //--------------------------------------------------------------------------------
        bool processLightFrames();

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of light frames
        //--------------------------------------------------------------------------------
        inline size_t nbLightFrames() const
        {
            return lightFrames.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of light frames already processed
        //--------------------------------------------------------------------------------
        inline size_t nbProcessedLightFrames() const
        {
            return nbLightFramesCalibrated;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Set the light frame to use as the reference during stacking
        ///
        /// Note: changing it invalidates all the light frames already processed, which
        /// will need to be reprocessed again.
        //--------------------------------------------------------------------------------
        inline void setReference(size_t index)
        {
            if (index != referenceFrame)
                invalidateLights();

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
        /// @brief  Indicates if a reference light frame was set
        //--------------------------------------------------------------------------------
        inline bool hasReference() const
        {
            return (referenceFrame >= 0) && (referenceFrame < lightFrames.size());
        }

        //--------------------------------------------------------------------------------
        /// @brief  Stack the light frames
        ///
        /// This method can be repeatedly called during live stacking.
        //--------------------------------------------------------------------------------
        BITMAP* process(unsigned int nbExpectedLightFrames = 0);


    private:
        void detectHotPixels() requires(BITMAP::Channels == 3);
        void detectHotPixels() requires(BITMAP::Channels == 1);

        void removeHotPixels(BITMAP* bitmap) const requires(BITMAP::Channels == 3);
        void removeHotPixels(BITMAP* bitmap) const requires(BITMAP::Channels == 1);

        void clear();
        void clearMasterDark();

        void invalidateDarks();
        void invalidateLights();

        BITMAP* loadBitmap(
            const std::filesystem::path& path, point_list_t* hotPixels = nullptr,
            star_list_t* stars = nullptr, Transformation* transformation = nullptr
        ) const;

        bool saveBitmap(
            BITMAP* bitmap, const std::filesystem::path& path,
            point_list_t* hotPixels = nullptr, star_list_t* stars = nullptr,
            Transformation* transformation = nullptr
        ) const;


    private:
        std::filesystem::path folder;

        std::vector<std::string> darkFrames;
        std::vector<std::string> lightFrames;

        BITMAP* masterDark = nullptr;
        point_list_t hotPixels;

        size_t referenceFrame = -1;
        bool lightFramesCalibrated = false;
        size_t nbLightFramesCalibrated = 0;

        BitmapStacker<BITMAP> stacker;
    };

}
}


#include <astrophoto-toolbox/stacking/stacking.hpp>
