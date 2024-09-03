#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/astrometry/starlist.h>
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
        /// @brief  Add a bitmap into the FITS file
        //--------------------------------------------------------------------------------
        bool write(Bitmap* bitmap, const std::string& name = "");

        //--------------------------------------------------------------------------------
        /// @brief  Add a list of stars into the FITS file
        //--------------------------------------------------------------------------------
        bool write(
            const star_list_t& starList, const std::string& name = "STARS",
            bool overwrite = false
        );

        //--------------------------------------------------------------------------------
        /// @brief  Read the bitmap with the given name from the FITS file
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(const std::string& name);

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th bitmap from the FITS file
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(int index = 0);


    private:
        Bitmap* readBitmapFromCurrentHDU();


        //_____ Attributes __________
    private:
        fitsfile* _file = nullptr;
    };

}
