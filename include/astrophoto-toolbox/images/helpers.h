#pragma once

#include <astrophoto-toolbox/images/bitmap.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Returns the conversion factor between two range of values
    //------------------------------------------------------------------------------------
    double getConversionFactor(range_t from, range_t to);


    //------------------------------------------------------------------------------------
    /// @brief  Compute the luminance component of a bitmap
    //------------------------------------------------------------------------------------
    DoubleGrayBitmap* computeLuminanceBitmap(Bitmap* bitmap);


    //------------------------------------------------------------------------------------
    /// @brief  Compute the median of a bitmap
    //------------------------------------------------------------------------------------
    double computeMedian(DoubleGrayBitmap* bitmap);


    //------------------------------------------------------------------------------------
    /// @brief  Ensure that a bitmap has the required format, or create a copy that does
    ///
    /// The caller must free the returned bitmap if it is different than the provided one.
    //------------------------------------------------------------------------------------
    template<class BITMAPTYPE>
    BITMAPTYPE* requiresFormat(
        Bitmap* bitmap, range_t range = RANGE_DEST, space_t space = SPACE_DEST
    )
    {
        if ((bitmap->isFloatingPoint() == BITMAPTYPE::FloatingPoint) &&
            (bitmap->channelSize() == BITMAPTYPE::ChannelSize) &&
            (bitmap->range() == (range != RANGE_DEST ? range : BITMAPTYPE::DefaultRange)) &&
            (bitmap->space() == (space != SPACE_DEST ? space : bitmap->space())))
        {
            return dynamic_cast<BITMAPTYPE*>(bitmap);
        }

        return new BITMAPTYPE(bitmap, range, space);
    }

}
