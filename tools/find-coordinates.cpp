/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <SimpleOpt.h>
#include <iostream>
#include <fstream>
#include <string>

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/platesolving/platesolver.h>
#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/images/helpers.h>

using namespace std;
using namespace astrophototoolbox;
using namespace astrophototoolbox::platesolving;


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
         << "Usage: " << strApplicationName << " [options] <--fits | --raw> <file>" << endl
         << endl
         << "Determine the astronomical coordinates of a FITS or RAW file." << endl
         << endl
         << "If the FITS file contains a list of stars, it is used. Otherwise, the image in the FITS file is" << endl
         << "loaded and star detection is performed (like for a RAW image)." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --fits            Indicates that the file is a FITS one, containing either a star list or an image (or both)" << endl
         << "    --raw             Indicates that the file is a RAW image" << endl
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
    Bitmap* bitmap = nullptr;
    star_list_t stars;
    size2d_t imageSize;

    if (isFits)
    {
        // Open the FITS file
        FITS fits;
        if (!fits.open(args.File(0), false))
        {
            cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
            return 1;
        }

        // Attempt to retrieve stars from the FITS file
        stars = fits.readStars(0, &imageSize);

        // Retrieve the bitmap if no stars were found
        if (stars.empty())
        {
            bitmap = fits.readBitmap();
            if (!bitmap)
            {
                cerr << "Failed to read stars or a bitmap from the FITS file" << endl;
                return 1;
            }
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

        removeHotPixels(bitmap);
    }


    PlateSolver solver;

    // Process the bitmap if one was loaded
    if (bitmap)
    {
        // Convert it to float & keep the first channel
        if (bitmap->channels() == 3)
        {
            auto converted = new FloatColorBitmap(bitmap, RANGE_BYTE);
            auto channel = converted->channel(0);
            delete bitmap;
            delete converted;
            bitmap = channel;
        }
        else
        {
            auto converted = new FloatGrayBitmap(bitmap, RANGE_BYTE);
            delete bitmap;
            bitmap = converted;
        }

        // Plate solving
        if (!solver.loadIndexes(indexesFolder))
        {
            cerr << "Failed to load the index files from '" << indexesFolder << "'" << endl;
            delete bitmap;
            return 1;
        }

        if (!solver.run(bitmap, minScale, maxScale))
        {
            cerr << "Failed to determine the coordinates" << endl;
            delete bitmap;
            return 1;
        }

        delete bitmap;
    }
    else
    {
        // Plate solving
        if (!solver.loadIndexes(indexesFolder))
        {
            cerr << "Failed to load the index files from '" << indexesFolder << "'" << endl;
            delete bitmap;
            return 1;
        }

        solver.setStars(stars, imageSize);
        solver.uniformize();
        solver.cut();

        if (!solver.solve(minScale, maxScale))
        {
            cerr << "Failed to determine the coordinates" << endl;
            return 1;
        }
    }


    if (verbose)
    {
        cout << solver.getStars().size() << " star(s) detected" << endl;
    }

    Coordinates coordinates = solver.getCoordinates();

    cout << "Coordinates:" << endl;
    cout << "  * " << coordinates.getRADECasHMSDMS() << endl;
    cout << "  * " << coordinates.getRA() << "°, " << coordinates.getDEC() << "°" << endl;
    cout << endl;
    cout << "Pixel size: " << solver.getPixelSize() << " arcsec" << endl;
    cout << endl;

    return 0;
}
