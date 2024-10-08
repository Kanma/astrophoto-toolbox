/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/starmatcher.h>
#include <astrophoto-toolbox/data/fits.h>
#include <cstdlib>

using namespace astrophototoolbox;


star_list_t generateStars(unsigned int nb, unsigned int imageWidth = 3500, unsigned int imageHeight = 3000)
{
    std::srand(100000);

    star_list_t stars;

    for (unsigned int i = 0; i < nb; ++i)
    {
        star_t star;
        star.position.x = std::rand() / (RAND_MAX / imageWidth);
        star.position.y = std::rand() / (RAND_MAX / imageHeight);
        star.intensity = 0.1 + (double(std::rand()) / RAND_MAX) * 0.9;
        stars.push_back(star);
    }

    return stars;
}


star_list_t translateStars(const star_list_t& stars, int dx, int dy)
{
    star_list_t result;

    for (const auto& star : stars)
    {
        star_t star2;
        star2.position.x = star.position.x + dx;
        star2.position.y = star.position.y + dy;
        star2.intensity = star.intensity + (double(std::rand()) / RAND_MAX) * 0.02 - 0.01;
        result.push_back(star2);
    }

    return result;
}


star_list_t cropStars(
    const star_list_t& stars, unsigned int margin = 500, unsigned int imageWidth = 3500,
    unsigned int imageHeight = 3000
)
{
    star_list_t result;

    margin = margin / 2;

    for (const auto& star : stars)
    {
        if ((star.position.x >= margin) && (star.position.x < imageWidth - margin) &&
            (star.position.y >= margin) && (star.position.y < imageHeight - margin))
        {
            star_t star2;
            star2.position.x = star.position.x - margin;
            star2.position.y = star.position.y - margin;
            star2.intensity = star.intensity;
            result.push_back(star2);
        }
    }

    return result;
}


star_list_t generateHotPixels(unsigned int nb, unsigned int imageWidth = 3500, unsigned int imageHeight = 3000)
{
    star_list_t stars;

    for (unsigned int i = 0; i < nb; ++i)
    {
        star_t star;
        star.position.x = std::rand() / (RAND_MAX / imageWidth);
        star.position.y = std::rand() / (RAND_MAX / imageHeight);
        star.intensity = 1.0 + (double(std::rand()) / RAND_MAX) * 0.2;
        stars.push_back(star);
    }

    return stars;
}


star_list_t mergeStars(const star_list_t& stars1, const star_list_t& stars2)
{
    star_list_t stars = stars1;

    for (const auto& star : stars2)
        stars.push_back(star);

    return stars;
}


TEST_CASE("Matching registered stars", "[StarMatcher]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "stars/matching_stars1.fits"));
    size2d_t imageSize;
    star_list_t stars1 = input.readStars(0, &imageSize);
    input.close();

    REQUIRE(input.open(DATA_DIR "stars/matching_stars2.fits"));
    star_list_t stars2 = input.readStars();
    input.close();

    Transformation transformation;
    stacking::StarMatcher matcher;

    REQUIRE(matcher.computeTransformation(stars1, stars2, imageSize, transformation));

    double dx, dy;
    transformation.offsets(dx, dy);

    REQUIRE(dx == Approx(78.9042));
    REQUIRE(dy == Approx(-98.49931));

    double angle = transformation.angle(imageSize.width);
    REQUIRE(angle == Approx(0.0002597452));

    REQUIRE(matcher.computeTransformation(stars2, stars1, imageSize, transformation));

    transformation.offsets(dx, dy);

    REQUIRE(dx == Approx(-78.9177));
    REQUIRE(dy == Approx(98.5097));

    angle = transformation.angle(imageSize.width);
    REQUIRE(angle == Approx(-0.0002531776));
}


TEST_CASE("Matching synthetic stars", "[StarMatcher]")
{
    Transformation transformation;
    stacking::StarMatcher matcher;

    star_list_t stars1 = generateStars(100);
    star_list_t matching_stars1 = cropStars(stars1);

    int dxs[] = {-150, -20, 90, 210};
    int dys[] = {-200, 10, 60, 160};

    for (int i  = 0; i < 4; ++i)
    {
        star_list_t stars2 = translateStars(stars1, dxs[i], dys[i]);
        star_list_t matching_stars2 = cropStars(stars2);

        REQUIRE(matcher.computeTransformation(
            matching_stars1, matching_stars2, size2d_t(3000, 2500), transformation
        ));

        double dx, dy;
        transformation.offsets(dx, dy);

        REQUIRE(dxs[i] == Approx(dx));
        REQUIRE(dys[i] == Approx(dy));

        double angle = transformation.angle(3000);
        REQUIRE(angle == Approx(0.0).margin(0.001));
    }
}


TEST_CASE("Matching synthetic stars with hot pixels", "[StarMatcher]")
{
    Transformation transformation;
    stacking::StarMatcher matcher;

    star_list_t stars1 = generateStars(50);
    star_list_t hotPixels = generateHotPixels(50);

    star_list_t matching_stars1 = cropStars(mergeStars(stars1, hotPixels));

    int dxs[] = {-150, -20, 90, 210};
    int dys[] = {-200, 10, 60, 160};

    for (int i  = 0; i < 4; ++i)
    {
        star_list_t stars2 = translateStars(stars1, dxs[i], dys[i]);
        star_list_t matching_stars2 = cropStars(mergeStars(stars2, hotPixels));

        REQUIRE(matcher.computeTransformation(
            matching_stars1, matching_stars2, size2d_t(3000, 2500), transformation, 20.0
        ));

        double dx, dy;
        transformation.offsets(dx, dy);

        REQUIRE(dxs[i] == Approx(dx));
        REQUIRE(dys[i] == Approx(dy));

        double angle = transformation.angle(3000);
        REQUIRE(angle == Approx(0.0).margin(0.001));
    }
}
