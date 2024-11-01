/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <SimpleOpt.h>
#include <iostream>
#include <string>

#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/images/io.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/stacking/utils/backgroundcalibration.h>

using namespace std;
using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::utils;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_WB,
    OPT_SRGB,
    OPT_REMOVE_HOT_PIXELS,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,                 "-h",                   SO_NONE },
    { OPT_HELP,                 "--help",               SO_NONE },
    { OPT_WB,                   "--wb",                 SO_NONE },
    { OPT_SRGB,                 "--srgb",               SO_NONE },
    { OPT_REMOVE_HOT_PIXELS,    "--remove-hot-pixels",  SO_NONE },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "raw2img" << endl
         << "Usage: " << strApplicationName << " [options] <reference> <image> <output>" << endl
         << endl
         << "Perform background calibration on an image, given a reference image." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h             Display this help" << endl
         << "    --remove-hot-pixels    Remove the hot pixels" << endl
         << endl
         << "For RAW images:" << endl
         << "    --wb                   Use camera white balance" << endl
         << "    --srgb                 Apply sRGB gamma correction" << endl
         << endl;
}


int main(int argc, char** argv)
{
    bool useCameraWB = false;
    bool sRGB = false;
    bool mustRemoveHotPixels = false;

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

                case OPT_WB:
                    useCameraWB = true;
                    break;

                case OPT_SRGB:
                    sRGB = true;
                    break;

                case OPT_REMOVE_HOT_PIXELS:
                    mustRemoveHotPixels = true;
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return 1;
        }
    }

    if (args.FileCount() != 3)
    {
        cerr << "Require a reference, an input and output file" << endl;
        return 1;
    }


    // Open the reference and input images
    Bitmap* reference = io::load(args.File(0), useCameraWB, !sRGB);
    if (!reference)
    {
        cerr << "Failed to load an image from file '" << args.File(0) << "'" << endl;
        return 1;
    }

    Bitmap* bitmap = io::load(args.File(1), useCameraWB, !sRGB);
    if (!bitmap)
    {
        cerr << "Failed to load an image from file '" << args.File(1) << "'" << endl;
        delete reference;
        return 1;
    }


    // If necessary: remove the hot pixels
    if (mustRemoveHotPixels)
    {
        removeHotPixels(reference);
        removeHotPixels(bitmap);
    }


    // Perform background calibration
    DoubleColorBitmap* reference2 = requiresFormat<DoubleColorBitmap>(
        reference, RANGE_ONE, SPACE_SOURCE
    );
    if (reference2 != reference)
        delete reference;

    DoubleColorBitmap* bitmap2 = requiresFormat<DoubleColorBitmap>(
        bitmap, RANGE_ONE, SPACE_SOURCE
    );
    if (bitmap2 != bitmap)
        delete bitmap;

    BackgroundCalibration<DoubleColorBitmap> calibration;
    calibration.setReference(reference2);
    calibration.calibrate(bitmap2);


    // Save the image
    if (!astrophototoolbox::io::save(args.File(2), bitmap2, true))
    {
        cerr << "Failed to save the file '" << args.File(2) << "'" << endl;
        delete reference2;
        delete bitmap2;
        return 1;
    }

    delete reference2;
    delete bitmap2;

    return 0;
}
