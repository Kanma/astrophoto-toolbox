#include <astrophoto-toolbox/images/bitmap.h>

using namespace astrophototoolbox;


/********************************** HELPER FUNCTIONS ************************************/

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2, typename FACTOR_TYPE>
    requires(std::is_same_v<T1, T2> && (CHANNELS1 == CHANNELS2))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, FACTOR_TYPE factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        memcpy(dest, src, destBitmap->bytesPerRow());
        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
        dest = (T1*)((uint8_t*) dest + destBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2, typename FACTOR_TYPE>
    requires(!std::is_same_v<T1, T2> && (CHANNELS1 == CHANNELS2) && (CHANNELS1 != 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, FACTOR_TYPE factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            for (unsigned int c = 0; c < CHANNELS1; ++c)
            {
                *dest = T1(FACTOR_TYPE(*src2) * factor);
                ++src2;
                ++dest;
            }
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2, typename FACTOR_TYPE>
    requires(!std::is_same_v<T1, T2> && (CHANNELS1 == CHANNELS2) && (CHANNELS1 == 1))
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, FACTOR_TYPE factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            *dest = T1(FACTOR_TYPE(*src2) * factor);
            ++src2;
            ++dest;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2, typename FACTOR_TYPE>
    requires(CHANNELS1 != CHANNELS2 && CHANNELS1 == 1)
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, FACTOR_TYPE factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            FACTOR_TYPE sum = 0.0;

            for (unsigned int c = 0; c < CHANNELS2; ++c)
            {
                sum += FACTOR_TYPE(*src2);
                ++src2;
            }

            *dest = T1(sum * factor / FACTOR_TYPE(CHANNELS2));
            ++dest;
        }

        src = (T2*)((uint8_t*) src + srcBitmap->bytesPerRow());
    }
}

//-----------------------------------------------------------------------------

template<typename T1, uint8_t CHANNELS1, typename T2, uint8_t CHANNELS2, typename FACTOR_TYPE>
    requires(CHANNELS1 != CHANNELS2 && CHANNELS2 == 1)
