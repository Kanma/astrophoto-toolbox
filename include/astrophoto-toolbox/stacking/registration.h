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
        //--------------------------------------------------------------------------------
        const star_list_t registerBitmap(Bitmap* bitmap) const;


    private:
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
    };

}
}
