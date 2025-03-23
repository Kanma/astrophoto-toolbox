/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/data/coordinates.h>
#include <astrophoto-toolbox/data/point.h>
#include <astrophoto-toolbox/data/size.h>

extern "C" {
    #include <astrometry/sip.h>
}


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represents an astronomical coordinates system localized at the center of
    ///         an image
    //------------------------------------------------------------------------------------
    class CoordinatesSystem
    {
        //_____ Construction / Destruction __________
    public:
        CoordinatesSystem();

        CoordinatesSystem(
            const size2d_t& imageSize, const Coordinates& coords, double pixelSize,
            double raAngle, const tan_t& wcstan
        );


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Returns the position in pixels of some coordinates
        //--------------------------------------------------------------------------------
        point_t convert(const Coordinates& coords) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the coordinates of some position in pixels
        //--------------------------------------------------------------------------------
        Coordinates convert(const point_t& position) const;

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if some coordinates are inside the image
        //--------------------------------------------------------------------------------
        bool isInsideImage(const Coordinates& coords) const;

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if a point is inside the image
        //--------------------------------------------------------------------------------
        bool isInsideImage(const point_t& point) const;

        //------------------------------------------------------------------------------------
        /// @brief  Draw RA and DEC axes into a color bitmap
        //------------------------------------------------------------------------------------
        template<class BITMAP>
            requires(BITMAP::Channels == 3)
        void drawAxes(BITMAP* bitmap, unsigned int length = 0) const
        {
            uint8_t* layer = drawAxes(bitmap->width(), bitmap->height(), length);
            mix(bitmap, layer);
        }

        //------------------------------------------------------------------------------------
        /// @brief  Returns a RGBA buffer containing RA and DEC axes
        //------------------------------------------------------------------------------------
        uint8_t* drawAxes(unsigned int width, unsigned int height, unsigned int length) const;

        //------------------------------------------------------------------------------------
        /// @brief  Draw a grid into a color bitmap
        ///
        /// If the dimensions of the bitmap aren't the same than the ones of the image from
        /// which the coordinates system was generated, the grid is scaled to cover the whole
        /// bitmap.
        //------------------------------------------------------------------------------------
        template<class BITMAP>
            requires(BITMAP::Channels == 3)
        void drawGrid(BITMAP* bitmap, float fontSize = -1.0f) const
        {
            uint8_t* layer = drawGrid(bitmap->width(), bitmap->height());
            mix(bitmap, layer);
        }

        //------------------------------------------------------------------------------------
        /// @brief  Returns a RGBA buffer containing a grid
        ///
        /// If the requested dimensions aren't the same than the ones of the image from
        /// which the coordinates system was generated, the grid is scaled to cover the whole
        /// buffer.
        //------------------------------------------------------------------------------------
        uint8_t* drawGrid(unsigned int width, unsigned int height, float fontSize = -1.0f) const;

        bool isNull() const;


        //_____ Internal methods __________
    private:
        void computeGridParameters(
            double inMin, double inMaxValue, double& outMin, double& outMax, double& step
        ) const;

        bool findRALabelLocation(double ra, double decmin, double decmax, double& dec) const;

        bool findDECLabelLocation(double dec, double ramin, double ramax, double& ra) const;

        //------------------------------------------------------------------------------------
        /// @brief  Mix a RGBA buffer and a color bitmap
        ///
        /// The RGBA buffer is released by this method.
        //------------------------------------------------------------------------------------
        template<class BITMAP>
            requires(BITMAP::Channels == 3)
        void mix(BITMAP* bitmap, uint8_t* layer) const
        {
            double maxRangeValue = (double) bitmap->maxRangeValue();

            uint8_t* pSrc = layer;
            for (unsigned int y = 0; y < bitmap->height(); ++y)
            {
                typename BITMAP::type_t* pDst = bitmap->data(y);

                for (unsigned int i = 0; i < bitmap->width(); ++i)
                {
                    uint8_t alpha = pSrc[3];
                    if (alpha != 0)
                    {
                        double alpha2 = ((double) alpha / 255.0);

                        pDst[0] = typename BITMAP::type_t((((double) pDst[0] / maxRangeValue) * (1.0 - alpha2) + ((double) pSrc[0] / 255.0) * alpha2) * maxRangeValue);
                        pDst[1] = typename BITMAP::type_t((((double) pDst[1] / maxRangeValue) * (1.0 - alpha2) + ((double) pSrc[1] / 255.0) * alpha2) * maxRangeValue);
                        pDst[2] = typename BITMAP::type_t((((double) pDst[2] / maxRangeValue) * (1.0 - alpha2) + ((double) pSrc[2] / 255.0) * alpha2) * maxRangeValue);
                    }

                    pDst += 3;
                    pSrc += 4;
                }
            }

            delete[] layer;
        }


        //_____ Attributes __________
    private:
        size2d_t imageSize;
        Coordinates coords;
        double pixelSize;
        double raAngle;
        point_t center;
        double dx;
        double dy;
        tan_t wcstan;
    };

}
