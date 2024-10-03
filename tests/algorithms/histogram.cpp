/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/algorithms/histogram.h>
#include <string.h>

using namespace astrophototoolbox;


TEST_CASE("Histogram of a bitmap computation", "[Histogram]")
{
    std::vector<uint16_t> data { 
        10, 20, 0, 5, 10, 1000, 1000, 5, 5, 5, 0, 0, 0, 5
    };

    UInt16GrayBitmap bitmap(20, 10);
    uint16_t* ptr = bitmap.data();
    memcpy(ptr, data.data(), data.size() * sizeof(uint16_t));

    histogram_t histogram;
    computeHistogram(&bitmap, histogram);

    REQUIRE(histogram.size() == 65536);

    for (size_t i = 0; i < histogram.size(); ++i)
    {
        if (i == 0)
            REQUIRE(histogram[i] == 190);
        else if (i == 5)
            REQUIRE(histogram[i] == 5);
        else if (i == 10)
            REQUIRE(histogram[i] == 2);
        else if (i == 20)
            REQUIRE(histogram[i] == 1);
        else if (i == 1000)
            REQUIRE(histogram[i] == 2);
        else  
            REQUIRE(histogram[i] == 0);
    }
}


TEST_CASE("Histogram of a vector of uint16s computation", "[Histogram]")
{
    std::vector<uint16_t> data { 
        10, 20, 0, 5, 10, 1000, 1000, 5, 5, 5, 0, 0, 0, 5
    };

    histogram_t histogram;
    computeHistogram(data, histogram, (uint16_t) 65535);

    REQUIRE(histogram.size() == 65536);

    for (size_t i = 0; i < histogram.size(); ++i)
    {
        if (i == 0)
            REQUIRE(histogram[i] == 4);
        else if (i == 5)
            REQUIRE(histogram[i] == 5);
        else if (i == 10)
            REQUIRE(histogram[i] == 2);
        else if (i == 20)
            REQUIRE(histogram[i] == 1);
        else if (i == 1000)
            REQUIRE(histogram[i] == 2);
        else  
            REQUIRE(histogram[i] == 0);
    }
}


TEST_CASE("Histogram of a vector of floats computation", "[Histogram]")
{
    std::vector<float> data { 
        10, 20, 0, 5, 10, 1000, 1000, 5, 5, 5, 0, 0, 0, 5
    };

    histogram_t histogram;
    computeHistogram(data, histogram, 1000.0f);

    REQUIRE(histogram.size() == 65536);

    for (size_t i = 0; i < histogram.size(); ++i)
    {
        if (i == 0)
            REQUIRE(histogram[i] == 4);
        else if (i == 327)
            REQUIRE(histogram[i] == 5);
        else if (i == 655)
            REQUIRE(histogram[i] == 2);
        else if (i == 1310)
            REQUIRE(histogram[i] == 1);
        else if (i == 65535)
            REQUIRE(histogram[i] == 2);
        else  
            REQUIRE(histogram[i] == 0);
    }
}
