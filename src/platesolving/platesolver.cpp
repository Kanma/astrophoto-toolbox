/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is essentially a reimplementation of parts of 'astrometry.net',
 * which is released under a BSD 3-Clause license,
 * Copyright (c) 2006-2015, Astrometry.net Developers.
*/

#include <astrophoto-toolbox/platesolving/platesolver.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <filesystem>
#include <cmath>
#include <iostream>

extern "C" {
    #include <astrometry/image2xy.h>
    #include <astrometry/matchobj.h>
    #include <astrometry/permutedsort.h>
    #include <astrometry/solver.h>
}

using namespace astrophototoolbox;
using namespace astrophototoolbox::platesolving;


/************************************* CALLBACKS ***************************************/

struct callback_data_t
{
    time_t limit = 10;
    bool* cancelled = nullptr;
};

//-----------------------------------------------------------------------------

static anbool match_callback(MatchObj* mo, void* userdata)
{
    return TRUE;
}

//-----------------------------------------------------------------------------

time_t timer_callback(void* userdata)
{
    callback_data_t* data = (callback_data_t*) userdata;

    if (*data->cancelled || (data->limit == 0))
        return 0;

    --data->limit;

    return 1;
}


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

PlateSolver::PlateSolver()
{
}

//-----------------------------------------------------------------------------

PlateSolver::~PlateSolver()
{
    clearIndexes();
}


/************************************** METHODS ****************************************/

bool PlateSolver::run(Bitmap* bitmap, double minWidth, double maxWidth, time_t limit)
{
    cancelled = false;

    if (!detectStars(bitmap, true, true) || cancelled)
        return false;

    return solve(minWidth, maxWidth, limit);
}

//-----------------------------------------------------------------------------

bool PlateSolver::run(
    const star_list_t& stars, const size2d_t& imageSize, double minWidth, double maxWidth,
    time_t limit
)
{
    cancelled = false;

    setStars(stars, imageSize);

    if (!uniformize())
        return false;

    cut();

    if (cancelled)
        return false;

    return solve(minWidth, maxWidth, limit);
}

//-----------------------------------------------------------------------------

bool PlateSolver::detectStars(Bitmap* bitmap, bool uniformize, bool cut)
{
    cancelled = false;

    coordinates = Coordinates();
    pixelScale = 0.0;

    if (!bitmap)
        return false;

    FloatGrayBitmap* grayBitmap = requiresFormat<FloatGrayBitmap>(bitmap, RANGE_BYTE);

    simplexy_t params = { 0 };
    simplexy_fill_in_defaults(&params);

    params.image = grayBitmap->data();
    params.nx = grayBitmap->width();
    params.ny = grayBitmap->height();

    stars.clear();

    int res = image2xy_run(&params, 2, 3);

    params.image = nullptr;

    if (grayBitmap != bitmap)
        delete grayBitmap;

    if ((res != 0) || cancelled)
        return false;

    std::vector<int> sortedIndices = sort(params, false);

    stars.resize(params.npeaks);

    star_t* dest = stars.data();
    for (int i = 0; i < params.npeaks; ++i)
    {
        int index = sortedIndices[i];
        dest->position.x = params.x[index];
        dest->position.y = params.y[index];
        dest->intensity = params.flux[index];
        ++dest;
    }

    imageSize.width = bitmap->width();
    imageSize.height = bitmap->height();

    if (uniformize)
        this->uniformize();

    if (cut)
        this->cut();

    simplexy_free_contents(&params);

    return true;
}

//-----------------------------------------------------------------------------

