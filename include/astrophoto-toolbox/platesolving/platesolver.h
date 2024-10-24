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
#include <astrophoto-toolbox/data/coordinates.h>
#include <string>

extern "C" {
    #include <astrometry/simplexy.h>
    #include <astrometry/index.h>
}


namespace astrophototoolbox {
namespace platesolving {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the plate solving operations
    ///
    /// The goal of this class is to perform 'plate solving', which is determining the
    /// celestial coordinates of an image containing stars.
    ///
    /// This class is a pure C++ wrapper/reimplementation of astrometry.net's library
    /// (which require external programs and Python scripts). Astronomy.net's software
    /// remains more complete and versatile, only a subset of its features are supported. 
    //------------------------------------------------------------------------------------
    class PlateSolver
    {
        //_____ Construction / Destruction __________
    public:
        PlateSolver();
        ~PlateSolver();


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Perform plate solving on a bitmap
        ///
        /// This is a shortcut to execute all the operations needed for plate solving,
        /// available individually as methods of this class.
        ///
        /// The method 'loadIndexes()' must have been called before this one.
        //--------------------------------------------------------------------------------
        bool run(
            Bitmap* bitmap, double minWidth = 0.1, double maxWidth = 180.0,
            time_t limit = 10
        );

        //--------------------------------------------------------------------------------
        /// @brief  Perform plate solving on a list of stars
        ///
        /// This is a shortcut to execute all the operations needed for plate solving,
        /// available individually as methods of this class.
        ///
        /// The method 'loadIndexes()' must have been called before this one.
        //--------------------------------------------------------------------------------
        bool run(
            const star_list_t& stars, const size2d_t& imageSize, double minWidth = 0.1,
            double maxWidth = 180.0, time_t limit = 10
        );

        //--------------------------------------------------------------------------------
        /// @brief  Detect the stars in the provided bitmap
        ///
        /// The result can be retrieved with 'getStarList()'. The detected stars are
        /// sorted according to their 'flux' and 'background'.
        ///
        /// This function is basically the equivalent in astrometry.net of:
        ///   - image2xy -d 2 -D 3 <image>
        ///   - resort_xylist(...)
        ///
        /// In order to successfully determine celestial coordinates (aka plate solving),
        /// the list of stars must be uniformized and cut to keep only the most relevant
        /// ones. This can be done either manually by calling the appropriate methods,
        /// or by setting 'uniformize' and 'cut' to 'true' (this will use the default
        /// parameter value for each method).
        //--------------------------------------------------------------------------------
        bool detectStars(Bitmap* bitmap, bool uniformize = false, bool cut = false);

        //--------------------------------------------------------------------------------
        /// @brief  Uniformize the list of detected stars
        ///
        /// This is needed to help the plate solving algorithm. The goal is to group the
        /// stars in a grid, and sort them using a "first star in each cell, second star
        /// in each cell, ..." method.
        ///
        /// It is assumed that the stars are already sorted by their flux (this is done by
        /// 'detectStars()').
        ///
        /// This function is a reimplementation of astrometry.net's "uniformize.py".
        //--------------------------------------------------------------------------------
        bool uniformize(unsigned int nbBoxes = 10);

        //--------------------------------------------------------------------------------
        /// @brief  Keep only the first 'nb' stars in the list
        //--------------------------------------------------------------------------------
        void cut(unsigned int nb = 1000);

        //--------------------------------------------------------------------------------
        /// @brief  Determine the celestial coordinates of the detected stars (aka plate
        ///         solving)
        ///
        /// It is expected that the list of stars has been uniformized and cut
        /// appropriately.
        //--------------------------------------------------------------------------------
        bool solve(double minWidth = 0.1, double maxWidth = 180.0, time_t limit = 10);

        //--------------------------------------------------------------------------------
        /// @brief  Set the list of detected stars
        ///
        /// Useful if you loaded it from a file.
        //--------------------------------------------------------------------------------
        inline void setStars(const star_list_t stars, const size2d_t& imageSize)
        {
            this->stars = stars;
            this->imageSize = imageSize;

            coordinates = Coordinates();
            pixelScale = 0.0;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Retrieve the list of detected stars 
        //--------------------------------------------------------------------------------
        inline const star_list_t& getStars() const
        {
            return stars;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Retrieve the additional infos about the detection process
        ///
        /// Mainly needed to maintain compatibility with astrometry.net.
        //--------------------------------------------------------------------------------
        inline const size2d_t& getImageSize() const
        {
            return imageSize;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the celestial coordinates that have been found 
        //--------------------------------------------------------------------------------
        inline const Coordinates& getCoordinates() const
        {
            return coordinates;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the size of a pixel in arcseconds
        //--------------------------------------------------------------------------------
        inline double getPixelSize() const
        {
            return pixelScale;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Load the index files found in the specified folder
        //--------------------------------------------------------------------------------
        bool loadIndexes(const std::string& folder);

        //--------------------------------------------------------------------------------
        /// @brief  Unload the index files from the memory
        //--------------------------------------------------------------------------------
        void clearIndexes();

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        ///
        /// Only useful in a multithreading scenario, where this method is called from a
        /// different thread than the one doing the processing.
        //--------------------------------------------------------------------------------
        void cancel();


    private:
        //--------------------------------------------------------------------------------
        /// @brief  Sort the starts in 'params' according to their 'flux' and 'background'
        ///
        /// This is a reimplementation of astrometry.net's "resort_xylist()", that doesn't
        /// need to open and save FITS files.
        //--------------------------------------------------------------------------------
        std::vector<int> sort(const simplexy_t& params, bool ascending);

        //--------------------------------------------------------------------------------
        /// @brief  Returns a list of indexes matching the provided angles (in degrees)
        ///
        /// 'loadIndexes()' must have already been called.
        //--------------------------------------------------------------------------------
        std::vector<index_t*> filterIndexes(float minWidth = 0.1f, float maxWidth = 180.0f);


        //_____ Attributes __________
    private:
        star_list_t stars;
        size2d_t imageSize;

        Coordinates coordinates;
        double pixelScale = 0.0;    // in arcsec/pixel

        std::vector<index_t*> indexes;

        bool cancelled = false;
    };

}
}