void convert(TypedBitmap<T1, CHANNELS1>* destBitmap, const TypedBitmap<T2, CHANNELS2>* srcBitmap, FACTOR_TYPE factor)
{
    const T2* src = srcBitmap->data();
    T1* dest = destBitmap->data();

    for (unsigned int y = 0; y < destBitmap->height(); ++y)
    {
        const T2* src2 = src;

        for (unsigned int x = 0; x < destBitmap->width(); ++x)
        {
            T1 v = T1(FACTOR_TYPE(*src2) * factor);

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


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Bitmap::Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint)
: _width(0), _height(0), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(0)
{
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint)
: _width(width), _height(height), _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(width * channels * channelSize)
{
    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}

//-----------------------------------------------------------------------------

Bitmap::Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow)
: _width(width), _height(height),  _channels(channels), _channelSize(channelSize),
  _floatingPoint(floatingPoint), _bytesPerRow(bytesPerRow)
{
    assert(bytesPerRow >= width * channels * channelSize);
    _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    memset((void*) _data.data(), 0, _data.size() * sizeof(uint8_t));
}


/************************************** METHODS ****************************************/

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

#define CONVERT_COLOR_INTEGER(dest, bitmap)    \
    {   \
        if (bitmap->channelSize() == 1)     \
            convert(dest, dynamic_cast<const UInt8ColorBitmap*>(bitmap), factor);   \
        else if (bitmap->channelSize() == 2)     \
            convert(dest, dynamic_cast<const UInt16ColorBitmap*>(bitmap), factor);  \
        else if (bitmap->channelSize() == 4)     \
            convert(dest, dynamic_cast<const UInt32ColorBitmap*>(bitmap), factor);  \
    }


#define CONVERT_GRAY_INTEGER(dest, bitmap)    \
    {   \
        if (bitmap->channelSize() == 1)     \
            convert(dest, dynamic_cast<const UInt8GrayBitmap*>(bitmap), factor);   \
        else if (bitmap->channelSize() == 2)     \
            convert(dest, dynamic_cast<const UInt16GrayBitmap*>(bitmap), factor);  \
        else if (bitmap->channelSize() == 4)     \
            convert(dest, dynamic_cast<const UInt32GrayBitmap*>(bitmap), factor);  \
    }


#define CONVERT_COLOR_FLOAT(dest, bitmap)    \
    {   \
        if (bitmap->channelSize() == 4)     \
            convert(dest, dynamic_cast<const FloatColorBitmap*>(bitmap), factor);   \
        else if (bitmap->channelSize() == 8)     \
            convert(dest, dynamic_cast<const DoubleColorBitmap*>(bitmap), factor);  \
    }


#define CONVERT_GRAY_FLOAT(dest, bitmap)    \
    {   \
        if (bitmap->channelSize() == 4)     \
            convert(dest, dynamic_cast<const FloatGrayBitmap*>(bitmap), factor);   \
        else if (bitmap->channelSize() == 8)     \
            convert(dest, dynamic_cast<const DoubleGrayBitmap*>(bitmap), factor);  \
    }


#define CONVERT_INTEGER_TO_INTEGER(dest, bitmap)    \
    {   \
        const double factor =  (_channelSize >= bitmap->channelSize()   \
            ? std::pow(2.0, (_channelSize - bitmap->channelSize()) * 8)   \
            : 1.0 / std::pow(2.0, (bitmap->channelSize() - _channelSize) * 8)   \
        );   \
    \
        if (bitmap->channels() == 3)   \
            CONVERT_COLOR_INTEGER(dest, bitmap)   \
        else   \
            CONVERT_GRAY_INTEGER(dest, bitmap)   \
    }


#define CONVERT_INTEGER_TO_FLOAT(dest, bitmap)    \
    {   \
        const double factor = 1.0 / (std::pow(2.0, bitmap->channelSize() * 8) - 1);   \
    \
        if (bitmap->channels() == 3)   \
            CONVERT_COLOR_INTEGER(dest, bitmap)   \
        else   \
            CONVERT_GRAY_INTEGER(dest, bitmap)   \
    }


#define CONVERT_FLOAT_TO_INTEGER(dest, bitmap)    \
    {   \
        const double factor = std::pow(2.0, _channelSize * 8) - 1;   \
    \
        if (bitmap->channels() == 3)   \
            CONVERT_COLOR_FLOAT(dest, bitmap)   \
        else   \
            CONVERT_GRAY_FLOAT(dest, bitmap)   \
    }


#define CONVERT_FLOAT_TO_FLOAT(dest, bitmap)    \
    {   \
        const double factor = 1.0;   \
    \
        if (bitmap->channels() == 3)   \
            CONVERT_COLOR_FLOAT(dest, bitmap)   \
        else   \
            CONVERT_GRAY_FLOAT(dest, bitmap)   \
    }


#define CONVERT_INTEGER(dest, bitmap)    \
    if (bitmap->isFloatingPoint())    \
        CONVERT_FLOAT_TO_INTEGER(dest, bitmap)    \
    else    \
        CONVERT_INTEGER_TO_INTEGER(dest, bitmap)


#define CONVERT_FLOAT(dest, bitmap)    \
    if (bitmap->isFloatingPoint())    \
        CONVERT_FLOAT_TO_FLOAT(dest, bitmap)    \
    else    \
        CONVERT_INTEGER_TO_FLOAT(dest, bitmap)



void Bitmap::set(const Bitmap* bitmap)
{
    assert(bitmap);

    if ((_width != bitmap->width()) || (_height != bitmap->height()) || (_bytesPerRow != bitmap->width() * _channels * _channelSize))
    {
        _width = bitmap->width();
        _height = bitmap->height();
        _bytesPerRow = bitmap->width() * _channels * _channelSize;
        _data.resize(_bytesPerRow * _height / sizeof(uint8_t));
    }

    if (isFloatingPoint())
    {
        if (_channels == 3)
        {
            if (_channelSize == 4)
            {
                FloatColorBitmap* dest = dynamic_cast<FloatColorBitmap*>(this);
                CONVERT_FLOAT(dest, bitmap)
            }
            else if (_channelSize == 8)
            {
                DoubleColorBitmap* dest = dynamic_cast<DoubleColorBitmap*>(this);
                CONVERT_FLOAT(dest, bitmap)
            }
        }
        else if (_channels == 1)
        {
            if (_channelSize == 4)
            {
                FloatGrayBitmap* dest = dynamic_cast<FloatGrayBitmap*>(this);
                CONVERT_FLOAT(dest, bitmap)
            }
            else if (_channelSize == 8)
            {
                DoubleGrayBitmap* dest = dynamic_cast<DoubleGrayBitmap*>(this);
                CONVERT_FLOAT(dest, bitmap)
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
                CONVERT_INTEGER(dest, bitmap)
            }
            else if (_channelSize == 2)
            {
                UInt16ColorBitmap* dest = dynamic_cast<UInt16ColorBitmap*>(this);
                CONVERT_INTEGER(dest, bitmap)
            }
            else if (_channelSize == 4)
            {
                UInt32ColorBitmap* dest = dynamic_cast<UInt32ColorBitmap*>(this);
                CONVERT_INTEGER(dest, bitmap)
            }
        }
        else if (_channels == 1)
        {
            if (_channelSize == 1)
            {
                UInt8GrayBitmap* dest = dynamic_cast<UInt8GrayBitmap*>(this);
                CONVERT_INTEGER(dest, bitmap)
            }
            else if (_channelSize == 2)
            {
                UInt16GrayBitmap* dest = dynamic_cast<UInt16GrayBitmap*>(this);
                CONVERT_INTEGER(dest, bitmap)
            }
            else if (_channelSize == 4)
            {
                UInt32GrayBitmap* dest = dynamic_cast<UInt32GrayBitmap*>(this);
                CONVERT_INTEGER(dest, bitmap)
            }
        }
    }
}
