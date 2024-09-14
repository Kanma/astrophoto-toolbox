#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <string.h>
#include <assert.h>

using namespace astrophototoolbox;


/********************************** HELPER FUNCTIONS ************************************/

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2>
    requires((CHANNELS1 == CHANNELS2) && (CHANNELS1 != 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, double factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int i = 0; i < destBitmap->width() * CHANNELS1; ++i)
        {
            *dest = T1(double(*src2) * factor);
            ++src2;
            ++dest;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2>
    requires((CHANNELS1 == CHANNELS2) && (CHANNELS1 == 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, double factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            *dest = T1(double(*src2) * factor);
            ++src2;
            ++dest;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2>
    requires((CHANNELS1 != CHANNELS2) && (CHANNELS1 == 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, double factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            double sum = 0.0;

            for (unsigned int c = 0; c < CHANNELS2; ++c)
            {
                sum += double(*src2);
                ++src2;
            }

            *dest = T1(sum * factor / double(CHANNELS2));
            ++dest;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2>
    requires((CHANNELS1 != CHANNELS2) && (CHANNELS2 == 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, double factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            T1 v = T1(double(*src2) * factor);

            for (unsigned int c = 0; c < CHANNELS1; ++c)
            {
                *dest = v;
                ++dest;
            }

            ++src2;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T, uint8_t CHANNELS>
void convertTosRGB(TypedBitmap<T, CHANNELS>* bitmap)
{
    T* ptr = bitmap->data();

    const double factorToOne = getConversionFactor(bitmap->range(), RANGE_ONE);
    const double factorFromOne = getConversionFactor(RANGE_ONE, bitmap->range());
    const double exponent = 1.0 / 2.4;

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        T* ptr2 = ptr;

        for (unsigned int i = 0; i < bitmap->width() * CHANNELS; ++i)
        {
            double v = double(*ptr2) * factorToOne;

            if (v <= 0.0031308)
                v *= 12.92;
            else
                v = (1.0 + 0.055) * pow(v, exponent) - 0.055;

            *ptr2 = T(v * factorFromOne);
            ++ptr2;
        }

        ptr = (T*)((uint8_t*) ptr + bitmap->bytesPerRow());
    }

    bitmap->setSpace(SPACE_sRGB, false);
}

//-----------------------------------------------------------------------------

template<typename T, uint8_t CHANNELS>
    requires(std::is_integral_v<T>)
void convertToLinear(TypedBitmap<T, CHANNELS>* bitmap)
{
    T* ptr = bitmap->data();

    const double factorToOne = getConversionFactor(bitmap->range(), RANGE_ONE);
    const double factorFromOne = getConversionFactor(RANGE_ONE, bitmap->range());

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        T* ptr2 = ptr;

        for (unsigned int i = 0; i < bitmap->width() * CHANNELS; ++i)
        {
            double v = double(*ptr2) * factorToOne;

            if (v <= 0.04045)
                v /= 12.92;
            else
                v = pow(((v + 0.055) / (1.0 + 0.055)), 2.4);

            *ptr2 = T(std::round(v * factorFromOne));
            ++ptr2;
        }

        ptr = (T*)((uint8_t*) ptr + bitmap->bytesPerRow());
    }

    bitmap->setSpace(SPACE_LINEAR, false);
}

//-----------------------------------------------------------------------------

template<typename T, uint8_t CHANNELS>
    requires(!std::is_integral_v<T>)
void convertToLinear(TypedBitmap<T, CHANNELS>* bitmap)
{
    T* ptr = bitmap->data();

    const double factorToOne = getConversionFactor(bitmap->range(), RANGE_ONE);
    const double factorFromOne = getConversionFactor(RANGE_ONE, bitmap->range());

    for (unsigned int y = 0; y < bitmap->height(); ++y)
    {
        T* ptr2 = ptr;

        for (unsigned int i = 0; i < bitmap->width() * CHANNELS; ++i)
        {
            double v = double(*ptr2) * factorToOne;

            if (v <= 0.04045)
                v /= 12.92;
            else
                v = pow(((v + 0.055) / (1.0 + 0.055)), 2.4);

            *ptr2 = T(v * factorFromOne);
            ++ptr2;
        }

        ptr = (T*)((uint8_t*) ptr + bitmap->bytesPerRow());
    }

    bitmap->setSpace(SPACE_LINEAR, false);
}

//-----------------------------------------------------------------------------

template<typename T, uint8_t CHANNELS>
void convert(TypedBitmap<T, CHANNELS>* destBitmap, const Bitmap* srcBitmap, double factor, space_t space)
{
    if (srcBitmap->isFloatingPoint())
    {
        if (srcBitmap->channels() == 3)
        {
            if (srcBitmap->channelSize() == 4)
            {
                const FloatColorBitmap* src = dynamic_cast<const FloatColorBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 8)
            {
                const DoubleColorBitmap* src = dynamic_cast<const DoubleColorBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
        }
        else if (srcBitmap->channels() == 1)
        {
            if (srcBitmap->channelSize() == 4)
            {
                const FloatGrayBitmap* src = dynamic_cast<const FloatGrayBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 8)
            {
                const DoubleGrayBitmap* src = dynamic_cast<const DoubleGrayBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
        }
    }
    else
    {
        if (srcBitmap->channels() == 3)
        {
            if (srcBitmap->channelSize() == 1)
            {
                const UInt8ColorBitmap* src = dynamic_cast<const UInt8ColorBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 2)
            {
                const UInt16ColorBitmap* src = dynamic_cast<const UInt16ColorBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 4)
            {
                const UInt32ColorBitmap* src = dynamic_cast<const UInt32ColorBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
        }
        else if (srcBitmap->channels() == 1)
        {
            if (srcBitmap->channelSize() == 1)
            {
                const UInt8GrayBitmap* src = dynamic_cast<const UInt8GrayBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 2)
            {
                const UInt16GrayBitmap* src = dynamic_cast<const UInt16GrayBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
            else if (srcBitmap->channelSize() == 4)
            {
                const UInt32GrayBitmap* src = dynamic_cast<const UInt32GrayBitmap*>(srcBitmap);
                convert(destBitmap, src, factor);
            }
        }
    }

    destBitmap->setSpace(srcBitmap->space(), false);

    if ((space == SPACE_LINEAR) && ((destBitmap->space() == SPACE_sRGB)))
        convertToLinear(destBitmap);
    else if ((space == SPACE_sRGB) && ((destBitmap->space() == SPACE_LINEAR)))
        convertTosRGB(destBitmap);
}


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Bitmap::Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint,
               range_t defaultRange)
: _width(0), _height(0), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(0)
{
    _range = defaultRange;
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint, range_t range,
               range_t defaultRange)
: _width(0), _height(0), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(0)
{
    if (range >= RANGE_SOURCE)
        _range = defaultRange;
    else if (!_floatingPoint && ((range == RANGE_ONE) || (range > defaultRange)))
        _range = defaultRange;
    else
        _range = range;
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, range_t defaultRange)
: _width(width), _height(height), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(width * channels * channelSize)
{
    _range = defaultRange;

    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, range_t range,
               range_t defaultRange)
: _width(width), _height(height), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(width * channels * channelSize)
{
    if (range >= RANGE_SOURCE)
        _range = defaultRange;
    else if (!_floatingPoint && ((range == RANGE_ONE) || (range > defaultRange)))
        _range = defaultRange;
    else
        _range = range;

    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow,
               range_t defaultRange)
: _width(width), _height(height),  _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(bytesPerRow)
{
    assert(bytesPerRow >= width * channels * channelSize);

    _range = defaultRange;

    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow,
               range_t range, range_t defaultRange)
: _width(width), _height(height),  _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(bytesPerRow)
{
    assert(bytesPerRow >= width * channels * channelSize);

    if (range >= RANGE_SOURCE)
        _range = defaultRange;
    else if (!_floatingPoint && ((range == RANGE_ONE) || (range > defaultRange)))
        _range = defaultRange;
    else
        _range = range;

    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}


/************************************** METHODS ****************************************/

bool Bitmap::setRange(range_t range, bool apply)
{
    if (range >= RANGE_SOURCE)
        return false;

    if (!_floatingPoint && ((range == RANGE_ONE) || (range > defaultRange())))
        return false;

    if (_range == range)
        return false;

    if (apply)
    {
        double factor = getConversionFactor(_range, range);

        if (_floatingPoint)
        {
            if (_channels == 3)
            {
                if (_channelSize == 4)
                {
                    FloatColorBitmap* dest = dynamic_cast<FloatColorBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 8)
                {
                    DoubleColorBitmap* dest = dynamic_cast<DoubleColorBitmap*>(this);
                    convert(dest, dest, factor);
                }
            }
            else if (_channels == 1)
            {
                if (_channelSize == 4)
                {
                    FloatGrayBitmap* dest = dynamic_cast<FloatGrayBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 8)
                {
                    DoubleGrayBitmap* dest = dynamic_cast<DoubleGrayBitmap*>(this);
                    convert(dest, dest, factor);
                }
            }
        }
        else
        {
            if (_channels == 3)
            {
                if (_channelSize == 1)
                {
                    UInt8ColorBitmap* dest = dynamic_cast<UInt8ColorBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 2)
                {
                    UInt16ColorBitmap* dest = dynamic_cast<UInt16ColorBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 4)
                {
                    UInt32ColorBitmap* dest = dynamic_cast<UInt32ColorBitmap*>(this);
                    convert(dest, dest, factor);
                }
            }
            else if (_channels == 1)
            {
                if (_channelSize == 1)
                {
                    UInt8GrayBitmap* dest = dynamic_cast<UInt8GrayBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 2)
                {
                    UInt16GrayBitmap* dest = dynamic_cast<UInt16GrayBitmap*>(this);
                    convert(dest, dest, factor);
                }
                else if (_channelSize == 4)
                {
                    UInt32GrayBitmap* dest = dynamic_cast<UInt32GrayBitmap*>(this);
                    convert(dest, dest, factor);
                }
            }
        }
    }

    _range = range;

    return true;
}

//-----------------------------------------------------------------------------

bool Bitmap::setSpace(space_t space, bool apply)
{
    if (space >= SPACE_SOURCE)
        return false;

    if (_space == space)
        return false;

    if (apply)
    {
        if (_floatingPoint)
        {
            if (_channels == 3)
            {
                if (_channelSize == 4)
                {
                    FloatColorBitmap* dest = dynamic_cast<FloatColorBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 8)
                {
                    DoubleColorBitmap* dest = dynamic_cast<DoubleColorBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
            }
            else if (_channels == 1)
            {
                if (_channelSize == 4)
                {
                    FloatGrayBitmap* dest = dynamic_cast<FloatGrayBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 8)
                {
                    DoubleGrayBitmap* dest = dynamic_cast<DoubleGrayBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
            }
        }
        else
        {
            if (_channels == 3)
            {
                if (_channelSize == 1)
                {
                    UInt8ColorBitmap* dest = dynamic_cast<UInt8ColorBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 2)
                {
                    UInt16ColorBitmap* dest = dynamic_cast<UInt16ColorBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 4)
                {
                    UInt32ColorBitmap* dest = dynamic_cast<UInt32ColorBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
            }
            else if (_channels == 1)
            {
                if (_channelSize == 1)
                {
                    UInt8GrayBitmap* dest = dynamic_cast<UInt8GrayBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 2)
                {
                    UInt16GrayBitmap* dest = dynamic_cast<UInt16GrayBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
                else if (_channelSize == 4)
                {
                    UInt32GrayBitmap* dest = dynamic_cast<UInt32GrayBitmap*>(this);
                    if (space == SPACE_LINEAR)
                        convertToLinear(dest);
                    else if (space == SPACE_sRGB)
                        convertTosRGB(dest);
                }
            }
        }
    }

    _space = space;

    return true;
}

//-----------------------------------------------------------------------------

void Bitmap::resize(unsigned int width, unsigned int height)
{
    if ((_width != width) || (_height != height) || (_bytesPerRow != _width * _channels * _channelSize))
    {
        _width = width;
        _height = height;
        _bytesPerRow = _width * _channels * _channelSize;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

void Bitmap::resize(unsigned int width, unsigned int height, unsigned int bytesPerRow)
{
    assert(bytesPerRow >= width * _channels * _channelSize);

    if ((_width != width) || (_height != height) || (_bytesPerRow != bytesPerRow))
    {
        _width = width;
        _height = height;
        _bytesPerRow = bytesPerRow;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

void Bitmap::set(uint8_t* data, unsigned int width, unsigned int height)
{
    assert(data);

    if ((_width != width) || (_height != height) || (_bytesPerRow != _width * _channels * _channelSize))
    {
        _width = width;
        _height = height;
        _bytesPerRow = _width *  _channels * _channelSize;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    memcpy((void*) _data.data(), (void*) data, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

void Bitmap::set(uint8_t* data, unsigned int width, unsigned int height, unsigned int bytesPerRow)
{
    assert(data);
    assert(bytesPerRow >= width * _channels * _channelSize);

    if ((_width != width) || (_height != height) || (_bytesPerRow != bytesPerRow))
    {
        _width = width;
        _height = height;
        _bytesPerRow = bytesPerRow;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    memcpy((void*) _data.data(), (void*) data, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

bool Bitmap::set(const Bitmap* bitmap, range_t range, space_t space)
{
    assert(bitmap);

    // Determine the destination range
    if (range == RANGE_SOURCE)
        range = bitmap->range();
    else if (range == RANGE_DEST)
        range = _range;

    // Determine the destination color space
    if (space == SPACE_SOURCE)
        space = bitmap->space();
    else if (space == SPACE_DEST)
        space = _space;

    if (!_floatingPoint)
    {
        if ((range == RANGE_ONE) || (range > defaultRange()))
            return false;
    }

    _range = range;

    // Resize the bitmap if necessary
    if ((_width != bitmap->width()) || (_height != bitmap->height()) || (_bytesPerRow != bitmap->width() * _channels * _channelSize))
    {
        _width = bitmap->width();
        _height = bitmap->height();
        _bytesPerRow = bitmap->width() * _channels * _channelSize;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    _info = bitmap->info();

    // Copy the source bitmap
    double factor = getConversionFactor(bitmap->range(), _range);

    if (_floatingPoint)
    {
        if (_channels == 3)
        {
            if (_channelSize == 4)
            {
                FloatColorBitmap* dest = dynamic_cast<FloatColorBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 8)
            {
                DoubleColorBitmap* dest = dynamic_cast<DoubleColorBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
        }
        else if (_channels == 1)
        {
            if (_channelSize == 4)
            {
                FloatGrayBitmap* dest = dynamic_cast<FloatGrayBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 8)
            {
                DoubleGrayBitmap* dest = dynamic_cast<DoubleGrayBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
        }
    }
    else
    {
        if (_channels == 3)
        {
            if (_channelSize == 1)
            {
                UInt8ColorBitmap* dest = dynamic_cast<UInt8ColorBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 2)
            {
                UInt16ColorBitmap* dest = dynamic_cast<UInt16ColorBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 4)
            {
                UInt32ColorBitmap* dest = dynamic_cast<UInt32ColorBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
        }
        else if (_channels == 1)
        {
            if (_channelSize == 1)
            {
                UInt8GrayBitmap* dest = dynamic_cast<UInt8GrayBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 2)
            {
                UInt16GrayBitmap* dest = dynamic_cast<UInt16GrayBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
            else if (_channelSize == 4)
            {
                UInt32GrayBitmap* dest = dynamic_cast<UInt32GrayBitmap*>(this);
                convert(dest, bitmap, factor, space);
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

Bitmap* Bitmap::channel(uint8_t index) const
{
    if (index >= _channels)
        return nullptr;

    Bitmap* result = nullptr;
    if (_floatingPoint)
    {
        if (_channelSize == 4)
            result = new FloatGrayBitmap(_width, _height, _range);
        else if (_channelSize == 8)
            result = new DoubleGrayBitmap(_width, _height, _range);
    }
    else
    {
        if (_channelSize == 1)
            result = new UInt8GrayBitmap(_width, _height, _range);
        else if (_channelSize == 2)
            result = new UInt16GrayBitmap(_width, _height, _range);
        else if (_channelSize == 4)
            result = new UInt32GrayBitmap(_width, _height, _range);
    }

    if (!result)
        return nullptr;

    for (unsigned int y = 0; y < _height; ++y)
    {
        const uint8_t* src = ptr(y) + index * _channelSize;
        uint8_t* dest = result->ptr(y);

        for (unsigned int x = 0; x < _width; ++x)
        {
            memcpy(dest, src, _channelSize);
            src += _channels * _channelSize;
            dest += _channelSize;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------

bool Bitmap::setChannel(uint8_t index, Bitmap* channel)
{
    if (!channel || (index >= _channels) || (channel->channels() != 1) || (channel->channelSize() != _channelSize) ||
        (channel->width() != _width) || (channel->height() != _height) || (channel->isFloatingPoint() != _floatingPoint) ||
        (channel->range() != _range))
    {
        return false;
    }

    for (unsigned int y = 0; y < _height; ++y)
    {
        const uint8_t* src = channel->ptr(y);
        uint8_t* dest = ptr(y) + index * _channelSize;

        for (unsigned int x = 0; x < _width; ++x)
        {
            memcpy(dest, src, _channelSize);
            src += _channelSize;
            dest += _channels * _channelSize;
        }
    }

    return true;
}
