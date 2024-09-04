#include <SimpleOpt.h>
#include <iostream>
#include <fstream>
#include <string>

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/astrometry/astrometry.h>

using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_OUTPUT,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,     "-h",           SO_NONE },
    { OPT_HELP,     "--help",       SO_NONE },
    { OPT_VERBOSE,  "-v",           SO_NONE },
    { OPT_VERBOSE,  "--verbose",    SO_NONE },
    { OPT_OUTPUT,   "-o",           SO_REQ_SEP },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "detect-stars" << endl
         << "Usage: " << strApplicationName << "[options] <fits_image>" << endl
         << endl
         << "Detect the stars in a FITS image." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h     Display this help" << endl
         << "    --verbose, -v  Display more details" << endl
         << "    -o <FILE>      FITS file into which write the coordinates (default: the input file)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;

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

    // Open the FITS file
    astrophototoolbox::FITS image;
    if (!image.open(args.File(0)))
    {
        cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
        return 1;
    }

    // Retrieve the bitmap
    astrophototoolbox::Bitmap* bitmap = image.readBitmap();
    if (!bitmap)
    {
        cerr << "Failed to read the bitmap from the FITS file" << endl;
        return 1;
    }

    // Convert it to float & keep the first channel
    if (bitmap->channels() == 3)
    {
        auto converted = new astrophototoolbox::FloatColorBitmap(bitmap, astrophototoolbox::RANGE_USHORT);
        auto channel = converted->channel(0);
        delete bitmap;
        delete converted;
        bitmap = channel;
    }
    else
    {
        auto converted = new astrophototoolbox::FloatGrayBitmap(bitmap, astrophototoolbox::RANGE_USHORT);
        delete bitmap;
        bitmap = converted;
    }

    // Star detection
    astrophototoolbox::Astrometry astrometry;
    if (!astrometry.detectStars(bitmap))
    {
        cerr << "Failed to detect the stars" << endl;
        delete bitmap;
        return 1;
    }

    delete bitmap;

    if (verbose)
    {
        cout << astrometry.getStarList().stars.size() << " star(s) detected" << endl;
    }

    // Save the coordinates
    astrophototoolbox::FITS* dest = &image;
    astrophototoolbox::FITS output;

    if (!outputFileName.empty())
    {
        if (std::ifstream(outputFileName).good())
            output.open(outputFileName, false);
        else
            output.create(outputFileName);

        dest = &output;
    }

    if (!dest->write(astrometry.getStarList(), "STARS", true))
    {
        cerr << "Failed to save the coordinates in the FITS file" << endl;
        return 1;
    }

    return 0;
}
