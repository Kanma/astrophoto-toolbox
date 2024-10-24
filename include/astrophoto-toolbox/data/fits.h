/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/data/size.h>
#include <astrophoto-toolbox/data/transformation.h>
#include <fitsio.h>
#include <string>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Container for a FITS file
    //------------------------------------------------------------------------------------
    class FITS
    {
        //_____ Construction / Destruction __________
    public:
        FITS();
        ~FITS();


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Open a FITS file
        //--------------------------------------------------------------------------------
        bool open(const std::string& filename, bool readOnly = true);

        //--------------------------------------------------------------------------------
        /// @brief  Create a new FITS file
        //--------------------------------------------------------------------------------
        bool create(const std::string& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Close the FITS file
        //--------------------------------------------------------------------------------
        void close();

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of HDUs (header-data-units) in the FITS file
        //--------------------------------------------------------------------------------
        int nbHDUs() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of images in the FITS file
        //--------------------------------------------------------------------------------
        int nbImages() const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of tables in the FITS file
        //--------------------------------------------------------------------------------
        int nbTables() const;

        //--------------------------------------------------------------------------------
        /// @brief  Add a bitmap into the FITS file
        //--------------------------------------------------------------------------------
        bool write(Bitmap* bitmap, const std::string& name = "");

        //--------------------------------------------------------------------------------
        /// @brief  Add a list of stars into the FITS file
        //--------------------------------------------------------------------------------
        bool write(
            const star_list_t& stars, const size2d_t& imageSize,
            const std::string& name = "STARS", bool overwrite = false
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add a list of points into the FITS file
        //--------------------------------------------------------------------------------
        bool write(
            const point_list_t& points, const std::string& name = "POINTS",
            bool overwrite = false
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add a transformation into the FITS file
        //--------------------------------------------------------------------------------
        bool write(
            const Transformation& transformation, const std::string& name = "TRANSFORMS",
            bool overwrite = false
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add the keywords needed by astrometry.net's 'astrometry-engine'
        ///         executable, that performs plate solving.
        ///
        /// Only make sense in a file containing a list of stars, ready to be solved.
        ///
        /// For maximum compatibility.
        //--------------------------------------------------------------------------------
        bool writeAstrometryNetKeywords(const size2d_t& imageSize);

        //--------------------------------------------------------------------------------
        /// @brief  Read the bitmap with the given name from the FITS file
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(const std::string& name);

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th bitmap from the FITS file
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(int index = 0);

        //--------------------------------------------------------------------------------
        /// @brief  Read the list of stars with the given name from the FITS file
        //--------------------------------------------------------------------------------
        star_list_t readStars(
            const std::string& name, size2d_t* imageSize = nullptr
        );

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th list of stars from the FITS file
        //--------------------------------------------------------------------------------
        star_list_t readStars(int index = 0, size2d_t* imageSize = nullptr);

        //--------------------------------------------------------------------------------
        /// @brief  Read the list of points with the given name from the FITS file
        //--------------------------------------------------------------------------------
        point_list_t readPoints(const std::string& name);

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th list of points from the FITS file
        //--------------------------------------------------------------------------------
        point_list_t readPoints(int index = 0);

        //--------------------------------------------------------------------------------
        /// @brief  Read the transformation with the given name from the FITS file
        //--------------------------------------------------------------------------------
        Transformation readTransformation(const std::string& name);

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th transformation from the FITS file
        //--------------------------------------------------------------------------------
        Transformation readTransformation(int index = 0);


        //_____ Static methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Determine if a file is a FITS one, by inspecting its header
        //--------------------------------------------------------------------------------
        static bool isFITS(const std::string& filename);


        //_____ Internal methods __________
    private:
        Bitmap* readBitmapFromCurrentHDU();
        star_list_t readStarsFromCurrentHDU(size2d_t* imageSize = nullptr);
        point_list_t readPointsFromCurrentHDU();
        Transformation readTransformationFromCurrentHDU();

        bool gotoHDU(const std::string& name, int type);
        bool gotoHDU(int index, int type, const std::string& datatype = "");


        //_____ Attributes __________
    private:
        fitsfile* _file = nullptr;
    };

}
