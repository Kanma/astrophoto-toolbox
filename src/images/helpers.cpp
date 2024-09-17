#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/algorithms/histogram.h>

namespace astrophototoolbox {

double CONVERSION_FACTORS[4][4] = {
    // RANGE_BYTE -> BYTE               | USHORT        | UINT          | ONE
    {                1.0,                 257.0,          16843009.0,     1.0 / 255.0 },
    // RANGE_USHORT -> ...
    {                1.0 / 257.0,         1.0,            65537.0,        1.0 / 65535.0 },
    // RANGE_UINT -> ...
    {                1.0 / 16843009.0,    1.0 / 65537.0,  1.0,            1.0 / 4294967295.0 },
    // RANGE_ONE -> ...
    {                255.0,               65535.0,        4294967295.0,   1.0 },
};

//-----------------------------------------------------------------------------

double getConversionFactor(range_t from, range_t to)
{
    return CONVERSION_FACTORS[from][to];
}

//-----------------------------------------------------------------------------

DoubleGrayBitmap* computeLuminanceBitmap(Bitmap* bitmap)
{
    // Only one channel: convert to the output format
    if (bitmap->channels() == 1)
        return new DoubleGrayBitmap(bitmap, RANGE_ONE);    

    // Ensure the bitmap is in the correct format
    DoubleColorBitmap* color = requiresFormat<DoubleColorBitmap>(bitmap, RANGE_ONE);

    // Compute the luminance
    DoubleGrayBitmap* luminance = new DoubleGrayBitmap(color->width(), color->height());
    for (unsigned int y = 0; y < color->height(); ++y)
    {
        double* src = color->data(y);
        double* dest = luminance->data(y);

        for (unsigned int x = 0; x < color->width(); ++x)
        {
            double minv = std::min(src[0], std::min(src[1], src[2]));
            double maxv = std::max(src[0], std::max(src[1], src[2]));

            *dest = (minv + maxv) * 0.5;

            src += 3;
            ++dest;
        }
    }

    // Cleanup, if necessary
    if (color != bitmap)
        delete color;

    return luminance;
}

//-----------------------------------------------------------------------------

double computeMedian(DoubleGrayBitmap* bitmap)
{
    // Compute the histogram of the bitmap
    histogram_t histogram;
    computeHistogram(bitmap, histogram);

    // Compute the median
    const size_t nbTotalValues = bitmap->width() * bitmap->height() / 2;
    size_t nbValues = 0;
    size_t index = 0;
    while (nbValues < nbTotalValues)
        nbValues += histogram[index++];

    return double(index) / 65535.0;
}

//-----------------------------------------------------------------------------

void removeHotPixels(Bitmap* bitmap)
{
    const double hotFactor = 4.0;

    // Ensure the bitmap is in the correct format
    DoubleColorBitmap* color = nullptr;
    DoubleGrayBitmap* gray = nullptr;
    double* data = nullptr;
    unsigned int rowSize;

    if (bitmap->channels() == 3)
    {
        color = requiresFormat<DoubleColorBitmap>(bitmap, RANGE_ONE);
        data = color->data();
        rowSize = color->bytesPerRow() / color->channelSize();
    }
    else
    {
        gray = requiresFormat<DoubleGrayBitmap>(bitmap, RANGE_ONE);
        data = gray->data();
        rowSize = gray->bytesPerRow() / gray->channelSize();
    }

    // Remove the hot pixels
    for (unsigned int y = 1; y < bitmap->height() - 1; ++y)
    {
        for (unsigned int x = 1; x < bitmap->width() - 1; ++x)
        {
            size_t offset = y * rowSize + x * bitmap->channels();

            for (unsigned int c = 0; c < bitmap->channels(); ++c)
            {
                double testValue = data[offset];

                if ((testValue > hotFactor * data[offset - bitmap->channels()]) &&
                    (testValue > hotFactor * data[offset + bitmap->channels()]) &&
                    (testValue > hotFactor * data[offset - rowSize]) &&
                    (testValue > hotFactor * data[offset + rowSize]))
                {
                    data[offset] = 0.0;
                }

                ++offset;
            }
        }
    }

    if (color && (color != bitmap))
    {
        bitmap->set(color);
        delete color;
    }
    else if (gray && (gray != bitmap))
    {
        bitmap->set(gray);
        delete gray;
    }
}

}
