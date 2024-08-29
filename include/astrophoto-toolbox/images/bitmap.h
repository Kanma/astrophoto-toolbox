#pragma once

#include <astrophoto-toolbox/images/bitmapinfo.h>
#include <vector>
#include <cmath>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Container for a bitmap image
    ///
    /// This class cannot be instantiated directly, you must use the TypedBitmap template
    /// or one of the predefined types.
    ///
    /// All bitmaps are stored in a uint8_t array, regardless of the real data type of the
    /// pixel components.
    ///
    /// Automatic conversion is performed between different bitmap types, on a full-range
    /// basis (ie. uint8 0-255 it scaled to uint16 0-65535). Float bitmaps have a range of
    /// 0.0-1.0.
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
        Bitmap(uint8_t channels, size_t channelSize, bool floatingPoint);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint);

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        Bitmap(unsigned int width, unsigned int height, uint8_t channels,
               size_t channelSize, bool floatingPoint, unsigned int bytesPerRow);


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
        /// The bitmap is resized if necessary, automatic conversion is performed if
        /// needed.
        //--------------------------------------------------------------------------------
        void set(const Bitmap* bitmap);


        //_____ Attributes __________
    protected:
        std::vector<uint8_t> _data;
        unsigned int _width = 0;
        unsigned int _height = 0;
        uint8_t _channels = 0;
        size_t _channelSize = 0;
        bool _floatingPoint = false;
        unsigned int _bytesPerRow = 0;
        bitmap_info_t _info;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Typed representation of a bitmap image
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
    template<typename T, uint8_t CHANNELS>
        requires(CHANNELS == 1 || CHANNELS == 3)
    class TypedBitmap : public Bitmap
    {
        //_____ Construction / Destruction __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Construct an empty bitmap (width & height = 0)
        ///
        /// Can be resized or filled later.
        //--------------------------------------------------------------------------------
        TypedBitmap()
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(unsigned int width, unsigned int height)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap with the specified dimensions, but with more bytes
        ///         per row than would be necessary
        ///
        /// All pixels are set to 0.
        //--------------------------------------------------------------------------------
        TypedBitmap(unsigned int width, unsigned int height, unsigned int bytesPerRow)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow)
        {
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided buffer
        ///
        /// It is expected than the dimensions provided are exactly those of the image in
        /// the buffer, and that the number of channels is correct.
        //--------------------------------------------------------------------------------
        TypedBitmap(T* data, unsigned int width, unsigned int height)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>)
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
        TypedBitmap(T* data, unsigned int width, unsigned int height,
                             unsigned int bytesPerRow)
        : Bitmap(width, height, CHANNELS, sizeof(T), !std::is_integral_v<T>, bytesPerRow)
        {
            set(data, width, height, bytesPerRow);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        ///
        /// Automatic conversion is performed as needed.
        //--------------------------------------------------------------------------------
        template<typename T2, uint8_t CHANNELS2>
        TypedBitmap(const TypedBitmap<T2, CHANNELS2>& bitmap)
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>)
        {
            set(&bitmap);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Construct a bitmap by copying the provided one
        ///
        /// Automatic conversion is performed as needed.
        //--------------------------------------------------------------------------------
        TypedBitmap(Bitmap* bitmap)
        : Bitmap(CHANNELS, sizeof(T), !std::is_integral_v<T>)
        {
            set(bitmap);
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
