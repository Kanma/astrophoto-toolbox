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

#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/images/pnm.h>
#include <astrophoto-toolbox/images/helpers.h>
#include <astrophoto-toolbox/data/fits.h>
#include <astrophoto-toolbox/stacking/backgroundcalibration.h>

using namespace std;
using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_WB,
    OPT_SRGB,
    OPT_REMOVE_HOT_PIXELS,
    OPT_RAW,
    OPT_FITS,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,                 "-h",                   SO_NONE },
    { OPT_HELP,                 "--help",               SO_NONE },
    { OPT_WB,                   "--wb",                 SO_NONE },
    { OPT_SRGB,                 "--srgb",               SO_NONE },
    { OPT_REMOVE_HOT_PIXELS,    "--remove-hot-pixels",  SO_NONE },
    { OPT_RAW,                  "--raw",                SO_NONE },
    { OPT_FITS,                 "--fits",               SO_NONE },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "raw2img" << endl
         << "Usage: " << strApplicationName << " [options] <--fits | --raw> <reference> <image> <output>" << endl
         << endl
         << "This program convert a RAW image file into a FITS, PPM or PGM one." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h             Display this help" << endl
         << "    --remove-hot-pixels    Remove the hot pixels" << endl
         << "    --fits                 Indicates that the image files are FITS ones" << endl
         << "    --raw                  Indicates that the image files are RAW images" << endl
         << endl
         << "For RAW images:" << endl
         << "    --wb                   Use camera white balance" << endl
         << "    --srgb                 Apply sRGB gamma correction" << endl
         << endl;
}


int main(int argc, char** argv)
{
    astrophototoolbox::RawImage image;
    bool useCameraWB = false;
    bool sRGB = false;
    bool mustRemoveHotPixels = false;
    bool isRaw = false;
    bool isFits = false;

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

                case OPT_RAW:
                    isRaw = true;
                    break;

                case OPT_FITS:
                    isFits = true;
                    break;

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

    // Open the reference and input images
    Bitmap* reference = nullptr;
    Bitmap* bitmap = nullptr;

    if (isFits)
    {
        // Open the FITS files
        {
            FITS fits;
            if (!fits.open(args.File(0), false))
            {
                cerr << "Failed to open the FITS file '" << args.File(0) << "'" << endl;
                return 1;
            }

            reference = fits.readBitmap();
            if (!reference)
            {
                cerr << "Failed to read a bitmap from the FITS file '" << args.File(0) << "'" << endl;
                return 1;
            }
        }

        {
            FITS fits;
            if (!fits.open(args.File(1), false))
            {
                cerr << "Failed to open the FITS file '" << args.File(1) << "'" << endl;
                delete reference;
                return 1;
            }

            bitmap = fits.readBitmap();
            if (!bitmap)
            {
                cerr << "Failed to read a bitmap from the FITS file '" << args.File(1) << "'" << endl;
                delete reference;
                return 1;
            }
        }
    }
    else
    {
        // Open the RAW images
        {
            RawImage image;
            if (!image.open(args.File(0)))
            {
                cerr << "Failed to open the RAW file '" << args.File(0) << "'" << endl;
                return 1;
            }

            // Decode the RAW image
            if (image.channels() == 3)
                reference = new UInt16ColorBitmap();
            else
                reference = new UInt8ColorBitmap();

            if (!image.toBitmap(reference, useCameraWB, !sRGB))
            {
                cerr << "Failed to convert the RAW image '" << args.File(0) << "'" << endl;
                delete reference;
                return 1;
            }
        }

        {
            RawImage image;
            if (!image.open(args.File(1)))
            {
                cerr << "Failed to open the RAW file '" << args.File(1) << "'" << endl;
                delete reference;
                return 1;
            }

            // Decode the RAW image
            if (image.channels() == 3)
                bitmap = new UInt16ColorBitmap();
            else
                bitmap = new UInt8ColorBitmap();

            if (!image.toBitmap(bitmap, useCameraWB, !sRGB))
            {
                cerr << "Failed to convert the RAW image '" << args.File(1) << "'" << endl;
                delete reference;
                delete bitmap;
                return 1;
            }
        }
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
    std::string outputFilename(args.File(2));
    if (outputFilename.ends_with(".fits"))
    {
        // Save the FITS file
        astrophototoolbox::FITS output;
        if (std::ifstream(args.File(2)).good())
        {
            cerr << "The file '" << args.File(2) << "' already exists, can't overwrite it" << endl;
            delete reference2;
            delete bitmap2;
            return 1;
        }

        if (!output.create(args.File(2)))
        {
            cerr << "Failed to create the FITS file '" << args.File(2) << "'" << endl;
            delete reference2;
            delete bitmap2;
            return 1;
        }

        if (!output.write(bitmap2))
        {
            cerr << "Failed to add the image into the FITS file" << endl;
            output.close();
            delete reference2;
            delete bitmap2;
            return 1;
        }

        output.close();
    }
    else if (outputFilename.ends_with(".ppm") || outputFilename.ends_with(".pgm"))
    {
        // Save the PNM file
        if (!astrophototoolbox::pnm::save(args.File(2), bitmap2))
        {
            cerr << "Failed to save the PNM file '" << args.File(2) << "'" << endl;
            delete reference2;
            delete bitmap2;
            return 1;
        }
    }
    else
    {
        cerr << "Unknown file format: '" << args.File(2) << "'" << endl;
        delete reference2;
        delete bitmap2;
        return 1;
    }

    delete reference2;
    delete bitmap2;

    return 0;
}
