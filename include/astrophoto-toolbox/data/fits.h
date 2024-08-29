#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
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
        /// @brief  Read a bitmap from the FITS file
        ///
        /// If no name is specified, the first image is retrieved
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(const std::string& name = "");

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th bitmap from the FITS file
        //--------------------------------------------------------------------------------
        Bitmap* readBitmap(int index);


    private:
        Bitmap* readBitmapFromCurrentHDU();


        //_____ Attributes __________
    private:
        fitsfile* _file = nullptr;
    };

}
