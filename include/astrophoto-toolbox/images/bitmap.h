#pragma once

#include <vector>
#include <iostream>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Container for a bitmap image
    ///
    /// The template parameters are the underlying data type and the number of channels.
    ///
    /// Automatic conversion is performed between different bitmap types, on a full-range
    /// basis (ie. uint8 0-255 it scaled to uint16 0-65535). Float bitmaps have a range of
    /// 0.0-1.0.
    ///
    /// Bitmaps with more bytes per row than necessary to hold the real horizontal pixel
    /// count are also supported. Thus, special care must be taken when you want to
    /// iterate over all the pixels.
    //------------------------------------------------------------------------------------
    template<typename T, unsigned int CHANNELS>
        requires(CHANNELS == 1 || CHANNELS == 3)
    class Bitmap
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        Bitmap()
        : _width(0), _height(0), _bytesPerRow(0)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height)
        : _width(width), _height(height), _bytesPerRow(width * CHANNELS * sizeof(T))
        {
            _data.resize(_bytesPerRow * height / sizeof(T));
            memset((void*) _data.data(), 0, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, unsigned int bytesPerRow)
        : _width(width), _height(height), _bytesPerRow(bytesPerRow)
        {
            assert(bytesPerRow >= width * CHANNELS * sizeof(T));
            _data.resize(_bytesPerRow * height / sizeof(T));
            memset((void*) _data.data(), 0, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        //--------------------------------------------------------------------------------
        Bitmap(T* data, unsigned int width, unsigned int height)
        {
            set(data, width, height);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels and the number of bytes per row
        /// are correct.
        //--------------------------------------------------------------------------------
        Bitmap(T* data, unsigned int width, unsigned int height, unsigned int bytesPerRow)
        {
            set(data, width, height, bytesPerRow);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        //--------------------------------------------------------------------------------
        Bitmap(const Bitmap<T, CHANNELS>& bitmap)
        {
            set(bitmap);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        ///
        /// Automatic conversion is performed as needed.
        //--------------------------------------------------------------------------------
        template<typename T2, unsigned int CHANNELS2>
        Bitmap(const Bitmap<T2, CHANNELS2>& bitmap)
        {
            set(bitmap);
        }


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Returns the width of the bitmap
        //--------------------------------------------------------------------------------
        inline unsigned int width() const
        {
            return _width;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the height of the bitmap
        //--------------------------------------------------------------------------------
        inline unsigned int height() const
        {
            return _height;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of channels of the bitmap
        //--------------------------------------------------------------------------------
        inline unsigned int channels() const
        {
            return CHANNELS;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of bytes per pixel
        //--------------------------------------------------------------------------------
        inline unsigned int bytesPerPixel() const
        {
            return CHANNELS * sizeof(T);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of bytes per row
        ///
        /// Might be greater than width() * bytesPerPixel()
        //--------------------------------------------------------------------------------
        inline unsigned int bytesPerRow() const
        {
            return _bytesPerRow;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the total size of the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline unsigned int size() const
        {
            return _bytesPerRow * _height;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline T* data()
        {
            return _data.data();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline const T* data() const
        {
            return _data.data();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the beginnining of the specified row
        //--------------------------------------------------------------------------------
        inline T* data(unsigned int y)
        {
            return (T*)((uint8_t*) _data.data() + y * _bytesPerRow );
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the beginnining of the specified row
        //--------------------------------------------------------------------------------
        inline const T* data(unsigned int y) const
        {
            return (T*)((uint8_t*) _data.data() + y * _bytesPerRow );
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the pixel at the specified coordinates
        //--------------------------------------------------------------------------------
        inline T* data(unsigned int x, unsigned int y)
        {
            return (T*)((uint8_t*) _data.data() + y * _bytesPerRow + x * bytesPerPixel());
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the pixel at the specified coordinates
        //--------------------------------------------------------------------------------
        inline const T* data(unsigned int x, unsigned int y) const
        {
            return (T*)((uint8_t*) _data.data() + y * _bytesPerRow + x * bytesPerPixel());
        }

        //--------------------------------------------------------------------------------
        /// @brief  Resize the bitmap to the specified dimensions
        ///
        /// The bitmap is filled with 0.
        //--------------------------------------------------------------------------------
        void resize(unsigned int width, unsigned int height)
        {
            _width = width;
            _height = height;
            _bytesPerRow = _width * CHANNELS * sizeof(T);
            _data.resize(_bytesPerRow * _height / sizeof(T));
            memset((void*) _data.data(), 0, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Resize the bitmap to the specified dimensions, but with more bytes per
        ///         row than would be necessary
        ///
        /// The bitmap is filled with 0.
        //--------------------------------------------------------------------------------
        void resize(unsigned int width, unsigned int height, unsigned int bytesPerRow)
        {
            assert(bytesPerRow >= width * CHANNELS * sizeof(T));

            _width = width;
            _height = height;
            _bytesPerRow = bytesPerRow;
            _data.resize(_bytesPerRow * _height / sizeof(T));
            memset((void*) _data.data(), 0, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        ///
        /// The bitmap is resized if necessary.
        //--------------------------------------------------------------------------------
        void set(T* data, unsigned int width, unsigned int height)
        {
            assert(data);

            if ((_width != width) || (_height != height) || (_bytesPerRow != _width * CHANNELS * sizeof(T)))
            {
                _width = width;
                _height = height;
                _bytesPerRow = _width * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            memcpy((void*) _data.data(), (void*) data, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels and the number of bytes per row
        /// are correct.
        ///
        /// The bitmap is resized if necessary.
        //--------------------------------------------------------------------------------
        void set(T* data, unsigned int width, unsigned int height, unsigned int bytesPerRow)
        {
            assert(data);
            assert(bytesPerRow >= width * CHANNELS * sizeof(T));

            if ((_width != width) || (_height != height) || (_bytesPerRow != bytesPerRow))
            {
                _width = width;
                _height = height;
                _bytesPerRow = bytesPerRow;
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            memcpy((void*) _data.data(), (void*) data, _data.size() * sizeof(T));
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        void set(const Bitmap<T, CHANNELS>& bitmap)
        {
            if ((_width != bitmap.width()) || (_height != bitmap.height()) || (_bytesPerRow != bitmap.width() * CHANNELS * sizeof(T)))
            {
                _width = bitmap.width();
                _height = bitmap.height();
                _bytesPerRow = bitmap.width() * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            const T* src = bitmap.data();
            T* dest = _data.data();

            for (unsigned int y = 0; y < _height; ++y)
            {
                memcpy((void*) dest, (void*) src, _bytesPerRow);
                src = (T*)((uint8_t*) src + bitmap.bytesPerRow());
                dest = (T*)((uint8_t*) dest + _bytesPerRow);
            }
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        template<typename T2, unsigned int CHANNELS2>
            requires(std::is_integral_v<T> && std::is_integral_v<T2>)
        void set(const Bitmap<T2, CHANNELS2>& bitmap)
        {
            if ((_width != bitmap.width()) || (_height != bitmap.height()) || (_bytesPerRow != bitmap.width() * CHANNELS * sizeof(T)))
            {
                _width = bitmap.width();
                _height = bitmap.height();
                _bytesPerRow = bitmap.width() * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            const double factor =  (sizeof(T) > sizeof(T2)
                ? std::pow(2.0, (sizeof(T) - sizeof(T2)) * 8)
                : 1.0 / std::pow(2.0, (sizeof(T2) - sizeof(T)) * 8)
            );

            convert(bitmap, factor);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        template<typename T2, unsigned int CHANNELS2>
            requires(!std::is_integral_v<T> && std::is_integral_v<T2>)
        void set(const Bitmap<T2, CHANNELS2>& bitmap)
        {
            if ((_width != bitmap.width()) || (_height != bitmap.height()) || (_bytesPerRow != bitmap.width() * CHANNELS * sizeof(T)))
            {
                _width = bitmap.width();
                _height = bitmap.height();
                _bytesPerRow = bitmap.width() * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            const T factor = 1.0 / (std::pow(2.0, sizeof(T2) * 8) - 1);
            convert(bitmap, factor);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        template<typename T2, unsigned int CHANNELS2>
            requires(std::is_integral_v<T> && !std::is_integral_v<T2>)
        void set(const Bitmap<T2, CHANNELS2>& bitmap)
        {
            if ((_width != bitmap.width()) || (_height != bitmap.height()) || (_bytesPerRow != bitmap.width() * CHANNELS * sizeof(T)))
            {
                _width = bitmap.width();
                _height = bitmap.height();
                _bytesPerRow = bitmap.width() * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            const double factor = std::pow(2.0, sizeof(T) * 8) - 1;
            convert(bitmap, factor);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        template<typename T2, unsigned int CHANNELS2>
            requires(!std::is_integral_v<T> && !std::is_integral_v<T2>)
        void set(const Bitmap<T2, CHANNELS2>& bitmap)
        {
            if ((_width != bitmap.width()) || (_height != bitmap.height()) || (_bytesPerRow != bitmap.width() * CHANNELS * sizeof(T)))
            {
                _width = bitmap.width();
                _height = bitmap.height();
                _bytesPerRow = bitmap.width() * CHANNELS * sizeof(T);
                _data.resize(_bytesPerRow * _height / sizeof(T));
            }

            convert(bitmap, 1.0);
        }


    private:
        template<typename T2, unsigned int CHANNELS2, typename FACTOR_TYPE>
            requires(CHANNELS == CHANNELS2 && CHANNELS != 1)
        void convert(const Bitmap<T2, CHANNELS2>& bitmap, FACTOR_TYPE factor)
        {
            const T2* src = bitmap.data();
            T* dest = _data.data();

            for (unsigned int y = 0; y < _height; ++y)
            {
                const T2* src2 = src;

                for (unsigned int x = 0; x < _width; ++x)
                {
                    for (unsigned int c = 0; c < CHANNELS; ++c)
                    {
                        *dest = T(FACTOR_TYPE(*src2) * factor);
                        ++src2;
                        ++dest;
                    }
                }

                src = (T2*)((uint8_t*) src + bitmap.bytesPerRow());
            }
        }

        template<typename T2, unsigned int CHANNELS2, typename FACTOR_TYPE>
            requires(CHANNELS == CHANNELS2 && CHANNELS == 1)
        void convert(const Bitmap<T2, CHANNELS2>& bitmap, FACTOR_TYPE factor)
        {
            const T2* src = bitmap.data();
            T* dest = _data.data();

            for (unsigned int y = 0; y < _height; ++y)
            {
                const T2* src2 = src;

                for (unsigned int x = 0; x < _width; ++x)
                {
                    *dest = T(FACTOR_TYPE(*src2) * factor);
                    ++src2;
                    ++dest;
                }

                src = (T2*)((uint8_t*) src + bitmap.bytesPerRow());
            }
        }

        template<typename T2, unsigned int CHANNELS2, typename FACTOR_TYPE>
            requires(CHANNELS != CHANNELS2 && CHANNELS == 1)
        void convert(const Bitmap<T2, CHANNELS2>& bitmap, FACTOR_TYPE factor)
        {
            const T2* src = bitmap.data();
            T* dest = _data.data();

            for (unsigned int y = 0; y < _height; ++y)
            {
                const T2* src2 = src;

                for (unsigned int x = 0; x < _width; ++x)
                {
                    FACTOR_TYPE sum = 0.0;

                    for (unsigned int c = 0; c < CHANNELS2; ++c)
                    {
                        sum += FACTOR_TYPE(*src2);
                        ++src2;
                    }

                    *dest = T(sum * factor / FACTOR_TYPE(CHANNELS2));
                    ++dest;
                }

                src = (T2*)((uint8_t*) src + bitmap.bytesPerRow());
            }
        }

        template<typename T2, unsigned int CHANNELS2, typename FACTOR_TYPE>
            requires(CHANNELS != CHANNELS2 && CHANNELS2 == 1)
        void convert(const Bitmap<T2, CHANNELS2>& bitmap, FACTOR_TYPE factor)
        {
            const T2* src = bitmap.data();
            T* dest = _data.data();

            for (unsigned int y = 0; y < _height; ++y)
            {
                const T2* src2 = src;

                for (unsigned int x = 0; x < _width; ++x)
                {
                    T v = T(FACTOR_TYPE(*src2) * factor);

                    for (unsigned int c = 0; c < CHANNELS; ++c)
                    {
                        *dest = v;
                        ++dest;
                    }

                    ++src2;
                }

                src = (T2*)((uint8_t*) src + bitmap.bytesPerRow());
            }
        }


        //_____ Attributes __________
    private:
        std::vector<T> _data; 
        unsigned int _width = 0;
        unsigned int _height = 0;
        unsigned int _bytesPerRow = 0;
    };


    typedef Bitmap<uint8_t, 3> UInt8ColorBitmap;
    typedef Bitmap<uint16_t, 3> UInt16ColorBitmap;
    typedef Bitmap<uint32_t, 3> UInt32ColorBitmap;
    typedef Bitmap<float_t, 3> FloatColorBitmap;
    typedef Bitmap<double_t, 3> DoubleColorBitmap;

    typedef Bitmap<uint8_t, 1> UInt8GrayBitmap;
    typedef Bitmap<uint16_t, 1> UInt16GrayBitmap;
    typedef Bitmap<uint32_t, 1> UInt32GrayBitmap;
    typedef Bitmap<float_t, 1> FloatGrayBitmap;
    typedef Bitmap<double_t, 1> DoubleGrayBitmap;
}
