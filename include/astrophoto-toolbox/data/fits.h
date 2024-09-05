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
            const star_list_t& stars, const star_detection_info_t& infos,
            const std::string& name = "STARS", bool overwrite = false
        );

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
        star_list_t readStarList(
            const std::string& name, star_detection_info_t* infos = nullptr
        );

        //--------------------------------------------------------------------------------
        /// @brief  Read the n-th bitmap from the FITS file
        //--------------------------------------------------------------------------------
        star_list_t readStarList(int index = 0, star_detection_info_t* infos = nullptr);


    private:
        Bitmap* readBitmapFromCurrentHDU();
        star_list_t readStarListFromCurrentHDU(star_detection_info_t* infos = nullptr);

        bool gotoHDU(const std::string& name, int type);
        bool gotoHDU(int index, int type);


        //_____ Attributes __________
    private:
        fitsfile* _file = nullptr;
    };

}
