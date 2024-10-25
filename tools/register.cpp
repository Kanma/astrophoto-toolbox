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
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/images/io.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/platesolving/platesolver.h>

using namespace std;
using namespace astrophototoolbox;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_OUTPUT,
    OPT_NO_AN_KEYWORDS,
    OPT_PLATESOLVING,
    OPT_UNIFORMIZE,
    OPT_OBJS,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",               SO_NONE },
    { OPT_HELP,             "--help",           SO_NONE },
    { OPT_VERBOSE,          "-v",               SO_NONE },
    { OPT_VERBOSE,          "--verbose",        SO_NONE },
    { OPT_OUTPUT,           "-o",               SO_REQ_SEP },
    { OPT_NO_AN_KEYWORDS,   "--no-an-keywords", SO_NONE },
    { OPT_PLATESOLVING,     "--platesolving",   SO_NONE },
    { OPT_UNIFORMIZE,       "-u",               SO_NONE },
    { OPT_UNIFORMIZE,       "--uniformize",     SO_NONE },
    { OPT_OBJS,             "--objs",           SO_REQ_SEP },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "register" << endl
         << "Usage: " << strApplicationName << " [options] <image>" << endl
         << endl
         << "Detect the stars in an image." << endl
         << endl
         << "By default, stars are detected using an algorithm tailored for image stacking." << endl
         << "Another detection algorithm is available, adapted to plate solving. It can be selected" << endl
         << "with the --platesolving option." << endl
         << endl

         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    -o FILE           FITS file into which write the coordinates (default: the input FITS" << endl
         << "                      image if applicable)" << endl
         << "    --no-an-keywords  Do not write astrometry.net specific keywords in the file (included by" << endl
         << "                      default, for compatibility)" << endl
         << "    --platesolving    Use the plate solving detection algorithm" << endl
         << endl
         << "When --platesolving is used:" << endl
         << "    --uniformize, -u  Uniformize the coordinates" << endl
         << "    -objs NB          Only keep the NB brightest objects" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;
    bool includeANKeywords = true;
    bool usePlateSolvingDetection = false;
    bool uniformize = false;
    int nbObjs = -1;

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

                case OPT_NO_AN_KEYWORDS:
                    includeANKeywords = false;
                    break;

                case OPT_PLATESOLVING:
                    usePlateSolvingDetection = true;
                    break;

                case OPT_UNIFORMIZE:
                    uniformize = true;
                    break;

                case OPT_OBJS:
                    nbObjs = stoi(args.OptionArg());
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

    bool isFits = FITS::isFITS(args.File(0));
    if (!isFits && outputFileName.empty())
    {
        cerr << "Must specify an output file with -o when not processing a FITS file" << endl;
        return 1;
    }


    // Open the image
    Bitmap* bitmap = io::load(args.File(0));
    if (!bitmap)
    {
        cerr << "Failed to load an image from file '" << args.File(0) << "'" << endl;
        return 1;
    }

    removeHotPixels(bitmap);


    // Registration
    star_list_t stars;
    size2d_t imageSize(bitmap->width(), bitmap->height());

    if (!usePlateSolvingDetection)
    {
        stacking::Registration registration;
        stars = registration.registerBitmap(bitmap);

        if (stars.empty())
        {
            cerr << "Failed to detect the stars" << endl;
            delete bitmap;
            return 1;
        }
    }
    else
    {
        // Convert the bitmap to float & keep the first channel
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

        // Star detection
        platesolving::PlateSolver solver;
        if (!solver.detectStars(bitmap))
        {
            cerr << "Failed to detect the stars" << endl;
            delete bitmap;
            return 1;
        }

        // If necessary, uniformize the coordinates
        if (uniformize)
        {
            if (!solver.uniformize())
            {
                cerr << "Failed to uniformize the coordinates" << endl;
                return 1;
            }
        }

        // If necessary, only keep the brightest objects
        if (nbObjs > 0)
            solver.cut(nbObjs);

        stars = solver.getStars();
    }

    delete bitmap;

    if (verbose)
    {
        cout << stars.size() << " star(s) detected, quality = " << computeQuality(stars)
             << ", FWHM = " << computeFWHM(stars) << endl;
    }


    // Save the stars in a FITS file
    FITS output;
    bool ok = false;

    if (!isFits)
    {
        if (std::filesystem::exists(outputFileName))
            ok = output.open(outputFileName, false);
        else
            ok = output.create(outputFileName);
    }
    else
    {
        ok = output.open(args.File(0), false);
    }

    if (!ok)
    {
        cerr << "Failed to open/create the FITS output file" << endl;
        return 1;
    }

    if (!output.write(stars, imageSize, "STARS", true))
    {
        cerr << "Failed to save the coordinates in the FITS file" << endl;
        return 1;
    }

    // Include the astrometry.net keywords if necessary
    if (includeANKeywords)
    {
        if (!output.writeAstrometryNetKeywords(imageSize))
        {
            cerr << "Failed to write the keywords specific to astrometry.net" << endl;
            return 1;
        }
    }

    return 0;
}
