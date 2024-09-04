#pragma once

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/astrometry/starlist.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform all the astrometry operations
    //------------------------------------------------------------------------------------
    class Astrometry
    {
        //_____ Construction / Destruction __________
    public:
        Astrometry();
        ~Astrometry();


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Detect the stars in the provided bitmap
        ///
        /// The result can be retrieved with 'getStarList()'
        //--------------------------------------------------------------------------------
        bool detectStars(Bitmap* bitmap);

        //--------------------------------------------------------------------------------
        /// @brief  Retrieve the list of stars detected
        //--------------------------------------------------------------------------------
        inline const star_list_t& getStarList() const
        {
            return starList;
        }


        //_____ Attributes __________
    private:
        star_list_t starList;
    };

}
