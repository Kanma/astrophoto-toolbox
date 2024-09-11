#pragma once

#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/data/size.h>
#include <astrophoto-toolbox/data/transformation.h>
#include <astrophoto-toolbox/stacking/starsdistance.h>
#include <astrophoto-toolbox/stacking/votingpair.h>


namespace astrophototoolbox {
namespace stacking {

    //------------------------------------------------------------------------------------
    /// @brief  Allows to compute the transformation between two list of stars
    ///
    /// This class is a reimplementation of the relevant parts of DeepSkyStacker's
    /// 'CMatchingStars' class, adapted to astrophoto-toolbox needs.
    //------------------------------------------------------------------------------------
    class StarMatcher
    {
    public:
        bool computeTransformation(
            const star_list_t& fromStars, const star_list_t& toStars, const size2d_t& imageSize,
            Transformation& transformation
        );

    private:
        bool computeLargeTriangleTransformation(Transformation& transforms);

        void computeStarDistances(const point_list_t& stars, stars_distance_list_t& distances);

        void initVotingGrid(voting_pair_list_t& votingPairs);

        bool computeSigmaClippingTransformation(
            const voting_pair_list_t& votingPairs, Transformation& transforms
        );

        bool computeCoordinatesTransformation(
            voting_pair_list_t& votingPairs, Transformation& transforms
        );

        bool computeTransformation(
            const voting_pair_list_t & votingPairs, Transformation& transforms
        );

        double validateTransformation(
            const voting_pair_list_t & testedPairs, Transformation& transforms
        );

    private:
        point_list_t references;
        point_list_t targets;
        size2d_t imageSize;
        stars_distance_list_t referenceDistances;
        stars_distance_list_t targetDistances;
    	std::vector<int> referenceIndices;
	    std::vector<int> targetIndices;
        voting_pair_list_t votedPairs;

        static constexpr double MAXSTARDISTANCEDELTA = 2.0;
    };

}
}
