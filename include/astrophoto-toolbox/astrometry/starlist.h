#pragma once

#include <vector>


namespace astrophototoolbox {

    struct star_t
    {
        float x;
        float y;
        float flux;
        float background;
    };

    struct star_list_t
    {
        std::vector<star_t> stars;
        float estimatedSourceVariance;
        float gaussianPsfWidth;
        float significanceLimit;
        float distanceLimit;
        float saddleDiffference;
        int maxNbPeaksPerObject;
        int maxNbPeaksTotal;
        int maxSize;
        int slidingSkyWindowHalfSize;
    };

}
