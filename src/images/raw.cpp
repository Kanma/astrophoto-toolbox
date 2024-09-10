#include <astrophoto-toolbox/images/raw.h>

using namespace astrophototoolbox;


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

RawImage::RawImage()
{
    _processor.imgdata.params.use_auto_wb = 0;
    _processor.imgdata.params.use_fuji_rotate = 0;
    _processor.imgdata.params.no_auto_bright = 1;
    _processor.imgdata.params.user_flip = 0;
    _processor.imgdata.params.output_bps = 16;
    _processor.imgdata.params.output_color = 1;
}

//-----------------------------------------------------------------------------

RawImage::~RawImage()
{
    _processor.recycle();
}


/************************************** METHODS ****************************************/

bool RawImage::open(const std::string& filename)
{
    int err = _processor.open_file(filename.c_str());

    if (err == 0)
        err = _processor.unpack();

    return (err == 0);
}

//-----------------------------------------------------------------------------

bool RawImage::open(void* buffer, size_t size)
{
    int err = _processor.open_buffer(buffer, size);

    if (err == 0)
        err = _processor.unpack();

    return (err == 0);
}

//-----------------------------------------------------------------------------

unsigned int RawImage::width() const
{
    return _processor.imgdata.sizes.width;
}

//-----------------------------------------------------------------------------

unsigned int RawImage::height() const
{
    return _processor.imgdata.sizes.height;
}

//-----------------------------------------------------------------------------

uint8_t RawImage::channels() const
{
    return _processor.imgdata.idata.colors;
}

//-----------------------------------------------------------------------------

unsigned int RawImage::isoSpeed() const
{
    return (unsigned int) _processor.imgdata.other.iso_speed;
}

//-----------------------------------------------------------------------------

float RawImage::shutterSpeed() const
{
    return _processor.imgdata.other.shutter;
}

//-----------------------------------------------------------------------------

float RawImage::aperture() const
{
    return _processor.imgdata.other.aperture;
}

//-----------------------------------------------------------------------------

float RawImage::focalLength() const
{
    return _processor.imgdata.other.focal_len;
}

//-----------------------------------------------------------------------------

bool RawImage::toBitmap(Bitmap* bitmap, bool useCameraWhiteBalance, bool linear)
{
    _processor.imgdata.params.use_camera_wb = (useCameraWhiteBalance ? 1 : 0);

    if (linear)
    {
        _processor.imgdata.params.gamm[0] = 1.0;
        _processor.imgdata.params.gamm[1] = 1.0;
    }
    else
    {
        _processor.imgdata.params.gamm[0] = 1.0 / 2.4;
        _processor.imgdata.params.gamm[1] = 12.92;
    }

    // Handle special cases
    if (bitmap->channelSize() == 1)
        _processor.imgdata.params.output_bps = 8;
    else
        _processor.imgdata.params.output_bps = 16;

    int err = _processor.dcraw_process();

    if (err == 0)
    {
        libraw_processed_image_t* processedImage = _processor.dcraw_make_mem_image(&err);

        if (processedImage)
        {
            Bitmap* src = nullptr;

            if (processedImage->colors == 3)
            {
                if (processedImage->bits == 16)
                    src = new UInt16ColorBitmap();
                else if (processedImage->bits == 8)
                    src = new UInt8ColorBitmap();
            }
            else if (processedImage->colors == 1)
            {
                if (processedImage->bits == 16)
                    src = new UInt16GrayBitmap();
                else if (processedImage->bits == 8)
                    src = new UInt8GrayBitmap();
            }

            if (src)
            {
                src->set(processedImage->data, processedImage->width, processedImage->height);
                bitmap->set(src);
                delete src;
            }
            else
            {
                err = 1;
            }

            _processor.dcraw_clear_mem(processedImage);
        }
    }

    bitmap_info_t& info = bitmap->info();
    info.isoSpeed = isoSpeed();
    info.shutterSpeed = shutterSpeed();
    info.aperture = aperture();
    info.focalLength = focalLength();

    return (err == 0);
}
