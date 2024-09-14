#include <astrophoto-toolbox/algorithms/histogram.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <string.h>


namespace astrophototoolbox {

void computeHistogram(Bitmap* bitmap, histogram_t& histogram)
{
    UInt16GrayBitmap* gray = requiresFormat<UInt16GrayBitmap>(bitmap);

    histogram.resize(size_t(std::numeric_limits<uint16_t>::max()) + 1);
    memset(histogram.data(), 0, histogram.size() * sizeof(uint16_t));

    uint16_t* ptr = gray->data();
    for (unsigned int i = 0; i < gray->width() * gray->height(); ++i)
    {
        ++histogram[*ptr];
        ++ptr;
    }

    if (gray != bitmap)
        delete gray;
}

}
