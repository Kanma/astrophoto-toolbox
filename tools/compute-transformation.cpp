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

#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/data/star.h>
#include <astrophoto-toolbox/stacking/starmatcher.h>

using namespace std;
using namespace astrophototoolbox;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_MIN_DISTANCE,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,         "-h",               SO_NONE },
    { OPT_HELP,         "--help",           SO_NONE },
    { OPT_VERBOSE,      "-v",               SO_NONE },
    { OPT_VERBOSE,      "--verbose",        SO_NONE },
    { OPT_MIN_DISTANCE, "--min-distance",   SO_REQ_SEP },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "compute-translation" << endl
         << "Usage: " << strApplicationName << "[options] <starlist1> <starlist2>" << endl
         << endl
         << "Compute the translation between two FITS files containing approximately the same stars." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --min-distance    Minimal distance to consider between the two files (default: 0)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;
    double minDistance = 0.0;

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

                case OPT_MIN_DISTANCE:
                    minDistance = std::stod(args.OptionArg());
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return 1;
        }
    }

    if (args.FileCount() != 2)
    {
        cerr << "Require two input files" << endl;
        return 1;
    }


    // Retrieve the lists of stars from the input files
    FITS fits;

    if (!fits.open(args.File(0)))
    {
        cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
        return 1;
    }

    size2d_t imageSize;
    star_list_t sourceStars = fits.readStars(0, &imageSize);
    if (sourceStars.empty())
    {
        cerr << "Failed to retrieve a list of stars in the FITS file '" << args.File(0) << "'" << endl;
        return 1;
    }

    fits.close();

    if (!fits.open(args.File(1)))
    {
        cerr << "Failed to open the FITS file '" << args.File(1) << "'" << endl;
        return 1;
    }

    star_list_t targetStars = fits.readStars();
    if (targetStars.empty())
    {
        cerr << "Failed to retrieve a list of stars in the FITS file '" << args.File(1) << "'" << endl;
        return 1;
    }

    fits.close();

    // Compute the transformation
    Transformation transformation;
    stacking::StarMatcher matcher;
    if (!matcher.computeTransformation(sourceStars, targetStars, imageSize, transformation, minDistance))
    {
        cerr << "Failed to compute the transformation" << endl;
        return 1;
    }

    double dx, dy;
    transformation.offsets(dx, dy);

    double angle = transformation.angle(imageSize.width);

    cout << "Offset: (" << dx << ", " << dy << "), angle = " << angle << std::endl;

    return 0;
}
