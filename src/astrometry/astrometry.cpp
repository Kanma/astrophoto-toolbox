#include <astrophoto-toolbox/astrometry/astrometry.h>
#include <filesystem>

extern "C" {
    #include <astrometry/image2xy.h>
    #include <astrometry/matchobj.h>
    #include <astrometry/permutedsort.h>
    #include <astrometry/solver.h>
}

using namespace astrophototoolbox;


static anbool match_callback(MatchObj* mo, void* userdata)
{
    return TRUE;
}


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Astrometry::Astrometry()
{
}

//-----------------------------------------------------------------------------

Astrometry::~Astrometry()
{
    clearIndexes();
}


/************************************** METHODS ****************************************/

bool Astrometry::run(Bitmap* bitmap, double minWidth, double maxWidth)
{
    return detectStars(bitmap, true, true) &&
           solve(minWidth, maxWidth);
}

//-----------------------------------------------------------------------------

bool Astrometry::detectStars(Bitmap* bitmap, bool uniformize, bool cut)
{
    coordinates = Coordinates();
    pixelScale = 0.0;

    if (!bitmap)
        return false;

    FloatGrayBitmap* grayBitmap = dynamic_cast<FloatGrayBitmap*>(bitmap);
    if (!grayBitmap || (grayBitmap->range() != RANGE_BYTE) ||
        (grayBitmap->bytesPerRow() > grayBitmap->width() * grayBitmap->bytesPerPixel()))
    {
        grayBitmap = new FloatGrayBitmap(bitmap, RANGE_BYTE);
    }

    simplexy_t params = { 0 };
    simplexy_fill_in_defaults(&params);

    params.image = grayBitmap->data();
    params.nx = grayBitmap->width();
    params.ny = grayBitmap->height();

    stars.clear();

    int res = image2xy_run(&params, 2, 3);

    if (grayBitmap != bitmap)
        delete grayBitmap;

    if (res != 0)
        return false;

    std::vector<int> sortedIndices = sort(params, false);

    stars.resize(params.npeaks);

    star_t* dest = stars.data();
    for (int i = 0; i < params.npeaks; ++i)
    {
        int index = sortedIndices[i];
        dest->x = params.x[index];
        dest->y = params.y[index];
        dest->flux = params.flux[index];
        dest->background = params.background[index];
        ++dest;
    }

    detectionInfo.imageWidth = bitmap->width();
    detectionInfo.imageHeight = bitmap->height();
    detectionInfo.estimatedSourceVariance = params.sigma;
    detectionInfo.gaussianPsfWidth = params.dpsf;
    detectionInfo.significanceLimit = params.plim;
    detectionInfo.distanceLimit = params.dlim;
    detectionInfo.saddleDiffference = params.saddle;
    detectionInfo.maxNbPeaksPerObject = params.maxper;
    detectionInfo.maxNbPeaksTotal = params.maxnpeaks;
    detectionInfo.maxSize = params.maxsize;
    detectionInfo.slidingSkyWindowHalfSize = params.halfbox;

    if (uniformize)
        this->uniformize();

    if (cut)
        this->cut();

    return true;
}

//-----------------------------------------------------------------------------

