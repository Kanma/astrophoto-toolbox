#include <catch.hpp>
#include "fits_helpers.h"
#include <fstream>


int getHeaderEntryPosition(const std::string& header, const std::string& key)
{
    int pos = header.find((key + " ").c_str());
    if (pos == -1)
        pos = header.find((key + "=").c_str());

    if (pos >= 0)
    {
        int pos2 = getHeaderEntryPosition(header.substr(pos + 1), key);
        REQUIRE(pos2 == -1);
    }

    return pos;
}


void checkHeaderEntry(const std::string& header, const std::string& key, int value)
{
    int pos = getHeaderEntryPosition(header, key);
    REQUIRE(pos >= 0);

    char buffer[10] = { 0 };
    snprintf(buffer, 10, " %d ", value);

    pos = header.find(buffer, pos);
    REQUIRE(pos >= 0);
}


void checkHeaderEntry(const std::string& header, const std::string& key, double value)
{
    int pos = getHeaderEntryPosition(header, key);
    REQUIRE(pos >= 0);

    char buffer[10] = { 0 };

    snprintf(buffer, 10, " %f ", value);
    int pos2 = header.find(buffer, pos);
    if (pos2 == -1)
    {
        snprintf(buffer, 10, " %ld. ", long(value));
        pos2 = header.find(buffer, pos);
    }

    REQUIRE(pos2 >= 0);
}


void checkHeaderEntry(const std::string& header, const std::string& key, const std::string& value)
{
    int pos = getHeaderEntryPosition(header, key);
    REQUIRE(pos >= 0);

    char buffer[20] = { 0 };
    snprintf(buffer, 20, "'%s", value.c_str());

    pos = header.find(buffer, pos);
    REQUIRE(pos >= 0);
}


void checkBoolHeaderEntry(const std::string& header, const std::string& key, bool value)
{
    int pos = getHeaderEntryPosition(header, key);
    REQUIRE(pos >= 0);

    char buffer[10] = { 0 };
    snprintf(buffer, 10, value ? " T " : " F ");

    pos = header.find(buffer, pos);
    REQUIRE(pos >= 0);
}


void checkBitmapHeader(
    const char* filename, int bitpix, double datamax, int naxis, int naxis1, int naxis2, int naxis3,
    const std::string& extname
)
{
    std::ifstream test(filename);
    REQUIRE(test.is_open());

    char header[2881] = { 0 };
    test.read(header, 2880);

    std::string sheader(header);

    checkHeaderEntry(sheader, "NAXIS", naxis);

    if (naxis > 0)
    {
        checkHeaderEntry(sheader, "BITPIX", bitpix);
        checkHeaderEntry(sheader, "DATAMIN", 0.0);
        checkHeaderEntry(sheader, "DATAMAX", datamax);
        checkHeaderEntry(sheader, "NAXIS1", naxis1);
        checkHeaderEntry(sheader, "NAXIS2", naxis2);
    }

    if (naxis3 != 0)
        checkHeaderEntry(sheader, "NAXIS3", naxis3);

    if (!extname.empty())
        checkHeaderEntry(sheader, "EXTNAME", extname);
}