bool PlateSolver::uniformize(unsigned int nbBoxes)
{
    if (stars.empty())
        return true;

    // Determine the actual number of boxes
    point_t max = stars[0].position;
    point_t min = stars[0].position;

    for (const auto& star : stars)
    {
        max.x = std::max(max.x, star.position.x);
        max.y = std::max(max.y, star.position.y);
        min.x = std::min(min.x, star.position.x);
        min.y = std::min(min.y, star.position.y);
    }

    float width = max.x - min.x;
    float height = max.y - min.y;

    if ((width < 1e-6f) || (height < 1e-6f))
        return false;

    int nbX = int(std::max(1.0f, std::round(width / std::sqrt(width * height / float(nbBoxes)))));
    int nbY = int(std::max(1.0f, std::round(float(nbBoxes) / float(nbX) - 1e-6f)));

    // Determine in which box each star belongs
    std::vector<std::vector<unsigned int>> bins(nbX * nbY);

    for (unsigned int i = 0; i < stars.size(); ++i)
    {
        const auto& star = stars[i];

        int x = std::clamp((float) std::floor((star.position.x - min.x) / width * nbX), 0.0f, nbX - 1.0f);
        int y = std::clamp((float) std::floor((star.position.y - min.y) / height * nbY), 0.0f, nbY - 1.0f);

        unsigned int index = y * nbX + x;
        bins[index].push_back(i);
    }

    // Sort the stars by "first star in each box, second star in each box, ..."
    size_t maxlen = 0;
    for (const auto& bin : bins)
        maxlen = std::max<size_t>(maxlen, bin.size());

    std::vector<unsigned int> indices;
    for (size_t i = 0; i < maxlen; ++i)
    {
        std::vector<unsigned int> selection;
        for (const auto& bin : bins)
        {
            if (i >= bin.size())
                continue;

            selection.push_back(bin[i]);
        }

        std::sort(selection.begin(), selection.end());
        indices.insert(indices.end(), selection.begin(), selection.end());
    }

    std::vector<star_t> uniformizedStars;
    for (auto i : indices)
        uniformizedStars.push_back(stars[i]);

    stars = uniformizedStars;

    return true;
}

//-----------------------------------------------------------------------------

void PlateSolver::cut(unsigned int nb)
{
    if (stars.size() > nb)
        stars.resize(nb);
}

//-----------------------------------------------------------------------------

bool PlateSolver::solve(double minWidth, double maxWidth, time_t limit)
{
    cancelled = false;

    coordinates = Coordinates();
    pixelScale = 0.0;

    if (stars.empty() || (imageSize.width == 0) || (imageSize.height == 0) ||
        indexes.empty())
    {
        return false;
    }

    solver_t* solver = solver_new();

    solver->pixel_xscale = 0.0;

    solver->field_maxx = double(imageSize.width);
    solver->field_maxy = double(imageSize.height);

    solver->funits_lower = deg2arcsec(minWidth) / imageSize.width;
    solver->funits_upper = deg2arcsec(maxWidth) / imageSize.width;

    solver->logratio_toprint = log(1e6);
    solver->logratio_tokeep = log(1e9);
    solver->logratio_totune = log(1e6);

    callback_data_t callback_data;
    callback_data.limit = limit;
    callback_data.cancelled = &cancelled;

    solver->record_match_callback = match_callback;
    solver->timer_callback = timer_callback;
    solver->userdata = &callback_data;

    solver->distance_from_quad_bonus = TRUE;
    solver->verify_dedup = FALSE;

    solver->do_tweak = TRUE;
    solver->tweak_aborder = 2;
    solver->tweak_abporder = 2;

    solver->quadsize_min = 0.1 * std::min(imageSize.width, imageSize.height);


    std::vector<index_t*> indexes = filterIndexes(minWidth, maxWidth);

    for (index_t* index : indexes)
    {
        if (!index->codekd)
        {
            if (index_reload(index) != 0)
                continue;
        }

        solver_add_index(solver, index);

        if (cancelled)
            break;
    }

    if (cancelled)
    {
        solver_free(solver);
        return false;
    }

    starxy_t* fieldxy = starxy_new(stars.size(), false, false);

    for (int i = 0; i < fieldxy->N; ++i)
    {
        const auto& star = stars[i];

        fieldxy->x[i] = star.position.x;
        fieldxy->y[i] = star.position.y;
    }

    solver_set_field(solver, fieldxy);

    solver_run(solver);

    bool success = solver_did_solve(solver);
    if (success)
    {
        double ra,dec;
        xyzarr2radecdeg(solver->best_match.center, &ra, &dec);

        coordinates.set(ra, dec);
        pixelScale = solver->best_match.scale;
    }

    solver_free(solver);

    return success;
}