bool Astrometry::uniformize(unsigned int nbBoxes)
{
    if (stars.empty())
        return true;

    // Determine the actual number of boxes
    float maxX = stars[0].x;
    float maxY = stars[0].y;
    float minX = stars[0].x;
    float minY = stars[0].y;

    for (const auto& star : stars)
    {
        maxX = std::max(maxX, star.x);
        maxY = std::max(maxY, star.y);
        minX = std::min(minX, star.x);
        minY = std::min(minY, star.y);
    }

    float width = maxX - minX;
    float height = maxY - minY;

    if ((width < 1e-6f) || (height < 1e-6f))
        return false;

    int nbX = int(std::max(1.0f, std::round(width / std::sqrtf(width * height / float(nbBoxes)))));
    int nbY = int(std::max(1.0f, std::round(float(nbBoxes) / float(nbX) - 1e-6f)));

    // Determine in which box each star belongs
    std::vector<std::vector<unsigned int>> bins(nbX * nbY);

    for (unsigned int i = 0; i < stars.size(); ++i)
    {
        const auto& star = stars[i];

        int x = std::clamp(std::floorf((star.x - minX) / width * nbX), 0.0f, nbX - 1.0f);
        int y = std::clamp(std::floorf((star.y - minY) / height * nbY), 0.0f, nbY - 1.0f);

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

void Astrometry::cut(unsigned int nb)
{
    if (stars.size() > nb)
        stars.resize(nb);
}

//-----------------------------------------------------------------------------

bool Astrometry::solve(double minWidth, double maxWidth)
{
    coordinates = Coordinates();
    pixelScale = 0.0;

    if (stars.empty() || (detectionInfo.imageWidth == 0) || (detectionInfo.imageHeight == 0) ||
        indexes.empty())
    {
        return false;
    }

    solver_t* solver = solver_new();

    solver->pixel_xscale = 0.0;

    solver->field_maxx = double(detectionInfo.imageWidth);
    solver->field_maxy = double(detectionInfo.imageHeight);

    solver->funits_lower = deg2arcsec(minWidth) / detectionInfo.imageWidth;
    solver->funits_upper = deg2arcsec(maxWidth) / detectionInfo.imageWidth;

    solver->logratio_toprint = log(1e6);
    solver->logratio_tokeep = log(1e9);
    solver->logratio_totune = log(1e6);

    solver->record_match_callback = match_callback;
    solver->userdata = nullptr;

    solver->distance_from_quad_bonus = TRUE;
    solver->verify_dedup = FALSE;

    solver->do_tweak = TRUE;
    solver->tweak_aborder = 2;
    solver->tweak_abporder = 2;

    solver->quadsize_min = 0.1 * std::min(detectionInfo.imageWidth, detectionInfo.imageHeight);


    std::vector<index_t*> indexes = filterIndexes(minWidth, maxWidth);

    for (index_t* index : indexes)
    {
        if (!index->codekd)
        {
            char* ifn = index->indexfn;
            char* iname = index->indexname;

            if (!index_load(index->indexfn, 0, index))
                continue;

            free(iname);
            free(ifn);
        }

        solver_add_index(solver, index);
    }


    starxy_t* fieldxy = starxy_new(stars.size(), false, false);

    for (int i = 0; i < fieldxy->N; ++i)
    {
        const auto& star = stars[i];

        fieldxy->x[i] = star.x;
        fieldxy->y[i] = star.y;
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

bool Astrometry::loadIndexes(const std::string& folder)
{
    // Find all index files in the folder
    std::vector<std::string> files;

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

void Astrometry::clearIndexes()
{
    for (index_t* index : indexes)
        index_free(index);

    indexes.clear();
}


/********************************** PRIVATE METHODS ************************************/

std::vector<int> Astrometry::sort(const simplexy_t& params, bool ascending)
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

    return result;
}

//-----------------------------------------------------------------------------

std::vector<index_t*> Astrometry::filterIndexes(float minWidth, float maxWidth)
{
    if ((detectionInfo.imageWidth == 0) || (detectionInfo.imageHeight == 0))
        return std::vector<index_t*>();

    double scaleMin = deg2arcsec(minWidth) / detectionInfo.imageWidth;
    double scaleMax = deg2arcsec(maxWidth) / detectionInfo.imageWidth;

    // range of quad sizes that could be found in the field, in arcsec.
    double quadsize_min = 0.1 * std::min(detectionInfo.imageWidth, detectionInfo.imageHeight);
    double fmax = hypot(detectionInfo.imageWidth, detectionInfo.imageHeight) * scaleMax;
    double fmin = quadsize_min * scaleMin;

    std::vector<index_t*> result;
    for (index_t* index : indexes)
    {
        if (index_overlaps_scale_range(index, fmin, fmax))
            result.push_back(index);
    }

    return result;
}
