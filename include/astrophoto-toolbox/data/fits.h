#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/data/size.h>
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
        /// @brief  Read the bitmap with the given name from the FITS file
        //--------------------------------------------------------------------------------
        star_list_t readStars(
            const std::string& name, size2d_t* imageSize = nullptr
        );

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th bitmap from the FITS file
        //--------------------------------------------------------------------------------
        star_list_t readStars(int index = 0, size2d_t* imageSize = nullptr);


    private:
        Bitmap* readBitmapFromCurrentHDU();
        star_list_t readStarsFromCurrentHDU(size2d_t* imageSize = nullptr);

        bool gotoHDU(const std::string& name, int type);
        bool gotoHDU(int index, int type);


        //_____ Attributes __________
    private:
        fitsfile* _file = nullptr;
    };

}
