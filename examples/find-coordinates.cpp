#include <SimpleOpt.h>
#include <iostream>
#include <fstream>
#include <string>

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/astrometry/astrometry.h>
#include <astrophoto-toolbox/images/raw.h>

using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_RAW,
    OPT_FITS,
    OPT_MIN_SCALE,
    OPT_MAX_SCALE,
    OPT_INDEXES_FOLDER,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",           SO_NONE },
    { OPT_HELP,             "--help",       SO_NONE },
    { OPT_VERBOSE,          "-v",           SO_NONE },
    { OPT_VERBOSE,          "--verbose",    SO_NONE },
    { OPT_RAW,              "--raw",        SO_NONE },
    { OPT_FITS,             "--fits",       SO_NONE },
    { OPT_MIN_SCALE,        "--min-scale",  SO_REQ_SEP },
    { OPT_MAX_SCALE,        "--max-scale",  SO_REQ_SEP },
    { OPT_INDEXES_FOLDER,   "--indexes",    SO_REQ_SEP },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "find-coordinates" << endl
         << "Usage: " << strApplicationName << "[options] <--fits | --raw> <image>" << endl
         << endl
         << "Determine the astronomical coordinates of a FITS or RAW image." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --fits            Indicates that the image is a FITS one" << endl
         << "    --raw             Indicates that the image is a RAW one" << endl
         << "    --min-scale       Minimum size (in degrees) of the image (default: 0.1)" << endl
         << "    --max-scale       Maximum size (in degrees) of the image (default: 180.0)" << endl
         << "    --indexes         Folder from which load the index files (default: /usr/local/astrometry/data)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    bool verbose = false;
    bool isRaw = false;
    bool isFits = false;
    std::string indexesFolder = "/usr/local/astrometry/data";
    double minScale = 0.1;
    double maxScale = 180.0;

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

                case OPT_MIN_SCALE:
                    minScale = stod(args.OptionArg());
                    break;

                case OPT_MAX_SCALE:
                    maxScale = stod(args.OptionArg());
                    break;

                case OPT_INDEXES_FOLDER:
                    indexesFolder = args.OptionArg();
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


    // Open the image
    astrophototoolbox::Bitmap* bitmap = nullptr;

    if (isFits)
    {
        // Open the FITS file
        astrophototoolbox::FITS image;
        if (!image.open(args.File(0), false))
        {
            cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
            return 1;
        }

        // Retrieve the bitmap
        bitmap = image.readBitmap();
        if (!bitmap)
        {
            cerr << "Failed to read the bitmap from the FITS file" << endl;
            return 1;
        }
    }
    else
    {
        // Open the RAW image
        astrophototoolbox::RawImage image;
        if (!image.open(args.File(0)))
        {
            cerr << "Failed to open the RAW file '" << args.File(0) << "'" << endl;
            return 1;
        }

        // Decode the RAW image
        if (image.channels() == 3)
            bitmap = new astrophototoolbox::UInt16ColorBitmap();
        else
            bitmap = new astrophototoolbox::UInt8ColorBitmap();

        if (!image.toBitmap(bitmap))
        {
            cerr << "Failed to convert the RAW image" << endl;
            delete bitmap;
            return 1;
        }
    }

    // Convert it to float & keep the first channel
    if (bitmap->channels() == 3)
    {
        auto converted = new astrophototoolbox::FloatColorBitmap(bitmap, astrophototoolbox::RANGE_BYTE);
        auto channel = converted->channel(0);
        delete bitmap;
        delete converted;
        bitmap = channel;
    }
    else
    {
        auto converted = new astrophototoolbox::FloatGrayBitmap(bitmap, astrophototoolbox::RANGE_BYTE);
        delete bitmap;
        bitmap = converted;
    }

    // Plate solving
    astrophototoolbox::Astrometry astrometry;
    if (!astrometry.loadIndexes(indexesFolder))
    {
        cerr << "Failed to load the index files from '" << indexesFolder << "'" << endl;
        delete bitmap;
        return 1;
    }
    
    if (!astrometry.run(bitmap, minScale, maxScale))
    {
        cerr << "Failed to determine the coordinates" << endl;
        delete bitmap;
        return 1;
    }

    delete bitmap;

    if (verbose)
    {
        cout << astrometry.getStarList().size() << " star(s) detected" << endl;
    }

    astrophototoolbox::Coordinates coordinates = astrometry.getCoordinates();

    cout << "Coordinates:" << endl;
    cout << "  * " << coordinates.getRADECasHMSDMS() << endl;
    cout << "  * " << coordinates.getRA() << "°, " << coordinates.getDEC() << "°" << endl;
    cout << endl;
    cout << "Pixel size: " << astrometry.getPixelSize() << " arcsec" << endl;
    cout << endl;

    return 0;
}
