/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/algorithms/math.h>

using namespace astrophototoolbox;


TEST_CASE("Average computation", "[Math]")
{
    std::vector<uint16_t> data { 
        10, 20, 0, 5, 10, 20, 10, 5, 5, 5, 0, 0, 0, 5
    };

    double average = computeAverage(data);

    REQUIRE(average == Approx(6.78571));
}


TEST_CASE("Standard deviation computation", "[Math]")
{
    std::vector<uint16_t> data { 
        10, 20, 0, 5, 10, 20, 10, 5, 5, 5, 0, 0, 0, 5
    };

    double average = 0.0;
    double sigma = computeStandardDeviation(data, average);

    REQUIRE(sigma == Approx(6.438484));
    REQUIRE(average == Approx(6.78571));
}


TEST_CASE("Median computation", "[Math]")
{
    std::vector<uint16_t> data { 
        10, 20, 0, 5, 10, 20, 10, 5, 5, 5, 0, 0, 0, 5
    };

    SECTION("with maximum specified")
    {
        uint16_t median = computeMedian(data, (uint16_t) 65535);
        REQUIRE(median == 6);
    }

    SECTION("without maximum specified")
    {
        uint16_t median = computeMedian(data);
        REQUIRE(median == 5);
    }
}
