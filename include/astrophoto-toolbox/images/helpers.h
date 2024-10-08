/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/algorithms/histogram.h>
#include <astrophoto-toolbox/algorithms/math.h>


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
    template<class BITMAP>
    inline double computeMedian(BITMAP* bitmap, uint8_t channel = 0)
    {
        histogram_t histogram;
        return computeMedian(
            ((typename BITMAP::type_t*) bitmap->ptr()) + channel,
            bitmap->width() * bitmap->height(), histogram,
            (typename BITMAP::type_t) bitmap->maxRangeValue(), BITMAP::Channels
        );
    }


    //------------------------------------------------------------------------------------
    /// @brief  Compute the standard deviation of a bitmap
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    inline double computeStandardDeviation(BITMAP* bitmap, double& average, uint8_t channel = 0)
    {
        return computeStandardDeviation(
            ((typename BITMAP::type_t*) bitmap->ptr()) + channel,
            bitmap->width() * bitmap->height(), average, BITMAP::Channels
        );
    }


    //------------------------------------------------------------------------------------
    /// @brief  Substract two bitmaps (with the same type)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
        requires(std::is_integral_v<typename BITMAP::type_t>)
    void substract(BITMAP* bitmap1, BITMAP* bitmap2)
    {
        unsigned int width = std::min(bitmap1->width(), bitmap2->width());
        unsigned int height = std::min(bitmap1->height(), bitmap2->height());

        for (unsigned int y = 0; y < height; ++y)
        {
            typename BITMAP::type_t* p1 = bitmap1->data(y);
            typename BITMAP::type_t* p2 = bitmap2->data(y);

            for (unsigned int i = 0; i < width * BITMAP::Channels; ++i)
            {
                *p1 = (typename BITMAP::type_t)(std::max(long(*p1) - long(*p2), long(0)));
                ++p1;
                ++p2;
            }
        }
    }


    //------------------------------------------------------------------------------------
    /// @brief  Substract two bitmaps (with the same type)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
        requires(!std::is_integral_v<typename BITMAP::type_t>)
    void substract(BITMAP* bitmap1, BITMAP* bitmap2)
    {
        unsigned int width = std::min(bitmap1->width(), bitmap2->width());
        unsigned int height = std::min(bitmap1->height(), bitmap2->height());

        for (unsigned int y = 0; y < height; ++y)
        {
            typename BITMAP::type_t* p1 = bitmap1->data(y);
            typename BITMAP::type_t* p2 = bitmap2->data(y);

            for (unsigned int i = 0; i < width * BITMAP::Channels; ++i)
            {
                *p1 = std::max((typename BITMAP::type_t)(*p1 - *p2), (typename BITMAP::type_t)(0.0f));
                ++p1;
                ++p2;
            }
        }
    }


    //------------------------------------------------------------------------------------
    /// @brief  Remove hot pixels in a bitmap
    //------------------------------------------------------------------------------------
    void removeHotPixels(Bitmap* bitmap);


    //------------------------------------------------------------------------------------
    /// @brief  Ensure that a bitmap has the required format, or create a copy that does
    ///
    /// The caller must free the returned bitmap if it is different than the provided one.
    //------------------------------------------------------------------------------------
    template<class BITMAPTYPE>
    inline BITMAPTYPE* requiresFormat(
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
