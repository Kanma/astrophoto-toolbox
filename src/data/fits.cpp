/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/data/fits.h>
#include <assert.h>

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

int FITS::nbTables() const
{
    int status = 0;
    int nb = nbHDUs();
    int type;
    int nbTables = 0;

    for (int i = 1; i <= nb; ++i)
    {
        fits_movabs_hdu(_file, i, &type, &status);
        if ((status == 0) && (type == BINARY_TBL))
            ++nbTables;
    }

    return nbTables;
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

    double vmin = 0.0;
    double vmax;

    if (bitmap->range() == RANGE_BYTE)
        vmax = 255.0;
    else if (bitmap->range() == RANGE_USHORT)
        vmax = 65535.0;
    else if (bitmap->range() == RANGE_UINT)
        vmax = 4294967295.0;
    else if (bitmap->range() == RANGE_ONE)
        vmax = 1.0;

    fits_write_key(_file, TDOUBLE, "DATAMIN", &vmin, "minimum data value", &status);
    fits_write_key(_file, TDOUBLE, "DATAMAX", &vmax, "maximum data value", &status);

    bitmap_info_t info = bitmap->info();
    fits_write_key(_file, TUINT, "ISO", &info.isoSpeed, "ISO speed", &status);
    fits_write_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, "Shutter speed", &status);
    fits_write_key(_file, TFLOAT, "APERTURE", &info.aperture, "Aperture", &status);
    fits_write_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, "Focal length", &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

