#include <astrophoto-toolbox/astrometry/astrometry.h>

extern "C" {
    #include <astrometry/image2xy.h>
}

using namespace astrophototoolbox;


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Astrometry::Astrometry()
{
}

//-----------------------------------------------------------------------------

Astrometry::~Astrometry()
{
}


/************************************** METHODS ****************************************/

bool Astrometry::detectStars(Bitmap* bitmap)
{
    if (!bitmap)
        return false;

    FloatGrayBitmap* grayBitmap = dynamic_cast<FloatGrayBitmap*>(bitmap);
    if (!grayBitmap || (grayBitmap->bytesPerRow() > grayBitmap->width() * grayBitmap->bytesPerPixel()))
    {
        grayBitmap = new FloatGrayBitmap(bitmap, RANGE_SOURCE);
    }

    simplexy_t params = { 0 };
    simplexy_fill_in_defaults(&params);

    params.image = grayBitmap->data();
    params.nx = grayBitmap->width();
    params.ny = grayBitmap->height();

    starList.stars.clear();

    int res = image2xy_run(&params, 2, 3);

    if (grayBitmap != bitmap)
        delete grayBitmap;

    if (res != 0)
        return false;

    starList.stars.resize(params.npeaks);

    star_t* dest = starList.stars.data();
    for (int i = 0; i < params.npeaks; ++i)
    {
        dest->x = params.x[i];
        dest->y = params.y[i];
        dest->flux = params.flux[i];
        dest->background = params.background[i];
        ++dest;
    }

    starList.estimatedSourceVariance = params.sigma;
    starList.gaussianPsfWidth = params.dpsf;
    starList.significanceLimit = params.plim;
    starList.distanceLimit = params.dlim;
    starList.saddleDiffference = params.saddle;
    starList.maxNbPeaksPerObject = params.maxper;
    starList.maxNbPeaksTotal = params.maxnpeaks;
    starList.maxSize = params.maxsize;
    starList.slidingSkyWindowHalfSize = params.halfbox;

    return true;
}
