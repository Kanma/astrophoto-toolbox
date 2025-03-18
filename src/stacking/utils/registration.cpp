/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is essentially a reimplementation of parts of 'DeepSkyStacker',
 * which is released under a BSD 3-Clause license,
 * Copyright (c) 2006-2019, LucCoiffier 
 * Copyright (c) 2018-2023, 
 *      David C. Partridge, Tony Cook, Mat Draper, 
 *      Simon C. Smith, Vitali Pelenjow, Michal Schulz, Martin Toeltsch
*/

#include <astrophoto-toolbox/stacking/utils/registration.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <array>
#include <algorithm>

using namespace astrophototoolbox;
using namespace stacking;
using namespace utils;


struct pixel_direction_t
{
    pixel_direction_t(int8_t x, int8_t y)
        : xDir{x}, yDir{y}
    {
    }

    int8_t xDir = 0;
    int8_t yDir = 0;
    int8_t radius = 0;
    double intensity = 0.0;
    int8_t nbBrighterPixels = 0;
    int8_t ok = 2;
};


struct searchEntry_t {
    int luminancyThreshold;
    int nbStars;

    friend auto operator<(const searchEntry_t& lhs, const searchEntry_t& rhs)
    {
        return lhs.luminancyThreshold < rhs.luminancyThreshold;
    }
};

//-----------------------------------------------------------------------------

const star_list_t Registration::registerBitmap(Bitmap* bitmap, int luminancyThreshold)
{
    DoubleGrayBitmap* luminance = computeLuminanceBitmap(bitmap);
    double median = computeMedian(luminance);

    this->luminancyThreshold = std::min(std::max(luminancyThreshold, -1), 100);

    star_list_t stars;

    if (this->luminancyThreshold >= 0)
        registerBitmapWithFixedThreshold(luminance, median, stars);
    else
        registerBitmapAndSearchThreshold(luminance, median, stars);

	std::sort(stars.begin(), stars.end(), star_t::compareIntensity);

    delete luminance;

    return stars;
}

//-----------------------------------------------------------------------------

void Registration::registerBitmapWithFixedThreshold(
    DoubleGrayBitmap* luminance, double median, star_list_t& stars
)
{
    const int rectSize = STARMAXSIZE * 5;
    const int stepSize = rectSize / 2;
    const int separation = 3;
    const int width = luminance->width() - 2 * STARMAXSIZE;
    const int height = luminance->height() - 2 * STARMAXSIZE;
    const int nbRectsX = (width - 1) / stepSize + 1;
    const int nbRectsY = (height - 1) / stepSize + 1;

    const int rightColumn = luminance->width() - STARMAXSIZE;
    const int bottomRow = luminance->height() - STARMAXSIZE;

    star_set_t foundStars;
    int nbStars = 0;

    double minLuminancy = double(luminancyThreshold) / 100.0;

    for (int row = 0; row < nbRectsY; ++row)
    {
        const int top = STARMAXSIZE + row * stepSize;
        const int bottom = std::min(bottomRow, top + rectSize);

        for (int col = 0; (col < nbRectsX) && (nbStars <= 100); ++col)
        {
            nbStars += registerRect(
                luminance,
                rect_t(
                    STARMAXSIZE + col * stepSize,
                    top,
                    std::min(rightColumn, STARMAXSIZE + col * stepSize + rectSize),
                    bottom),
                median,
                minLuminancy,
                foundStars
            );
        }
    }

    stars.assign(foundStars.cbegin(), foundStars.cend());
}

//-----------------------------------------------------------------------------

