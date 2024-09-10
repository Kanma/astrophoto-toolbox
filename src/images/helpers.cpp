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

}
