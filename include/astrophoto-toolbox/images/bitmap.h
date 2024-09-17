/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/images/bitmapinfo.h>
#include <vector>
#include <cmath>
#include <stdint.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  The different ranges of values supported by the bitmaps
    //------------------------------------------------------------------------------------
    enum range_t
    {
        RANGE_BYTE,     ///< 0-255
        RANGE_USHORT,   ///< 0-65535
        RANGE_UINT,     ///< 0-2^32-1
        RANGE_ONE,      ///< 0.0-1.0, for floating-point bitmaps only

        RANGE_SOURCE,   ///< During bitmap copy, keep the source bitmap range if possible
        RANGE_DEST,     ///< During bitmap copy, use the destination bitmap range
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different color spaces supported by the bitmaps
    //------------------------------------------------------------------------------------
    enum space_t
    {
        SPACE_LINEAR,
        SPACE_sRGB,

        SPACE_SOURCE,   ///< During bitmap copy, keep the source bitmap space
        SPACE_DEST,     ///< During bitmap copy, use the destination bitmap space
    };


    //------------------------------------------------------------------------------------
    /// @brief  Container for a bitmap image
    ///
    /// This class cannot be instantiated directly, you must use the TypedBitmap template
    /// or one of the predefined types.
    ///
    /// All bitmaps are stored in a uint8_t array, regardless of the real data type of the
    /// pixel components.
    ///
    /// Bitmaps have a data range associated with them. When converting a bitmap from a
    /// format to another, the desired range of values for the destination bitmap can be
    /// specified. By default, it is the one of the destination bitmap.
    ///
    /// Bitmaps with more bytes per row than necessary to hold the real horizontal pixel
    /// count are also supported. Thus, special care must be taken when you want to
    /// iterate over all the pixels.
    //------------------------------------------------------------------------------------
    class Bitmap
    {
        //_____ Construction / Destruction __________
    public:
        Bitmap() = delete;
        virtual ~Bitmap() = default;


    protected:
        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint,
               range_t defaultRange);

        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint, range_t range,
               range_t defaultRange);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, range_t defaultRange);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, range_t range, range_t defaultRange);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow,
               range_t defaultRange);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow,
               range_t range, range_t defaultRange);


        //_____ Methods __________
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
        inline uint8_t channels() const
        {
            return _channels;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the size in bytes of each channel component of the pixels
        //--------------------------------------------------------------------------------
        inline size_t channelSize() const
        {
            return _channelSize;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of bytes per pixel
        //--------------------------------------------------------------------------------
        inline unsigned int bytesPerPixel() const
        {
            return _channels * _channelSize;
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
        /// @brief  Returns the total size of the buffer holding the bitmap, in bytes
        //--------------------------------------------------------------------------------
        inline unsigned int size() const
        {
            return _bytesPerRow * _height;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if the pixmap contains floating-point numbers (float or
        ///         double)
        //--------------------------------------------------------------------------------
        inline bool isFloatingPoint() const
        {
            return _floatingPoint;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the range of the values in the bitmap
        //--------------------------------------------------------------------------------
        inline range_t range() const
        {
            return _range;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Change the range of the values in the bitmap, optionally applying it
        ///         (the default)
        //--------------------------------------------------------------------------------
        bool setRange(range_t range, bool apply = true);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the default range for bitmaps of this type
        //--------------------------------------------------------------------------------
        virtual range_t defaultRange() = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the color space of the bitmap
        //--------------------------------------------------------------------------------
        inline space_t space() const
        {
            return _space;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Change the color space of the bitmap, optionally applying it
        ///         (the default)
        //--------------------------------------------------------------------------------
        bool setSpace(space_t space, bool apply = true);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the informations about the capture of the bitmap
        //--------------------------------------------------------------------------------
        inline bitmap_info_t& info()
        {
            return _info;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the informations about the capture of the bitmap
        //--------------------------------------------------------------------------------
        inline bitmap_info_t info() const
        {
            return _info;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline uint8_t* ptr()
        {
            return _data.data();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline const uint8_t* ptr() const
        {
            return _data.data();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the beginnining of the specified row
        //--------------------------------------------------------------------------------
        inline uint8_t* ptr(unsigned int y)
        {
            return _data.data() + y * _bytesPerRow;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the beginnining of the specified row
        //--------------------------------------------------------------------------------
        inline const uint8_t* ptr(unsigned int y) const
        {
            return _data.data() + y * _bytesPerRow;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the pixel at the specified coordinates
        //--------------------------------------------------------------------------------
        inline uint8_t* ptr(unsigned int x, unsigned int y)
        {
            return _data.data() + y * _bytesPerRow + x * bytesPerPixel();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the pixel at the specified coordinates
        //--------------------------------------------------------------------------------
        inline const uint8_t* ptr(unsigned int x, unsigned int y) const
        {
            return _data.data() + y * _bytesPerRow + x * bytesPerPixel();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Resize the bitmap to the specified dimensions
        ///
        /// The bitmap is filled with 0.
        //--------------------------------------------------------------------------------
        void resize(unsigned int width, unsigned int height);

        //--------------------------------------------------------------------------------
        /// @brief  Resize the bitmap to the specified dimensions, but with more bytes per
        ///         row than would be necessary
        ///
        /// The bitmap is filled with 0.
        //--------------------------------------------------------------------------------
        void resize(unsigned int width, unsigned int height, unsigned int bytesPerRow);

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        ///
        /// The bitmap is resized if necessary.
        //--------------------------------------------------------------------------------
        void set(uint8_t* data, unsigned int width, unsigned int height);

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels and the number of bytes per row
        /// are correct.
        ///
        /// The bitmap is resized if necessary.
        //--------------------------------------------------------------------------------
        void set(
            uint8_t* data, unsigned int width, unsigned int height,
            unsigned int bytesPerRow
        );

        //--------------------------------------------------------------------------------
        /// @brief  Fill the bitmap with a copy of the provided one
        ///
        /// The bitmap is resized if necessary. The pixel values are scaled to the range
        /// of this bitmap by default (for floating point images: between 0 and 1), but
        /// this behavior can be disabled.
        //--------------------------------------------------------------------------------
        bool set(
            const Bitmap* bitmap, range_t range = RANGE_DEST, space_t space = SPACE_DEST
        );

        //--------------------------------------------------------------------------------
        /// @brief  Returns a single-channel bitmap containing a copy of one channel of
        ///         this bitmap
        ///
        /// The caller is responsible to delete the returned bitmap.
        //--------------------------------------------------------------------------------
        Bitmap* channel(uint8_t index) const;

        //--------------------------------------------------------------------------------
        /// @brief  Set one channel of this bitmap to the values provided
        //--------------------------------------------------------------------------------
        bool setChannel(uint8_t index, Bitmap* channel);


        //_____ Attributes __________
    protected:
        std::vector<uint8_t> _data;
        unsigned int _width = 0;
        unsigned int _height = 0;
        uint8_t _channels = 0;
        size_t _channelSize = 0;
        bool _floatingPoint = false;
        unsigned int _bytesPerRow = 0;
        range_t _range = RANGE_BYTE;
        space_t _space = SPACE_LINEAR;
        bitmap_info_t _info;
    };


    template<typename T>
    constexpr range_t rangeof()
    {
        if (std::is_same_v<T, uint8_t>)
            return RANGE_BYTE;
        else if (std::is_same_v<T, uint16_t>)
            return RANGE_USHORT;
        else if (std::is_same_v<T, uint32_t>)
            return RANGE_UINT;
        else if (std::is_same_v<T, float>)
            return RANGE_ONE;
        else if (std::is_same_v<T, double>)
            return RANGE_ONE;
        else
            return RANGE_BYTE;
    }


    //------------------------------------------------------------------------------------
    /// @brief  Typed representation of a bitmap image
    ///
    /// The template parameters are the underlying data type and the number of channels.
    ///
    /// Bitmaps have a data range associated with them. When converting a bitmap from a
    /// format to another, the desired range of values for the destination bitmap can be
    /// specified. By default, it is the one of the destination bitmap.
    ///
    /// Bitmaps with more bytes per row than necessary to hold the real horizontal pixel
    /// count are also supported. Thus, special care must be taken when you want to
    /// iterate over all the pixels.
    //------------------------------------------------------------------------------------
    template<typename T, uint8_t CHANNELS>
        requires(CHANNELS == 1 || CHANNELS == 3)
    class TypedBitmap : public Bitmap
    {
    public:
        static constexpr uint8_t Channels = CHANNELS;
        static constexpr size_t ChannelSize = sizeof(T);
        static constexpr range_t DefaultRange = rangeof<T>();
        static constexpr bool FloatingPoint = std::is_integral_v<T>;


        //_____ Construction / Destruction __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        TypedBitmap()
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, _defaultRange)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        TypedBitmap(range_t range)
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, range, _defaultRange)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        TypedBitmap(space_t space)
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, _defaultRange)
        {
            _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        TypedBitmap(range_t range, space_t space)
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, range, _defaultRange)
        {
            _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(unsigned int width, unsigned int height)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, _defaultRange)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(
            unsigned int width, unsigned int height, range_t range,
            space_t space = SPACE_LINEAR
        )
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, range,
                 _defaultRange)
        {
            _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(unsigned int width, unsigned int height, unsigned int bytesPerRow)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow,
                 _defaultRange)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(
            unsigned int width, unsigned int height, unsigned int bytesPerRow,
            range_t range, space_t space = SPACE_LINEAR
        )
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow,
                 range, _defaultRange)
        {
           _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        //--------------------------------------------------------------------------------
        TypedBitmap(T* data, unsigned int width, unsigned int height)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, _defaultRange)
        {
            set((uint8_t*) data, width, height);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        //--------------------------------------------------------------------------------
        TypedBitmap(
            T* data, unsigned int width, unsigned int height, range_t range,
            space_t space = SPACE_LINEAR
        )
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, range,
                 _defaultRange)
        {
            _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
            set((uint8_t*) data, width, height);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels and the number of bytes per row
        /// are correct.
        //--------------------------------------------------------------------------------
        TypedBitmap(
            T* data, unsigned int width, unsigned int height, unsigned int bytesPerRow
        )
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow,
                 _defaultRange)
        {
            set((uint8_t*) data, width, height, bytesPerRow);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels and the number of bytes per row
        /// are correct.
        //--------------------------------------------------------------------------------
        TypedBitmap(
            T* data, unsigned int width, unsigned int height, unsigned int bytesPerRow,
            range_t range, space_t space = SPACE_LINEAR
        )
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow,
                 range, _defaultRange)
        {
            _space = (space >= SPACE_SOURCE ? SPACE_LINEAR : space);
            set((uint8_t*) data, width, height, bytesPerRow);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        //--------------------------------------------------------------------------------
        template<typename T2, uint8_t CHANNELS2>
        TypedBitmap(
            const TypedBitmap<T2, CHANNELS2>& bitmap, range_t range = RANGE_DEST,
            space_t space = SPACE_DEST
        )
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, range, _defaultRange)
        {
            set(&bitmap, range, space);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        //--------------------------------------------------------------------------------
        TypedBitmap(
            Bitmap* bitmap, range_t range = RANGE_DEST, space_t space = SPACE_DEST
        )
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>, range, _defaultRange)
        {
            set(bitmap, range, space);
        }

        virtual ~TypedBitmap() = default;


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline T* data()
        {
            return (T*) _data.data();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a pointer to the buffer holding the bitmap
        //--------------------------------------------------------------------------------
        inline const T* data() const
        {
            return (T*) _data.data();
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
        /// @brief  Returns the default range for bitmaps of this type
        //--------------------------------------------------------------------------------
        range_t defaultRange() override
        {
            return _defaultRange;
        }

    protected:
        static constexpr range_t _defaultRange = rangeof<T>();
    };


    typedef TypedBitmap<uint8_t, 3> UInt8ColorBitmap;
    typedef TypedBitmap<uint16_t, 3> UInt16ColorBitmap;
    typedef TypedBitmap<uint32_t, 3> UInt32ColorBitmap;
    typedef TypedBitmap<float_t, 3> FloatColorBitmap;
    typedef TypedBitmap<double_t, 3> DoubleColorBitmap;

    typedef TypedBitmap<uint8_t, 1> UInt8GrayBitmap;
    typedef TypedBitmap<uint16_t, 1> UInt16GrayBitmap;
    typedef TypedBitmap<uint32_t, 1> UInt32GrayBitmap;
    typedef TypedBitmap<float_t, 1> FloatGrayBitmap;
    typedef TypedBitmap<double_t, 1> DoubleGrayBitmap;
}
