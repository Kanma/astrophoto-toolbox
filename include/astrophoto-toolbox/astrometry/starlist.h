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

    typedef std::vector<star_t> star_list_t;

    struct star_detection_info_t
    {
        unsigned int imageWidth;
        unsigned int imageHeight;
        float estimatedSourceVariance;
        float gaussianPsfWidth;
        float significanceLimit;
        float distanceLimit;
        float saddleDifference;
        int maxNbPeaksPerObject;
        int maxNbPeaksTotal;
        int maxSize;
        int slidingSkyWindowHalfSize;
    };

}
