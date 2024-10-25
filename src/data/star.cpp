/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/algorithms/math.h>


namespace astrophototoolbox {

double computeQuality(const star_list_t& stars)
{
    double quality = 0.0;
    for (const auto& star : stars)
        quality += star.quality;

    return quality;
}

//-----------------------------------------------------------------------------

double computeFWHM(const star_list_t& stars)
{
    std::vector<double> fwhm;
    for (const auto& star : stars)
        fwhm.push_back(star.meanRadius * (2.35 / 1.5));

    return computeAverage(fwhm);
}

}
