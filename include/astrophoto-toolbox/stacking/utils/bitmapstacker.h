/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <filesystem>
#include <vector>
#include <string>


namespace astrophototoolbox {
namespace stacking {
namespace utils {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to stack bitmaps, using the 'median' method
    ///
    /// The median value of the pixels in the stack is computed for each pixel.
    ///
    /// Internally, this class use several temporary files to reduce the memory load
    /// during computation.
    ///
    /// This class is a reimplementation of the relevant parts of DeepSkyStacker's
    /// 'CMultiBitmap' class, adapted to astrophoto-toolbox needs.
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class BitmapStacker
    {
    public:
        ~BitmapStacker();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Setup the stacker
        //--------------------------------------------------------------------------------
        void setup(
            unsigned int nbBitmaps, const std::filesystem::path& tempFolder = "",
            unsigned long maxFileSize = 50000000L
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add a bitmap to the list of bitmaps to be stacked
        ///
        /// It is expected that all the bitmaps have the same dimensions and range.
        ///
        /// Note: 'setup()' must have been called before this method.
        //--------------------------------------------------------------------------------
        bool addBitmap(BITMAP* bitmap);

        //--------------------------------------------------------------------------------
        /// @brief  Performs the stacking of all the images that were added
        //--------------------------------------------------------------------------------
        BITMAP* process() const;

        //--------------------------------------------------------------------------------
        /// @brief  Delete all the temporary files
        ///
        /// After this, the stacker object can be reused with an unrelated set of bitmaps.
        ///
        /// Note: automatically done when the object is destroyed
        //--------------------------------------------------------------------------------
        void clear();

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if the stacker has been initialised with the 'setup()'
        ///         method
        //--------------------------------------------------------------------------------
        inline bool isInitialised() const
        {
            return (nbBitmaps != 0);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of bitmaps to be stacked
        //--------------------------------------------------------------------------------
        inline unsigned int nbStackedBitmaps() const
        {
            return nbAddedBitmaps;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        ///
        /// Only useful in a multithreading scenario, where this method is called from a
        /// different thread than the one doing the processing.
        //--------------------------------------------------------------------------------
        inline void cancel()
        {
            cancelled = true;
        }


    private:
        //--------------------------------------------------------------------------------
        /// @brief  Determine how the rows of all the images must be split in different
        ///         temporary files
        //--------------------------------------------------------------------------------
        void initPartFiles();

        //--------------------------------------------------------------------------------
        /// @brief  Stack the rows located in the provided buffer
        //--------------------------------------------------------------------------------
        void stack(
            unsigned int startRow, unsigned int endRow, unsigned int nbRowElements,
            typename BITMAP::type_t* buffer, BITMAP* output
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Stack one row by combining the provided row pointers (using the
        ///         'median' method)
        //--------------------------------------------------------------------------------
        void combine(
            unsigned int row, const std::vector<typename BITMAP::type_t*>& srcRows,
            BITMAP* output
        ) const requires(BITMAP::Channels == 3);

        //--------------------------------------------------------------------------------
        /// @brief  Stack one row by combining the provided row pointers (using the
        ///         'median' method)
        //--------------------------------------------------------------------------------
        void combine(
            unsigned int row, const std::vector<typename BITMAP::type_t*>& srcRows,
            BITMAP* output
        ) const requires(BITMAP::Channels == 1);


    private:
        //--------------------------------------------------------------------------------
        /// @brief  Infos about a given temporary file
        //--------------------------------------------------------------------------------
        struct part_file_t {
            std::filesystem::path filename;
            unsigned int startRow;
            unsigned int endRow;
        };


    private:
        unsigned int nbBitmaps = 0;
        unsigned int nbAddedBitmaps = 0;
        unsigned int width = 0;
        unsigned int height = 0;
        range_t range = RANGE_ONE;
        unsigned long maxFileSize = 0;

        std::filesystem::path tempFolder;
        std::vector<part_file_t> partFiles;
        bool cancelled = false;
    };

}
}
}


#include <astrophoto-toolbox/stacking/utils/bitmapstacker.hpp>
