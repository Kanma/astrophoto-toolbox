#pragma once

#include <vector>
#include <stdint.h>


namespace astrophototoolbox {
namespace stacking {

    constexpr int VPFLAG_ACTIVE = 0x00000001;
    constexpr int VPFLAG_USED   = 0x00000002;


    //------------------------------------------------------------------------------------
    /// @brief  Used by the star matching algorithm to holds the number of votes in favor
    ///         of the association of two stars (from different lists)
    //------------------------------------------------------------------------------------
    class VotingPair
    {
    public:
        VotingPair() = default;

        VotingPair(uint8_t refStar, uint8_t targetStar)
        : refStar(refStar), targetStar(targetStar)
        {
        }

    public:
        bool isActive() const
        {
            return (flags & VPFLAG_ACTIVE);
        }

        void setActive(bool active)
        {
            if (active)
                flags |= VPFLAG_ACTIVE;
            else
                flags &=~VPFLAG_ACTIVE;
        }

        bool isUsed() const
        {
            return (flags & VPFLAG_USED);
        }

        void setUsed(bool used)
        {
            if (used)
                flags |= VPFLAG_USED;
            else
                flags &=~VPFLAG_USED;
        }

    public:
        uint8_t refStar = 0;
        uint8_t targetStar = 0;
        unsigned int nbVotes = 0;
        unsigned int flags = VPFLAG_ACTIVE;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represent a list of voting pairs
    //------------------------------------------------------------------------------------
    typedef std::vector<VotingPair> voting_pair_list_t;

}
}
