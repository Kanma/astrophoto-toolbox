#pragma once

#include <vector>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Holds the distance between two stars. Used by the star matching algorithm.
    //------------------------------------------------------------------------------------
    class StarsDistance
    {
    public:
        StarsDistance(uint8_t star1, uint8_t star2, float distance = 0.0f)
        {
            if (star1 < star2)
            {
                this->star1 = star1;
                this->star2 = star2;
            }
            else
            {
                this->star1 = star2;
                this->star2 = star1;
            }

            this->distance = distance;
        }

    public:
        bool operator<(const StarsDistance& dist) const
        {
            if (star1 < dist.star1)
                return true;
            else if (star1 > dist.star1)
                return false;
            else
                return (star2 < dist.star2);
        };

    public:
        uint8_t star1;
        uint8_t star2;
        float distance;
    };


    typedef std::vector<StarsDistance>  stars_distance_list_t;

}
}
