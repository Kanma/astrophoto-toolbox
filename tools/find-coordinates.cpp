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
#include <astrophoto-toolbox/images/io.h>
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
    OPT_MIN_SCALE,
    OPT_MAX_SCALE,
    OPT_INDEXES_FOLDER,
    OPT_GRID_FILENAME,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",           SO_NONE },
    { OPT_HELP,             "--help",       SO_NONE },
    { OPT_VERBOSE,          "-v",           SO_NONE },
    { OPT_VERBOSE,          "--verbose",    SO_NONE },
    { OPT_MIN_SCALE,        "--min-scale",  SO_REQ_SEP },
    { OPT_MAX_SCALE,        "--max-scale",  SO_REQ_SEP },
    { OPT_INDEXES_FOLDER,   "--indexes",    SO_REQ_SEP },
    { OPT_GRID_FILENAME,    "--grid",       SO_REQ_SEP },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "find-coordinates" << endl
         << "Usage: " << strApplicationName << " [options] <file>" << endl
         << endl
         << "Determine the astronomical coordinates of an image." << endl
         << endl
         << "If the image file is a FITS one and it contains a list of stars, it is used." << endl
         << "Otherwise, the image in the FITS file is loaded and star detection is performed (like" << endl
         << "for any other image format)." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --min-scale       Minimum size (in degrees) of the image (default: 0.1)" << endl
         << "    --max-scale       Maximum size (in degrees) of the image (default: 180.0)" << endl
         << "    --indexes         Folder from which load the index files (default: /usr/local/astrometry/data)" << endl
         << "    --grid FILENAME   If specified, an image containing the celestial grid is saved at this location" << endl
         << endl;
}


int main(int argc, char** argv)
{
    bool verbose = false;
    double minScale = 0.1;
    double maxScale = 180.0;
    std::string indexesFolder = "/usr/local/astrometry/data";
    std::string outputFilename = "";

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

                case OPT_MIN_SCALE:
                    minScale = stod(args.OptionArg());
                    break;

                case OPT_MAX_SCALE:
                    maxScale = stod(args.OptionArg());
                    break;

                case OPT_INDEXES_FOLDER:
                    indexesFolder = args.OptionArg();
                    break;

                case OPT_GRID_FILENAME:
                    outputFilename = args.OptionArg();
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


    // Open the image
    Bitmap* bitmap = nullptr;
    star_list_t stars;
    size2d_t imageSize;

    if (FITS::isFITS(args.File(0)))
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

        // Retrieve the bitmap if no stars were found, or if we need to generate a grid image
        if (stars.empty() || !outputFilename.empty())
        {
            bitmap = fits.readBitmap();
            if (bitmap)
                removeHotPixels(bitmap);
        }

        if (stars.empty() && !bitmap)
        {
            cerr << "Failed to read stars or a bitmap from the FITS file" << endl;
            return 1;
        }
    }
    else
    {
        bitmap = io::load(args.File(0), true);
        if (!bitmap)
        {
            cerr << "Failed to load an image from file '" << args.File(0) << "'" << endl;
            return 1;
        }

        removeHotPixels(bitmap);
    }


    // Load the index files
    PlateSolver solver;

    if (!solver.loadIndexes(indexesFolder))
    {
        cerr << "Failed to load the index files from '" << indexesFolder << "'" << endl;
        delete bitmap;
        return 1;
    }


    // Process the bitmap if one was loaded
    if (!stars.empty())
    {
        // Plate solving
        solver.setStars(stars, imageSize);
        solver.uniformize();
        solver.cut();

        if (!solver.solve(minScale, maxScale))
        {
            cerr << "Failed to determine the coordinates" << endl;
            return 1;
        }
    }
    else
    {
        Bitmap* converted = nullptr;

        // Convert it to float & keep the first channel
        if (bitmap->channels() == 3)
        {
            converted = new FloatColorBitmap(bitmap, RANGE_BYTE);
            auto channel = converted->channel(0);
            delete converted;
            converted = channel;
        }
        else
        {
            converted = new FloatGrayBitmap(bitmap, RANGE_BYTE);
        }

        // Plate solving
        if (!solver.run(converted, minScale, maxScale))
        {
            cerr << "Failed to determine the coordinates" << endl;
            delete converted;
            delete bitmap;
            return 1;
        }

        delete converted;
    }


    // Display the result
    if (verbose)
    {
        cout << solver.getStars().size() << " star(s) detected" << endl;
    }

    Coordinates coordinates = solver.getCoordinates();

    cout << "Coordinates:" << endl;
    cout << "  * " << coordinates.getRADECasHMSDMS() << endl;
    cout << "  * " << coordinates.getRA() << "°, " << coordinates.getDEC() << "°" << endl;
    cout << endl;
    cout << "RA Orientation: " << solver.getRightAscensionOrientation() << "°" << endl;
    cout << "Pixel size: " << solver.getPixelSize() << " arcsec" << endl;
    cout << endl;


    // Generate the output image if necessary
    if (!outputFilename.empty() && bitmap)
    {
        UInt16ColorBitmap* output = requiresFormat<UInt16ColorBitmap>(bitmap, RANGE_DEST, SPACE_LINEAR);

        CoordinatesSystem system = solver.getCoordinatesSystem();
        system.drawGrid(output);
        system.drawAxes(output);

        io::save(outputFilename, output, true);

        if (output != bitmap)
            delete output;
    }

    if (bitmap)
        delete bitmap;

    return 0;
}
