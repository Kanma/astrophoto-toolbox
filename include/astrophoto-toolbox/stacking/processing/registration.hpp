/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/stacking/processing/registration.h>
#include <astrophoto-toolbox/images/io.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;
using namespace stacking;
using namespace processing;


template<class BITMAP>
void RegistrationProcessor<BITMAP>::setParameters(const star_list_t& stars, int luminancyThreshold)
{
    referenceStars = stars;
    this->luminancyThreshold = luminancyThreshold;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
star_list_t RegistrationProcessor<BITMAP>::processReference(
    const std::filesystem::path& lightFrame, int luminancyThreshold,
    const std::filesystem::path& destination
)
{
    referenceStars.clear();

    Bitmap* bitmap = io::load(lightFrame);
    if (!bitmap)
        return star_list_t();

    return processReference(std::make_shared<BITMAP>(bitmap), luminancyThreshold, destination);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
star_list_t RegistrationProcessor<BITMAP>::processReference(
    const std::shared_ptr<BITMAP>& lightFrame, int luminancyThreshold,
    const std::filesystem::path& destination
)
{
    referenceStars = registration.registerBitmap(lightFrame.get(), luminancyThreshold);
    this->luminancyThreshold = registration.getLuminancyThreshold();

    if (!destination.empty())
    {
        FITS fits;

        if (std::filesystem::exists(destination))
        {
            if (!FITS::isFITS(destination))
                return star_list_t();

            if (!fits.open(destination, false))
                return star_list_t();
        }
        else
        {
            if (!fits.create(destination))
                return star_list_t();
        }

        if (!fits.write(referenceStars, size2d_t(lightFrame->width(), lightFrame->height()),
                        &this->luminancyThreshold, "STARS", true))
        {
            return star_list_t();
        }

        if (!fits.write("REGISTERED", true))
            return star_list_t();
    }

    return referenceStars;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
std::tuple<star_list_t, Transformation> RegistrationProcessor<BITMAP>::process(
    const std::filesystem::path& lightFrame, const std::filesystem::path& destination
)
{
    Bitmap* bitmap = io::load(lightFrame);
    if (!bitmap)
        return std::make_tuple(star_list_t(), Transformation());

    return process(std::make_shared<BITMAP>(bitmap), destination);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
std::tuple<star_list_t, Transformation> RegistrationProcessor<BITMAP>::process(
    const std::shared_ptr<BITMAP>& lightFrame, const std::filesystem::path& destination
)
{
    star_list_t stars = registration.registerBitmap(lightFrame.get(), luminancyThreshold);

    Transformation transformation;

    bool valid = matcher.computeTransformation(
        stars, referenceStars, size2d_t(lightFrame->width(), lightFrame->height()),
        transformation
    );

    if (!destination.empty())
    {
        FITS fits;

        if (std::filesystem::exists(destination))
        {
            if (!FITS::isFITS(destination))
                return std::make_tuple(star_list_t(), Transformation());

            if (!fits.open(destination, false))
                return std::make_tuple(star_list_t(), Transformation());
        }
        else
        {
            if (!fits.create(destination))
                return std::make_tuple(star_list_t(), Transformation());
        }

        if (!fits.write(stars, size2d_t(lightFrame->width(), lightFrame->height()), &luminancyThreshold, "STARS", true))
            return std::make_tuple(star_list_t(), Transformation());

        if (valid)
        {
            if (!fits.write(transformation, "TRANSFORMS", true))
                return std::make_tuple(star_list_t(), Transformation());
        }

        if (!fits.write("REGISTERED", valid))
            return std::make_tuple(star_list_t(), Transformation());
    }

    if (!valid)
        return std::make_tuple(star_list_t(), Transformation());

    return std::make_tuple(stars, transformation);
}