bool FITS::write(
    const star_list_t& stars, const size2d_t& imageSize, const std::string& name,
    bool overwrite
)
{
    int status = 0;
    bool tableExisting = false;

    // Does the table already exist?
    if (!name.empty())
    {
        fits_movnam_hdu(_file, BINARY_TBL, (char*) name.c_str(), 0, &status);
        if (status == 0)
        {
            if (!overwrite)
                return false;

            long nrows;
            fits_get_num_rows(_file, &nrows, &status);
            fits_delete_rows(_file, 1, nrows, &status);
            fits_insert_rows(_file, 0, stars.size(), &status);

            if (status != 0)
                return false;

            tableExisting = true;
        }

        status = 0;
    }

    // Creates the table if necessary
    if (!tableExisting)
    {
        const char* ttype[] = {"X", "Y", "INTENSITY", "QUALITY", "MEANRADIUS"};
        const char* tform[] = {"D", "D", "E", "E", "E"};
        const char* tunit[] = {"pix", "pix", "unknown", "unknown", "pix"};

        fits_create_tbl(
            _file, BINARY_TBL, stars.size(), 5, (char**) ttype, (char**) tform, (char**) tunit,
            (name.empty() ? nullptr : name.c_str()), &status
        );
    }

    // Save the stars in the table
    const star_t* src = stars.data();
    for (size_t i = 1; i <= stars.size(); ++i)
    {
        fits_write_col(_file, TDOUBLE, 1, i, 1, 1, (void*) &src->position.x, &status);
        fits_write_col(_file, TDOUBLE, 2, i, 1, 1, (void*) &src->position.y, &status);
        fits_write_col(_file, TDOUBLE, 3, i, 1, 1, (void*) &src->intensity, &status);
        fits_write_col(_file, TDOUBLE, 4, i, 1, 1, (void*) &src->quality, &status);
        fits_write_col(_file, TDOUBLE, 4, i, 1, 1, (void*) &src->meanRadius, &status);
        ++src;
    }

    // Save the other data
    fits_update_key(_file, TINT, "IMAGEW", (void*) &imageSize.width, "Width of the image", &status);
    fits_update_key(_file, TINT, "IMAGEH", (void*) &imageSize.height, "Height of the image", &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

bool FITS::writeAstrometryNetKeywords(const size2d_t& imageSize)
{
    if (!gotoHDU(0, ANY_HDU))
        return false;

    int status = 0;

    const int sTRUE = 1;
    const int sFALSE = 0;
    const int tweak = 2;

    fits_update_key(_file, TINT, "IMAGEW", (void*) &imageSize.width, "image width", &status);
    fits_update_key(_file, TINT, "IMAGEH", (void*) &imageSize.height, "image height", &status);
    fits_update_key(_file, TLOGICAL, "ANRUN", (void*) &sTRUE, "Solve this field!", &status);
    fits_update_key(_file, TLOGICAL, "ANVERUNI", (void*) &sTRUE, "Uniformize field during verification", &status);
    fits_update_key(_file, TLOGICAL, "ANVERDUP", (void*) &sFALSE, "Deduplicate field during verification", &status);
    fits_update_key(_file, TLOGICAL, "ANTWEAK", (void*) &sTRUE, "Tweak: yes please!   ", &status);
    fits_update_key(_file, TINT, "ANTWEAKO", (void*) &tweak, "Tweak order    ", &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

Bitmap* FITS::readBitmap(const std::string& name)
{
    if (!gotoHDU(name, IMAGE_HDU))
        return nullptr;

    return readBitmapFromCurrentHDU();
}

//-----------------------------------------------------------------------------

Bitmap* FITS::readBitmap(int index)
{
    if (!gotoHDU(index, IMAGE_HDU))
        return nullptr;

    return readBitmapFromCurrentHDU();
}

//-----------------------------------------------------------------------------

star_list_t FITS::readStars(
    const std::string& name, size2d_t* imageSize
)
{
    if (!gotoHDU(name, BINARY_TBL))
        return star_list_t();

    return readStarsFromCurrentHDU(imageSize);
}

//-----------------------------------------------------------------------------

star_list_t FITS::readStars(int index, size2d_t* imageSize)
{
    if (!gotoHDU(index, BINARY_TBL))
        return star_list_t();

    return readStarsFromCurrentHDU(imageSize);
}

//-----------------------------------------------------------------------------

Bitmap* FITS::readBitmapFromCurrentHDU()
{
    int status = 0;
    int bitpix;
    int naxis;
    long naxes[3];

    fits_get_img_param(_file, 3, &bitpix, &naxis, naxes, &status);
    if (status != 0)
        return nullptr;

    Bitmap* dest = nullptr;

    if (naxis == 3)
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
        return nullptr;


    double vmax;
    bool hasRange = true;
    fits_read_key(_file, TDOUBLE, "DATAMAX", &vmax, nullptr, &status);
    if (status == 0)
    {
        if (vmax <= 2.0)
            dest->setRange(RANGE_ONE, false);
        else if (vmax <= 256.0)
            dest->setRange(RANGE_BYTE, false);
        else if (vmax <= 65536.0)
            dest->setRange(RANGE_USHORT, false);
        else
            dest->setRange(RANGE_UINT, false);
    }
    else
    {
        status = 0;
        hasRange = false;
    }

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
        delete dest;
        return nullptr;
    }

    if (!hasRange && dest->isFloatingPoint())
    {
        double maxValue = 0.0;

        if (dest->channelSize() == 4)
        {
            for (unsigned int y = 0; y < dest->height(); ++y)
            {
                float* data = (float*) dest->ptr(y);
                for (unsigned int x = 0; x < dest->width(); ++x)
                    maxValue = std::max(maxValue, double(data[x]));
            }
        }
        else if (dest->channelSize() == 8)
        {
            for (unsigned int y = 0; y < dest->height(); ++y)
            {
                double* data = (double*) dest->ptr(y);
                for (unsigned int x = 0; x < dest->width(); ++x)
                    maxValue = std::max(maxValue, data[x]);
            }
        }

        if (maxValue > 65535.0)
            dest->setRange(RANGE_UINT, false);
        else if (maxValue > 255.0)
            dest->setRange(RANGE_USHORT, false);
        if (maxValue > 1.0)
            dest->setRange(RANGE_BYTE, false);
    }

    // Retrieve the bitmap info
    bitmap_info_t& info = dest->info();
    fits_read_key(_file, TUINT, "ISO", &info.isoSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "APERTURE", &info.aperture, nullptr, &status);
    fits_read_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, nullptr, &status);

    return dest;
}

//-----------------------------------------------------------------------------

star_list_t FITS::readStarsFromCurrentHDU(size2d_t* imageSize)
{
    int status = 0;
    star_list_t stars;

    long nrows;
    fits_get_num_rows(_file, &nrows, &status);
    if (status != 0)
        return stars;

    // Load the stars from the table
    stars.resize(nrows);

    const star_t* dst = stars.data();
    for (size_t i = 1; i <= stars.size(); ++i)
    {
        fits_read_col(_file, TDOUBLE, 1, i, 1, 1, nullptr, (void*) &dst->position.x, nullptr, &status);
        fits_read_col(_file, TDOUBLE, 2, i, 1, 1, nullptr, (void*) &dst->position.y, nullptr, &status);
        fits_read_col(_file, TDOUBLE, 3, i, 1, 1, nullptr, (void*) &dst->intensity, nullptr, &status);
        fits_read_col(_file, TDOUBLE, 4, i, 1, 1, nullptr, (void*) &dst->quality, nullptr, &status);
        fits_read_col(_file, TDOUBLE, 4, i, 1, 1, nullptr, (void*) &dst->meanRadius, nullptr, &status);
        ++dst;
    }

    // Load the other data
    if (imageSize)
    {
        fits_read_key(_file, TINT, "IMAGEW", (void*) &imageSize->width, nullptr, &status);
        fits_read_key(_file, TINT, "IMAGEH", (void*) &imageSize->height, nullptr, &status);
    }

    return stars;
}

//-----------------------------------------------------------------------------

bool FITS::gotoHDU(const std::string& name, int type)
{
    int status = 0;

    // Search the correct HDU
    if (!name.empty())
    {
        fits_movnam_hdu(_file, type, (char*) name.c_str(), 0, &status);
        if (status != 0)
            return false;
    }
    else
    {
        int nb = nbHDUs();
        int hduType = ANY_HDU;

        for (int i = 1; (i <= nb) && (hduType != type); ++i)
        {
            fits_movabs_hdu(_file, i, &hduType, &status);
            if (status != 0)
                return false;
        }

        if (hduType != type)
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool FITS::gotoHDU(int index, int type)
{
    int status = 0;

    // Search the correct HDU
    int nb = nbHDUs();
    int nbHDUs = 0;
    int hduType = ANY_HDU;

    for (int i = 1; i <= nb; ++i)
    {
        fits_movabs_hdu(_file, i, &hduType, &status);
        if (status != 0)
            return false;
        
        if ((hduType == type) || (type == ANY_HDU))
        {
            ++nbHDUs;
            if (nbHDUs == index + 1)
                return true;
        }
    }

    return false;
}
