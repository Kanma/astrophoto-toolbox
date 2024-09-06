#include <string>


void checkHeaderEntry(const std::string& header, const std::string& key, int value);

void checkHeaderEntry(const std::string& header, const std::string& key, double value);

void checkHeaderEntry(const std::string& header, const std::string& key, const std::string& value);

void checkBoolHeaderEntry(const std::string& header, const std::string& key, bool value);

void checkBitmapHeader(
    const char* filename, int bitpix, double datamax, int naxis, int naxis1, int naxis2,
    int naxis3 = 0, const std::string& extname = ""
);
