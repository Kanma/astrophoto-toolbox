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
    OPT_OUTPUT,
    OPT_UNIFORMIZE,
    OPT_OBJS,
    OPT_NO_AN_KEYWORDS,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",               SO_NONE },
    { OPT_HELP,             "--help",           SO_NONE },
    { OPT_VERBOSE,          "-v",               SO_NONE },
    { OPT_VERBOSE,          "--verbose",        SO_NONE },
    { OPT_RAW,              "--raw",            SO_NONE },
    { OPT_FITS,             "--fits",           SO_NONE },
    { OPT_OUTPUT,            "-o",              SO_REQ_SEP },
    { OPT_UNIFORMIZE,       "-u",               SO_NONE },
    { OPT_UNIFORMIZE,       "--uniformize",     SO_NONE },
    { OPT_OBJS,             "--objs",           SO_REQ_SEP },
    { OPT_NO_AN_KEYWORDS,   "--no-an-keywords", SO_NONE },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "detect-stars" << endl
         << "Usage: " << strApplicationName << "[options] <--fits | --raw> <image>" << endl
         << endl
         << "Detect the stars in a FITS or RAW image." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --fits            Indicates that the image is a FITS one" << endl
         << "    --raw             Indicates that the image is a RAW one" << endl
         << "    -o FILE           FITS file into which write the coordinates (default: the input FITS image if applicable)" << endl
         << "    --uniformize, -u  Uniformize the coordinates" << endl
         << "    -objs NB          Only keep the NB brightest objects" << endl
         << "    --no-an-keywords  Do not write astrometry.net specific keywords in the file (included by default, for compatibilty)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;
    bool isRaw = false;
    bool isFits = false;
    bool uniformize = false;
    int nbObjs = -1;
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

                case OPT_UNIFORMIZE:
                    uniformize = true;
                    break;

                case OPT_OBJS:
                    nbObjs = stoi(args.OptionArg());
                    break;

                case OPT_NO_AN_KEYWORDS:
                    includeANKeywords = false;
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
    astrophototoolbox::Bitmap* bitmap = nullptr;
    astrophototoolbox::FITS fitsImage;

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
        cout << astrometry.getStarList().size() << " star(s) detected" << endl;
    }

    // If necessary, uniformize the coordinates
    if (uniformize)
    {
        if (!astrometry.uniformize())
        {
            cerr << "Failed to uniformize the coordinates" << endl;
            return 1;
        }
    }

    // If necessary, only keep the brightest objects
    if (nbObjs > 0)
        astrometry.cut(nbObjs);

    // Save the coordinates
    astrophototoolbox::FITS* dest = (isFits ? &fitsImage : nullptr);
    astrophototoolbox::FITS output;

    if (!outputFileName.empty())
    {
        if (std::ifstream(outputFileName).good())
            output.open(outputFileName, false);
        else
            output.create(outputFileName);

        dest = &output;
    }

    astrophototoolbox::star_detection_info_t info = astrometry.getDetectionInfo();
    if (!dest->write(astrometry.getStarList(), info, "STARS", true))
    {
        cerr << "Failed to save the coordinates in the FITS file" << endl;
        return 1;
    }

    if (includeANKeywords)
    {
        if (!dest->writeAstrometryNetKeywords(info.imageWidth, info.imageHeight))
        {
            cerr << "Failed to write the keywords specific to astrometry.net" << endl;
            return 1;
        }
    }

    return 0;
}
