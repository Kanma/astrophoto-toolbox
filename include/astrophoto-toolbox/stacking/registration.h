/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/rect.h>
#include <astrophoto-toolbox/data/star.h>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform the registration (star detection) of an image
    ///
    /// This class is a reimplementation of the relevant parts of DeepSkyStacker's
    /// 'CRegisteredFrame' class, adapted to astrophoto-toolbox needs.
    //------------------------------------------------------------------------------------
    class Registration
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Detect the stars in a bitmap
        ///
        /// Note that the returned list of stars is meant to be used stacking-related
        /// purposes. In particular, only a few stars (the most relevant ones for
        /// stacking) are in the list.
        ///
        /// The luminancy threshold to use can be either specified (between 0 and 100),
        /// or you can ask to search for a value that produces a good amount of stars for
        /// the stacking algorithm (by setting 'luminancyThreshold' to -1).
        ///
        /// Note that to compare the overall quality of two lists of stars, the same
        /// threshold must have been used during registration.
        //--------------------------------------------------------------------------------
        const star_list_t registerBitmap(Bitmap* bitmap, int luminancyThreshold = 10);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the last luminancy threshold that was used/found
        //--------------------------------------------------------------------------------
        inline int getLuminancyThreshold() const
        {
            return luminancyThreshold;
        }


    private:
        //--------------------------------------------------------------------------------
        /// @brief  Detect the stars in a luminance bitmap, with a threshold previously
        ///         specified
        //--------------------------------------------------------------------------------
        void registerBitmapWithFixedThreshold(
            DoubleGrayBitmap* luminance, double median, star_list_t& stars
        );

        //--------------------------------------------------------------------------------
        /// @brief  Search a good threshold and detect the stars in a luminance bitmap at
        ///         the same time
        //--------------------------------------------------------------------------------
        void registerBitmapAndSearchThreshold(
            DoubleGrayBitmap* luminance, double median, star_list_t& stars
        );

        //--------------------------------------------------------------------------------
        /// @brief  Detect the stars in a rectangular part of the bitmap
        //--------------------------------------------------------------------------------
        int registerRect(
            const DoubleGrayBitmap* bitmap, const rect_t& rect, double background,
            double minLuminancy, star_set_t& stars
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Affine the coordinates of a detected star
        //--------------------------------------------------------------------------------
        bool computeStarCenter(
            const DoubleGrayBitmap* bitmap, point_t& position, double& radius,
            double background
        ) const;


    private:
        static constexpr int STARMAXSIZE = 50;
        static constexpr double ROUNDNESS_TOLERANCE = 2.0;

        int luminancyThreshold = 10;
    };

}
}