//-----------------------------------------------------------------------------

bool PlateSolver::loadIndexes(const std::string& folder)
{
    // Find all index files in the folder
    std::vector<std::string> files;

    if (!std::filesystem::is_directory(folder))
        return false;

    for (const auto& dirEntry : std::filesystem::directory_iterator(folder))
    {
        if (dirEntry.is_directory())
            continue;

        if (dirEntry.path().extension() == ".fits")
        {
            if (index_is_file_index(dirEntry.path().c_str()))
                files.push_back(dirEntry.path().c_str());
        }
    }

    if (files.empty())
        return false;

    // Sort them
    std::sort(files.begin(), files.end());

    // Load them in reverse
    for (auto iter = files.rbegin(); iter != files.rend(); ++iter)
    {
        index_t* index = index_load(iter->c_str(), INDEX_ONLY_LOAD_METADATA, nullptr);
        if (index)
            indexes.push_back(index);
    }

    return true;
}

//-----------------------------------------------------------------------------

void PlateSolver::clearIndexes()
{
    for (index_t* index : indexes)
        index_free(index);

    indexes.clear();
}

//-----------------------------------------------------------------------------

void PlateSolver::cancel()
{
    cancelled = true;
}


/********************************** PRIVATE METHODS ************************************/

std::vector<int> PlateSolver::sort(const simplexy_t& params, bool ascending)
{
    if (params.npeaks <= 1)
        return std::vector<int>();

    int (*compare)(const void*, const void*);

    if (ascending)
        compare = compare_floats_asc;
    else
        compare = compare_floats_desc;

    // Set background = flux + background (ie, non-background-subtracted flux)
    float* background = new float[params.npeaks];
    for (int i = 0; i < params.npeaks; ++i)
        background[i] = params.background[i] + params.flux[i];

    // Sort by flux
    int* perm1 = permuted_sort(params.flux, sizeof(float), compare, NULL, params.npeaks);

    // Sort by non-background-subtracted flux
    int* perm2 = permuted_sort(background, sizeof(float), compare, NULL, params.npeaks);

    // Determine the final order
    bool* used = new bool[params.npeaks];
    memset(used, 0, params.npeaks * sizeof(bool));

    std::vector<int> result(params.npeaks);
    int k = 0;
    for (int i = 0; i < params.npeaks; ++i)
    {
        int inds[] = { perm1[i], perm2[i] };
        for (int j = 0; j < 2; ++j)
        {
            int index = inds[j];
            if (used[index])
                continue;

            used[index] = true;
            result[k] = index;
            ++k;
        }
    }

    delete[] background;
    delete[] used;
    free(perm1);
    free(perm2);

    return result;
}

//-----------------------------------------------------------------------------

std::vector<index_t*> PlateSolver::filterIndexes(float minWidth, float maxWidth)
{
    if ((imageSize.width == 0) || (imageSize.height == 0))
        return std::vector<index_t*>();

    double scaleMin = deg2arcsec(minWidth) / imageSize.width;
    double scaleMax = deg2arcsec(maxWidth) / imageSize.width;

    // range of quad sizes that could be found in the field, in arcsec.
    double quadsize_min = 0.1 * std::min(imageSize.width, imageSize.height);
    double fmax = hypot(imageSize.width, imageSize.height) * scaleMax;
    double fmin = quadsize_min * scaleMin;

    std::vector<index_t*> result;
    for (index_t* index : indexes)
    {
        if (index_overlaps_scale_range(index, fmin, fmax))
            result.push_back(index);
    }

    return result;
}
