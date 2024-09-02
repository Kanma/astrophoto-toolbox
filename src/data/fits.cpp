#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

FITS::FITS()
{
}

//-----------------------------------------------------------------------------

FITS::~FITS()
{
    close();
}


/************************************** METHODS ****************************************/

bool FITS::open(const std::string& filename, bool readOnly)
{
    int status = 0;

    fits_open_file(&_file, filename.c_str(), readOnly ? READONLY : READWRITE, &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

bool FITS::create(const std::string& filename)
{
    int status = 0;

    fits_create_file(&_file, filename.c_str(), &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

void FITS::close()
{
    if (_file)
    {
        int status = 0;
        fits_close_file(_file, &status);
        _file = nullptr;
    }
}

//-----------------------------------------------------------------------------

int FITS::nbHDUs() const
{
    int status = 0;
    int nb;

    fits_get_num_hdus(_file, &nb, &status);

    return (status == 0 ? nb : 0);
}

//-----------------------------------------------------------------------------

int FITS::nbImages() const
{
    int status = 0;
    int nb = nbHDUs();
    int type;
    int nbImages = 0;

    for (int i = 1; i <= nb; ++i)
    {
        fits_movabs_hdu(_file, i, &type, &status);
        if ((status == 0) && (type == IMAGE_HDU))
            ++nbImages;
    }

    return nbImages;
}

//-----------------------------------------------------------------------------

bool FITS::write(Bitmap* bitmap, const std::string& name)
{
    assert(bitmap);

    int status = 0;

    int bitpix = bitmap->channelSize() * 8  * (bitmap->isFloatingPoint() ? -1 : 1);
    int naxis = 2 + (bitmap->channels() == 3 ? 1 : 0);
    long naxes[3] = {
        bitmap->width(),
        bitmap->height(),
        bitmap->channels()
    };

    fits_create_img(_file, bitpix, naxis, naxes, &status);

    if (!name.empty())
        fits_write_key(_file, TSTRING, "EXTNAME", (void*) name.c_str(), "", &status);
    
    fits_write_date(_file,  &status);

    int datatype = TBYTE;
    if (bitmap->isFloatingPoint())
    {
        datatype = (bitmap->channelSize() == 4 ? TFLOAT : TDOUBLE);
    }
    else
    {
        datatype = (bitmap->channelSize() == 1
                    ? TBYTE
                    : bitmap->channelSize() == 2
                      ? TUSHORT
                      : bitmap->channelSize() == 4
                        ? TUINT
                        : TULONG
        );
    }

    long fpixel[3] = { 1, 1, 1 };
    uint16_t v16;
    uint32_t v32;
    uint64_t v64;
    float f32;
    double f64;
    int64_t bzero = (datatype == TUSHORT
                     ? 0x8000
                     : datatype == TUINT
                       ? 0x80000000
                       : datatype == TULONG
                         ? 0x8000000000000000
                         : 0
    );

    for (fpixel[1] = 1; fpixel[1] <= bitmap->height(); ++fpixel[1])
    {
        uint8_t* ptr = bitmap->ptr(fpixel[1] - 1);

        for (fpixel[0] = 1; fpixel[0] <= bitmap->width(); ++fpixel[0])
        {
            for (fpixel[2] = 1; fpixel[2] <= bitmap->channels(); ++fpixel[2])
            {
                if ((datatype == TBYTE) || (datatype == TFLOAT) || (datatype == TDOUBLE))
                {
                    fits_write_pix(_file, datatype, fpixel, 1, ptr, &status);
                }
                else if (datatype == TUSHORT)
                {
                    v16 = *((uint16_t*) ptr) - bzero;
                    fits_write_pix(_file, TSHORT, fpixel, 1, &v16, &status);
                }
                else if (datatype == TUINT)
                {
                    v32 = *((uint32_t*) ptr) - bzero;
                    fits_write_pix(_file, TINT, fpixel, 1, &v32, &status);
                }
                else if (datatype == TULONG)
                {
                    v64 = *((uint64_t*) ptr) - bzero;
                    fits_write_pix(_file, TLONG, fpixel, 1, &v64, &status);
                }

                ptr += bitmap->channelSize();
            }
        }
    }

    if (bzero != 0)
        fits_write_key(_file, TUINT, "BZERO", &bzero, "zero point in scaling equation", &status);

    bitmap_info_t info = bitmap->info();
    fits_write_key(_file, TUINT, "ISO", &info.isoSpeed, "ISO speed", &status);
    fits_write_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, "Shutter speed", &status);
    fits_write_key(_file, TFLOAT, "APERTURE", &info.aperture, "Aperture", &status);
    fits_write_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, "Focal length", &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

bool FITS::write(const star_list_t& starList, const std::string& name, bool overwrite)
{
    assert(!starList.stars.empty());

    int status = 0;
    bool tableExisting = false;

    // Does the table already exist?
    if (!name.empty())
    {
        fits_movnam_hdu(_file, IMAGE_HDU, (char*) name.c_str(), 0, &status);
        if (status == 0)
        {
            if (!overwrite)
                return false;

            long nrows;
            fits_get_num_rows(_file, &nrows, &status);
            fits_delete_rows(_file, 1, nrows, &status);
            fits_insert_rows(_file, 1, starList.stars.size(), &status);

            if (status != 0)
                return false;

            tableExisting = true;
        }

        status = 0;
    }

    // Creates the table if necessary
    if (!tableExisting)
    {
        const char* ttype[] = {"X", "Y", "FLUX", "BACKGROUND"};
        const char* tform[] = {"E", "E", "E", "E"};
        const char* tunit[] = {"pix", "pix", "unknown", "unknown"};

        fits_create_tbl(
            _file, BINARY_TBL, starList.stars.size(), 4, (char**) ttype, (char**) tform, (char**) tunit,
            (name.empty() ? nullptr : name.c_str()), &status
        );
    }

    // Save the stars in the table
    fits_write_col(
        _file, TFLOAT, 1, 1, 1, starList.stars.size(),
        (uint8_t*) starList.stars.data() + offsetof(star_t, x), &status
    );
    fits_write_col(
        _file, TFLOAT, 2, 1, 1, starList.stars.size(),
        (uint8_t*) starList.stars.data() + offsetof(star_t, y), &status
    );
    fits_write_col(
        _file, TFLOAT, 3, 1, 1, starList.stars.size(),
        (uint8_t*) starList.stars.data() + offsetof(star_t, flux), &status
    );
    fits_write_col(
        _file, TFLOAT, 4, 1, 1, starList.stars.size(),
        (uint8_t*) starList.stars.data() + offsetof(star_t, background), &status
    );

    // Save the other data
    fits_write_key(_file, TFLOAT, "ESTSIGMA", (void*) &starList.estimatedSourceVariance, "Estimated source image variance", &status);
    fits_write_key(_file, TFLOAT, "DPSF", (void*) &starList.gaussianPsfWidth, "Assumed gaussian psf width", &status);
    fits_write_key(_file, TFLOAT, "PLIM", (void*) &starList.significanceLimit, "Significance to keep", &status);
    fits_write_key(_file, TFLOAT, "DLIM", (void*) &starList.distanceLimit, "Closest two peaks can be", &status);
    fits_write_key(_file, TFLOAT, "SADDLE", (void*) &starList.saddleDiffference, "Saddle difference (in sig)", &status);
    fits_write_key(_file, TINT, "MAXPER", (void*) &starList.maxNbPeaksPerObject, "Max num of peaks per object", &status);
    fits_write_key(_file, TINT, "MAXPEAKS", (void*) &starList.maxNbPeaksTotal, "Max num of peaks total", &status);
    fits_write_key(_file, TINT, "MAXSIZE", (void*) &starList.maxSize, "Max size for extended objects", &status);
    fits_write_key(_file, TINT, "HALFBOX", (void*) &starList.slidingSkyWindowHalfSize, "Half-size for sliding sky window", &status);

    fits_write_comment(
        _file,
        "The X and Y points are specified assuming 1,1 is the center of the leftmost bottom pixel of the "
        "image in accordance with the FITS standard.",
        &status
    );

    return (status == 0);
}

//-----------------------------------------------------------------------------

bool FITS::readBitmap(Bitmap* bitmap)
{
    return readBitmap(0, bitmap);
}

//-----------------------------------------------------------------------------

bool FITS::readBitmap(const std::string& name, Bitmap* bitmap)
{
    int status = 0;

    // Search the correct HDU
    if (!name.empty())
    {
        fits_movnam_hdu(_file, IMAGE_HDU, (char*) name.c_str(), 0, &status);
    }
    else
    {
        int nb = nbHDUs();
        int type = ASCII_TBL;

        for (int i = 1; (i <= nb) && (type != IMAGE_HDU); ++i)
        {
            fits_movabs_hdu(_file, i, &type, &status);
            if (status != 0)
                return false;
        }

        if (type != IMAGE_HDU)
            return false;
    }

    // Read the image
    return readBitmapFromCurrentHDU(bitmap);
}

//-----------------------------------------------------------------------------

bool FITS::readBitmap(int index, Bitmap* bitmap)
{
    int status = 0;

    // Search the correct HDU
    int nb = nbHDUs();
    int nbImages = 0;
    int type = ASCII_TBL;

    for (int i = 1; i <= nb; ++i)
    {
        fits_movabs_hdu(_file, i, &type, &status);
        if (status != 0)
            return false;
        
        if (type == IMAGE_HDU)
        {
            ++nbImages;
            if (nbImages == index + 1)
            {
                // Read the image
                return readBitmapFromCurrentHDU(bitmap);
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------

bool FITS::readBitmapFromCurrentHDU(Bitmap* bitmap)
{
    int status = 0;
    int bitpix;
    int naxis;
    long naxes[3];

    fits_get_img_param(_file, 3, &bitpix, &naxis, naxes, &status);
    if (status != 0)
        return false;

    Bitmap* dest = nullptr;

    if ((((bitmap->channels() == 1) && (naxis == 2)) || ((naxis == 3) && (naxes[2] == bitmap->channels()))) &&
        (bitmap->width() == naxes[0]) && (bitmap->height() == naxes[1]))
    {
        dest = bitmap;
    }
    else if (naxis == 3)
    {
        if (bitpix == 8)
            dest = new UInt8ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == 16)
            dest = new UInt16ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == 32)
            dest = new UInt32ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == -32)
            dest = new FloatColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == -64)
            dest = new DoubleColorBitmap(naxes[0], naxes[1]);
    }
    else if (naxis == 2)
    {
        if (bitpix == 8)
            dest = new UInt8GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == 16)
            dest = new UInt16GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == 32)
            dest = new UInt32GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == -32)
            dest = new FloatGrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == -64)
            dest = new DoubleGrayBitmap(naxes[0], naxes[1]);
    }

    if (!dest)
        return false;

    int datatype = TBYTE;
    if (dest->isFloatingPoint())
    {
        datatype = (dest->channelSize() == 4 ? TFLOAT : TDOUBLE);
    }
    else
    {
        datatype = (dest->channelSize() == 1
                    ? TBYTE
                    : dest->channelSize() == 2
                      ? TUSHORT
                      : dest->channelSize() == 4
                        ? TUINT
                        : TULONG
        );
    }

    long fpixel[3] = { 1, 1, 1 };

    for (fpixel[1] = 1; fpixel[1] <= dest->height(); ++fpixel[1])
    {
        uint8_t* ptr = dest->ptr(fpixel[1] - 1);

        for (fpixel[0] = 1; fpixel[0] <= dest->width(); ++fpixel[0])
        {
            for (fpixel[2] = 1; fpixel[2] <= dest->channels(); ++fpixel[2])
            {
                fits_read_pix(_file, datatype, fpixel, 1, nullptr, ptr, nullptr, &status);
                ptr += dest->channelSize();
            }
        }
    }

    if (status != 0)
    {
        if (dest != bitmap)
            delete dest;
        return false;
    }

    // Retrieve the bitmap info
    bitmap_info_t& info = dest->info();
    fits_read_key(_file, TUINT, "ISO", &info.isoSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "APERTURE", &info.aperture, nullptr, &status);
    fits_read_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, nullptr, &status);

    if (dest != bitmap)
    {
        bitmap->set(dest);
        delete dest;
    }

    return true;
}