void Registration::registerBitmapAndSearchThreshold(
    DoubleGrayBitmap* luminance, double median, star_list_t& stars
)
{
    const int rectSize = STARMAXSIZE * 5;
    const int stepSize = rectSize / 2;
    const int separation = 3;
    const int width = luminance->width() - 2 * STARMAXSIZE;
    const int height = luminance->height() - 2 * STARMAXSIZE;
    const int nbRectsX = (width - 1) / stepSize + 1;
    const int nbRectsY = (height - 1) / stepSize + 1;

    const int rightColumn = luminance->width() - STARMAXSIZE;
    const int bottomRow = luminance->height() - STARMAXSIZE;

    std::set<searchEntry_t> searchGrid;
    bool found = false;

    luminancyThreshold = 10;

    while (true)
    {
        star_set_t foundStars;
        int nbStars = 0;

        double minLuminancy = double(luminancyThreshold) / 100.0;

        for (int row = 0; (row < nbRectsY) && (nbStars <= 100); ++row)
        {
            const int top = STARMAXSIZE + row * stepSize;
            const int bottom = std::min(bottomRow, top + rectSize);

            for (int col = 0; (col < nbRectsX) && (nbStars <= 100); ++col)
            {
                nbStars += registerRect(
                    luminance,
                    rect_t(
                        STARMAXSIZE + col * stepSize,
                        top,
                        std::min(rightColumn, STARMAXSIZE + col * stepSize + rectSize),
                        bottom),
                    median,
                    minLuminancy,
                    foundStars
                );
            }
        }

        if (found)
        {
            stars.assign(foundStars.cbegin(), foundStars.cend());
            break;
        }

        searchEntry_t searchentry{ luminancyThreshold, nbStars };

        if (nbStars > 100)
        {
            auto it = searchGrid.upper_bound(searchentry);
            if (it == searchGrid.end())
                luminancyThreshold = std::min(luminancyThreshold * 2, int((1.0 - median) * 0.9 * 100.0));
            else
                luminancyThreshold = (luminancyThreshold + it->luminancyThreshold + 1) / 2;
        }
        else if (nbStars < 40)
        {
            std::set<searchEntry_t>::iterator prev = searchGrid.end();
            for (auto it = searchGrid.begin(); (it != searchGrid.end()) && (it->luminancyThreshold < luminancyThreshold); ++it)
                prev = it;

            if (prev == searchGrid.end())
                luminancyThreshold = std::max(luminancyThreshold / 2, 0);
            else
                luminancyThreshold = (luminancyThreshold + prev->luminancyThreshold + 1) / 2;
        }

        searchGrid.insert(searchentry);

        if (searchentry.luminancyThreshold == luminancyThreshold)
        {
            if (nbStars < 20)
            {
                --luminancyThreshold;
                found = true;
            }
            else
            {
                stars.assign(foundStars.cbegin(), foundStars.cend());
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

int Registration::registerRect(
    const DoubleGrayBitmap* bitmap, const rect_t &rect, double background,
    double minLuminancy, star_set_t& stars
) const
{
    double maxIntensity = std::numeric_limits<double>::min();
    size_t nbStars = 0;

    // Copy the content of the rectangle in a local buffer
    const int width = rect.width();
    std::vector<double> values(width * rect.height());
    for (int y = rect.top, index = 0; y < rect.bottom; ++y)
    {
        const double* ptr = bitmap->data(rect.left, y);

        for (int x = 0; x < width; ++x, ++index)
        {
            values[index] = *ptr;
            maxIntensity = std::max(maxIntensity, *ptr);
            ++ptr;
        }
    }

    const auto getValue = [&values, rect, width](const int x, const int y) -> double
    {
        return values[(y - rect.top) * width + x - rect.left];
    };

    // Luminance-based check to determine if bright enough stars are in the rectangle
    const double intensityThreshold = minLuminancy + background;
    if (maxIntensity < intensityThreshold)
        return 0;

    // Find how many wanabee stars are existing above 90% maximum luminance
    for (int deltaRadius = 0; deltaRadius < 4; ++deltaRadius)
    {
        for (int y = rect.top; y < rect.bottom; ++y)
        {
            for (int x = rect.left; x < rect.right; ++x)
            {
                const double intensity = getValue(x, y);

                if (x == 265)
                    x = 265;

                if (intensity < intensityThreshold)
                    continue;

                // Check that this pixel is not already used in a wanabee star
                bool isNew = true;
                for (star_set_t::const_iterator it = stars.lower_bound(star_t(x - STARMAXSIZE, 0));
                     it != stars.cend() && isNew; ++it)
                {
                    if (it->contains(x, y))
                        isNew = false;
                    else if (it->position.x > x + STARMAXSIZE)
                        break;
                }

                if (!isNew)
                    continue;

                // Search around the point until intensity is divided by 2
                std::array<pixel_direction_t, 8> directions{{{0, -1}, {1, 0}, {0, 1}, {-1, 0}, {1, -1}, {1, 1}, {-1, 1}, {-1, -1}}};

                bool brighterPixel = false;
                bool allOk = true;
                int maxRadius = 0;

                for (int testedRadius = 1; (testedRadius < STARMAXSIZE) && allOk && !brighterPixel; ++testedRadius)
                {
                    for (auto &pixel : directions)
                        pixel.intensity = *bitmap->data(x + pixel.xDir * testedRadius, y + pixel.yDir * testedRadius);

                    allOk = false;
                    for (auto &pixel : directions)
                    {
                        if (pixel.ok)
                        {
                            if (pixel.intensity - background < 0.25 * (intensity - background))
                            {
                                pixel.radius = testedRadius;
                                --pixel.ok;
                                maxRadius = std::max(maxRadius, testedRadius);
                            }
                            else if (pixel.intensity > 1.05 * intensity)
                            {
                                brighterPixel = true;
                            }
                            else if (pixel.intensity > intensity)
                            {
                                ++pixel.nbBrighterPixels;
                            }
                        }

                        if (pixel.ok)
                            allOk = true;

                        if (pixel.nbBrighterPixels > 2)
                            brighterPixel = true;

                        if (brighterPixel)
                            break;
                    }
                }

                if (allOk || brighterPixel || (maxRadius <= 2))
                    continue;

                // Check the roundness of the wanabee star
                // Radiuses should be within deltaRadius pixels of each others
                bool wanabeeStarOk = true;
                double meanRadius1 = 0.0;
                double meanRadius2 = 0.0;

                for (size_t k1 = 0; (k1 < 4) && wanabeeStarOk; ++k1)
                {
                    for (size_t k2 = 0; (k2 < 4) && wanabeeStarOk; ++k2)
                    {
                        if ((k1 != k2) && std::abs(directions[k2].radius - directions[k1].radius) > deltaRadius)
                            wanabeeStarOk = false;
                    }
                }
                for (size_t k1 = 4; (k1 < 8) && wanabeeStarOk; ++k1)
                {
                    for (size_t k2 = 4; (k2 < 8) && wanabeeStarOk; ++k2)
                    {
                        if ((k1 != k2) && std::abs(directions[k2].radius - directions[k1].radius) > deltaRadius)
                            wanabeeStarOk = false;
                    }
                }

                if (!wanabeeStarOk)
                    continue;

                // Compute the radiuses of the wannabe star
                for (size_t k1 = 0; k1 < 4; ++k1)
                    meanRadius1 += directions[k1].radius;

                meanRadius1 /= 4.0;

                for (size_t k1 = 4; k1 < 8; ++k1)
                    meanRadius2 += directions[k1].radius;

                meanRadius2 /= 4.0;
                meanRadius2 *= sqrt(2.0);

                int leftRadius = 0;
                int rightRadius = 0;
                int topRadius = 0;
                int bottomRadius = 0;

                for (const auto &pixel : directions)
                {
                    if (pixel.xDir < 0)
                        leftRadius = std::max(leftRadius, static_cast<int>(pixel.radius));
                    else if (pixel.xDir > 0)
                        rightRadius = std::max(rightRadius, static_cast<int>(pixel.radius));

                    if (pixel.yDir < 0)
                        topRadius = std::max(topRadius, static_cast<int>(pixel.radius));
                    else if (pixel.yDir > 0)
                        bottomRadius = std::max(bottomRadius, static_cast<int>(pixel.radius));
                }

                // Add the star to the set
                star_t star(x, y);
                star.intensity = intensity;
                star.meanRadius = (meanRadius1 + meanRadius2) / 2.0;

                constexpr double radiusFactor = 2.35 / 1.5;

                // Compute the real position
                if (computeStarCenter(bitmap, star.position, star.meanRadius, background))
                {
                    // Check last overlap condition
                    for (star_set_t::const_iterator it = stars.lower_bound(star_t(star.position.x - star.meanRadius * radiusFactor - STARMAXSIZE, 0));
                         it != stars.cend() && wanabeeStarOk; ++it)
                    {
                        if (star.position.distance(it->position) < (star.meanRadius + it->meanRadius) * radiusFactor)
                            wanabeeStarOk = false;
                        else if (it->position.x > star.position.x + star.meanRadius * radiusFactor + STARMAXSIZE)
                            break;
                    }

                    if (wanabeeStarOk)
                    {
                        star.quality = (10 - deltaRadius) + intensity - star.meanRadius;
                        stars.insert(std::move(star));
                        ++nbStars;
                    }
                }
            }
        }
    }

    return nbStars;
}

//-----------------------------------------------------------------------------

bool Registration::computeStarCenter(
    const DoubleGrayBitmap *bitmap, point_t& position, double& radius, double background
) const
{
    double sumX = 0.0;
    double sumY = 0.0;
    double nbValuesX = 0.0;
    double nbValuesY = 0.0;
    double averageX = 0.0;
    double averageY = 0.0;

    // Compute the position
    int nbRows = 0;
    for (int j = position.y - radius; j <= position.y + radius; ++j)
    {
        sumX = 0.0;
        nbValuesX = 0.0;
        for (int i = position.x - radius; i <= position.x + radius; ++i)
        {
            double value = *bitmap->data(i, j);
            sumX += value * i;
            nbValuesX += value;
        }

        if (nbValuesX > 0.0)
        {
            ++nbRows;
            averageX += sumX / nbValuesX;
        }
    }
    averageX /= (double) nbRows;

    int nbColumns = 0;
    for (int j = position.x - radius; j <= position.x + radius; ++j)
    {
        sumY = 0.0;
        nbValuesY = 0.0;
        for (int i = position.y - radius; i <= position.y + radius; ++i)
        {
            double value = *bitmap->data(j, i);
            sumY += value * i;
            nbValuesY += value;
        }

        if (nbValuesY > 0.0)
        {
            ++nbColumns;
            averageY += sumY / nbValuesY;
        };
    };
    averageY /= (double) nbColumns;

    position.x = averageX;
    position.y = averageY;

    // Compute the radius
    double squareSumX = 0.0;
    double squareSumY = 0.0;
    double stdDevX = 0.0;
    double stdDevY = 0.0;

    sumX = 0.0;
    nbValuesX = 0.0;
    for (int i = position.x - radius; i <= position.x + radius; ++i)
    {
        double value = *bitmap->data(i, position.y);
        value = std::max(0.0, value - background);
        sumX += value * i;
        squareSumX += pow(i - position.x, 2) * value;
        nbValuesX += value;
    };
    stdDevX = sqrt(squareSumX / nbValuesX);

    sumY = 0.0;
    nbValuesY = 0.0;
    for (int i = position.y - radius; i <= position.y + radius; ++i)
    {
        double value = *bitmap->data(position.x, i);
        value = std::max(0.0, value - background);
        sumY += value * i;
        squareSumY += pow(i - position.y, 2) * value;
        nbValuesY += value;
    };
    stdDevY = sqrt(squareSumY / nbValuesY);

    // The radius is the average of the standard deviations
    radius = (stdDevX + stdDevY) / 2.0 * 1.5;

    return fabs(stdDevX - stdDevY) < ROUNDNESS_TOLERANCE;
}
