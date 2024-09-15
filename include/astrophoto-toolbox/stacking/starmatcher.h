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
        //------------------------------------------------------------------------------------
        /// @brief  Compute the transformation between two list of stars
        ///
        /// Originally, this algorithm expects images to have been processed using flats &
        /// darks. In those cases, hot pixels are supposed to have been removed from the
        /// images, and not be present in the star lists.
        ///
        /// Should you want to process images with hot pixels in them, the tranformation will
        /// most likely be almost 0 (since hot pixels are at the same position in each image).
        ///
        /// To avoid that, you can provide a minimum distance estimate to this method. Then,
        /// when a transformation smaller that that is computed, the "stars" using to compute
        /// it are discarded as begin hot pixels, and a new computation occurs, until either
        /// a good transformation (bigger than the minimum distance) is found, or it is not
        /// possible to find one anymore.
        //------------------------------------------------------------------------------------
        bool computeTransformation(
            const star_list_t& fromStars, const star_list_t& toStars, const size2d_t& imageSize,
            Transformation& transformation, double minDistance = 0.0
        );

        std::vector<std::tuple<point_t, point_t>> pairs() const;

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
