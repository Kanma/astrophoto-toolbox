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
                      ? TSHORT
                      : bitmap->channelSize() == 4
                        ? TINT
                        : TLONG
        );
    }

    long fpixel[3] = { 1, 1, 1 };
    int16_t v16;
    int32_t v32;
    int64_t v64;
    float f32;
    double f64;
    int64_t bzero = (datatype == TSHORT
                     ? 0x8000
                     : datatype == TINT
                       ? 0x80000000
                       : datatype == TLONG
                         ? 0x8000000000000000
                         : 0
    );

    for (fpixel[1] = 1; fpixel[1] <= bitmap->height(); ++fpixel[1])
    {
        uint8_t* ptr = bitmap->ptr(bitmap->height() - fpixel[1]);

        for (fpixel[0] = 1; fpixel[0] <= bitmap->width(); ++fpixel[0])
        {
            for (fpixel[2] = 1; fpixel[2] <= bitmap->channels(); ++fpixel[2])
            {
                if ((datatype == TBYTE) || (datatype == TFLOAT) || (datatype == TDOUBLE))
                {
                    fits_write_pix(_file, datatype, fpixel, 1, ptr, &status);
                }
                else if (datatype == TSHORT)
                {
                    v16 = (int16_t) (int64_t(*((uint16_t*) ptr)) - bzero);
                    fits_write_pix(_file, datatype, fpixel, 1, &v16, &status);
                }
                else if (datatype == TINT)
                {
                    v32 = (int32_t) (int64_t(*((uint32_t*) ptr)) - bzero);
                    fits_write_pix(_file, datatype, fpixel, 1, &v32, &status);
                }
                else if (datatype == TLONG)
                {
                    v64 = int64_t(*((uint64_t*) ptr)) - bzero;
                    fits_write_pix(_file, datatype, fpixel, 1, &v64, &status);
                }

                ptr += bitmap->channelSize();
            }
        }
    }

    bitmap_info_t info = bitmap->info();
    fits_write_key(_file, TUINT, "ISO", &info.isoSpeed, "ISO speed", &status);
    fits_write_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, "Shutter speed", &status);
    fits_write_key(_file, TFLOAT, "APERTURE", &info.aperture, "Aperture", &status);
    fits_write_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, "Focal length", &status);

    return (status == 0);
}

//-----------------------------------------------------------------------------

Bitmap* FITS::readBitmap(const std::string& name)
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
                return nullptr;
        }

        if (type != IMAGE_HDU)
            return nullptr;
    }

    // Read the image
    return readBitmapFromCurrentHDU();
}

//-----------------------------------------------------------------------------

Bitmap* FITS::readBitmap(int index)
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
            return nullptr;
        
        if (type == IMAGE_HDU)
        {
            ++nbImages;
            if (nbImages == index + 1)
            {
                // Read the image
                return readBitmapFromCurrentHDU();
            }
        }
    }

    return nullptr;
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

    Bitmap* bitmap = nullptr;
    if (naxis == 3)
    {
        if (bitpix == 8)
            bitmap = new UInt8ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == 16)
            bitmap = new UInt16ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == 32)
            bitmap = new UInt32ColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == -32)
            bitmap = new FloatColorBitmap(naxes[0], naxes[1]);
        else if (bitpix == -64)
            bitmap = new DoubleColorBitmap(naxes[0], naxes[1]);
    }
    else if (naxis == 2)
    {
        if (bitpix == 8)
            bitmap = new UInt8GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == 16)
            bitmap = new UInt16GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == 32)
            bitmap = new UInt32GrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == -32)
            bitmap = new FloatGrayBitmap(naxes[0], naxes[1]);
        else if (bitpix == -64)
            bitmap = new DoubleGrayBitmap(naxes[0], naxes[1]);
    }

    if (!bitmap)
        return nullptr;

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
                      ? TSHORT
                      : bitmap->channelSize() == 4
                        ? TINT
                        : TLONG
        );
    }

    long fpixel[3] = { 1, 1, 1 };
    int16_t v16;
    int32_t v32;
    int64_t v64;
    int64_t bzero = (datatype == TSHORT
                     ? 0x8000
                     : datatype == TINT
                       ? 0x80000000
                       : datatype == TLONG
                         ? 0x8000000000000000
                         : 0
    );

    for (fpixel[1] = 1; fpixel[1] <= bitmap->height(); ++fpixel[1])
    {
        uint8_t* ptr = bitmap->ptr(bitmap->height() - fpixel[1]);

        for (fpixel[0] = 1; fpixel[0] <= bitmap->width(); ++fpixel[0])
        {
            for (fpixel[2] = 1; fpixel[2] <= bitmap->channels(); ++fpixel[2])
            {
                if ((datatype == TBYTE) || (datatype == TFLOAT) || (datatype == TDOUBLE))
                {
                    fits_read_pix(_file, datatype, fpixel, 1, nullptr, ptr, nullptr, &status);
                }
                else if (datatype == TSHORT)
                {
                    fits_read_pix(_file, datatype, fpixel, 1, nullptr, &v16, nullptr, &status);
                    *((uint16_t*) ptr) = uint16_t(int64_t(v16) + bzero);
                }
                else if (datatype == TINT)
                {
                    fits_read_pix(_file, datatype, fpixel, 1, nullptr, &v32, nullptr, &status);
                    *((uint32_t*) ptr) = uint32_t(int64_t(v32) + bzero);
                }
                else if (datatype == TLONG)
                {
                    fits_read_pix(_file, datatype, fpixel, 1, nullptr, &v64, nullptr, &status);
                    *((uint64_t*) ptr) = uint64_t(int64_t(v64) + bzero);
                }

                ptr += bitmap->channelSize();
            }
        }
    }

    if (status != 0)
    {
        delete bitmap;
        return nullptr;
    }

    // Retrieve the bitmap info
    bitmap_info_t& info = bitmap->info();
    fits_read_key(_file, TUINT, "ISO", &info.isoSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "SHUTTER_SPEED", &info.shutterSpeed, nullptr, &status);
    fits_read_key(_file, TFLOAT, "APERTURE", &info.aperture, nullptr, &status);
    fits_read_key(_file, TFLOAT, "FOCAL_LENGTH", &info.focalLength, nullptr, &status);

    return bitmap;
}
