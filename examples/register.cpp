#include <SimpleOpt.h>
#include <iostream>
#include <fstream>
#include <string>

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/images/raw.h>

using namespace std;
using namespace astrophototoolbox;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_RAW,
    OPT_FITS,
    OPT_OUTPUT,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",               SO_NONE },
    { OPT_HELP,             "--help",           SO_NONE },
    { OPT_VERBOSE,          "-v",               SO_NONE },
    { OPT_VERBOSE,          "--verbose",        SO_NONE },
    { OPT_RAW,              "--raw",            SO_NONE },
    { OPT_FITS,             "--fits",           SO_NONE },
    { OPT_OUTPUT,            "-o",              SO_REQ_SEP },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "register" << endl
         << "Usage: " << strApplicationName << "[options] <--fits | --raw> <image>" << endl
         << endl
         << "Register a FITS or RAW image." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --fits            Indicates that the image is a FITS one" << endl
         << "    --raw             Indicates that the image is a RAW one" << endl
         << "    -o FILE           FITS file into which write the coordinates (default: the input FITS image if applicable)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;
    bool isRaw = false;
    bool isFits = false;
    bool includeANKeywords = true;

    // Parse the command-line parameters
    CSimpleOpt args(argc, argv, COMMAND_LINE_OPTIONS);
    while (args.Next())
    {
        if (args.LastError() == SO_SUCCESS)
        {
            switch (args.OptionId())
            {
                case OPT_HELP:
                    showUsage(argv[0]);
                    return 0;

                case OPT_VERBOSE:
                    verbose = true;
                    break;

                case OPT_RAW:
                    isRaw = true;
                    break;

                case OPT_FITS:
                    isFits = true;
                    break;

                case OPT_OUTPUT:
                    outputFileName = args.OptionArg();
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return 1;
        }
    }

    if (args.FileCount() != 1)
    {
        cerr << "Require an input file" << endl;
        return 1;
    }

    if (isRaw && isFits)
    {
        cerr << "Must indicates either --raw or --fits, not both" << endl;
        return 1;
    }
    else if (!isRaw && !isFits)
    {
        cerr << "Must indicates either --raw or --fits" << endl;
        return 1;
    }
    else if (isRaw && outputFileName.empty())
    {
        cerr << "Must specify an output file with -o when processing a RAW file" << endl;
        return 1;
    }


    // Open the image
    Bitmap* bitmap = nullptr;
    FITS fitsImage;

    if (isFits)
    {
        // Open the FITS file
        if (!fitsImage.open(args.File(0), false))
        {
            cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
            return 1;
        }

        // Retrieve the bitmap
        bitmap = fitsImage.readBitmap();
        if (!bitmap)
        {
            cerr << "Failed to read the bitmap from the FITS file" << endl;
            return 1;
        }
    }
    else
    {
        // Open the RAW image
        RawImage image;
        if (!image.open(args.File(0)))
        {
            cerr << "Failed to open the RAW file '" << args.File(0) << "'" << endl;
            return 1;
        }

        // Decode the RAW image
        if (image.channels() == 3)
            bitmap = new UInt16ColorBitmap();
        else
            bitmap = new UInt8ColorBitmap();

        if (!image.toBitmap(bitmap))
        {
            cerr << "Failed to convert the RAW image" << endl;
            delete bitmap;
            return 1;
        }
    }

    // Registration
    stacking::Registration registration;
    star_list_t stars = registration.registerBitmap(bitmap);

    size2d_t imageSize(bitmap->width(), bitmap->height());

    delete bitmap;

    if (verbose)
        cout << stars.size() << " star(s) detected" << endl;

    // Save the coordinates
    FITS* dest = (isFits ? &fitsImage : nullptr);
    FITS output;

    if (!outputFileName.empty())
    {
        if (std::ifstream(outputFileName).good())
            output.open(outputFileName, false);
        else
            output.create(outputFileName);

        dest = &output;
    }

    if (!dest->write(stars, imageSize, "STARS", true))
    {
        cerr << "Failed to save the coordinates in the FITS file" << endl;
        return 1;
    }

    dest->writeAstrometryNetKeywords(imageSize);

    return 0;
}
