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
#include <astrophoto-toolbox/images/io.h>
#include <astrophoto-toolbox/images/helpers.h>

using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_INFO,
    OPT_RECT,
    OPT_WB,
    OPT_SRGB,
    OPT_REMOVE_HOT_PIXELS,
    OPT_GRAY,
    OPT_LUMINANCE,
    OPT_CHANNEL,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,                 "-h",                   SO_NONE },
    { OPT_HELP,                 "--help",               SO_NONE },
    { OPT_INFO,                 "-i",                   SO_NONE },
    { OPT_INFO,                 "--info",               SO_NONE },
    { OPT_RECT,                 "--rect",               SO_MULTI },
    { OPT_WB,                   "--wb",                 SO_NONE },
    { OPT_SRGB,                 "--srgb",               SO_NONE },
    { OPT_REMOVE_HOT_PIXELS,    "--remove-hot-pixels",  SO_NONE },
    { OPT_GRAY,                 "--gray",               SO_NONE },
    { OPT_LUMINANCE,            "--luminance",          SO_NONE },
    { OPT_CHANNEL,              "--channel",            SO_REQ_SEP },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "raw2img" << endl
         << "Usage: " << strApplicationName << " [options] <input> <output>" << endl
         << endl
         << "This program convert a RAW image file into a FITS, PPM, PGM, PNG, BMP, JPG, TGA or HDR one." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h             Display this help" << endl
         << "    --info, -i             Display informations about the image" << endl
         << "    --rect X Y W H         Extract the portion of the image defined by this rectangle" << endl
         << "    --wb                   Use camera white balance" << endl
         << "    --srgb                 Apply sRGB gamma correction" << endl
         << "    --remove-hot-pixels    Remove the hot pixels" << endl
         << "    --gray                 Convert the image to grayscale" << endl
         << "    --luminance            Extract the luminance of the image" << endl
         << "    --channel INDEX        Only save the given channel (0-2)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    astrophototoolbox::RawImage image;
    bool displayInfo = false;
    bool useRect = false;
    unsigned int rect_x;
    unsigned int rect_y;
    unsigned int rect_w;
    unsigned int rect_h;
    bool useCameraWB = false;
    bool sRGB = false;
    bool mustRemoveHotPixels = false;
    bool gray = false;
    bool luminance = false;
    int channel = -1;

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

                case OPT_INFO:
                    displayInfo = true;
                    break;

                case OPT_RECT:
                {
                    char** rect = args.MultiArg(4);
                    if (!rect)
                    {
                        cerr << "Incorrect number of values for argument: " << args.OptionText() << endl;
                        return 1;
                    }

                    useRect = true;
                    rect_x = stoi(rect[0]);
                    rect_y = stoi(rect[1]);
                    rect_w = stoi(rect[2]);
                    rect_h = stoi(rect[3]);
                    break;
                }

                case OPT_WB:
                    useCameraWB = true;
                    break;

                case OPT_SRGB:
                    sRGB = true;
                    break;

                case OPT_REMOVE_HOT_PIXELS:
                    mustRemoveHotPixels = true;
                    break;

                case OPT_GRAY:
                    gray = true;
                    break;

                case OPT_LUMINANCE:
                    luminance = true;
                    break;

                case OPT_CHANNEL:
                    channel = stoi(args.OptionArg());
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
        cerr << "Require an input and output file" << endl;
        return 1;
    }

    // Open the RAW file
    if (!image.open(args.File(0)))
    {
        cerr << "Failed to open the RAW file '" << args.File(0) << "'" << endl;
        return 1;
    }

    // Display the informations (if necessary)
    if (displayInfo)
    {
        cout << "Size:          " << image.width() << "x" << image.height() << endl;
        cout << "Channels:      " << (unsigned int) image.channels() << endl;
        cout << "ISO:           " << image.isoSpeed() << endl;
        cout << "Shutter speed: " << image.shutterSpeed() << endl;
        cout << "Aperture:      " << image.aperture() << endl;
        cout << "Focal length:  " << image.focalLength() << endl;
    }

    // Decode the RAW image
    astrophototoolbox::Bitmap* bitmap = nullptr;
    if (image.channels() == 3)
        bitmap = new astrophototoolbox::UInt16ColorBitmap();
    else
        bitmap = new astrophototoolbox::UInt16GrayBitmap();

    if (!image.toBitmap(bitmap, useCameraWB, !sRGB))
    {
        cerr << "Failed to convert the RAW image" << endl;
        delete bitmap;
        return 1;
    }

    // If necessary: keep only a portion of the image
    if (useRect)
    {
        rect_x = min(rect_x, bitmap->width() - 1);
        rect_y = min(rect_y, bitmap->height() - 1);
        rect_w = min(rect_w, bitmap->width() - rect_x);
        rect_h = min(rect_h, bitmap->height() - rect_y);

        astrophototoolbox::Bitmap* portion = nullptr;
        if (image.channels() == 3)
            portion = new astrophototoolbox::UInt16ColorBitmap(rect_w, rect_h);
        else
            portion = new astrophototoolbox::UInt16GrayBitmap(rect_w, rect_h);

        for (unsigned int y = 0; y < rect_h; ++y)
        {
            uint8_t* src = bitmap->ptr(rect_x, rect_y + y);
            uint8_t* dest = portion->ptr(y);

            memcpy(dest, src, rect_w * bitmap->bytesPerPixel());
        }

        delete bitmap;
        bitmap = portion;
    }

    // If necessary: remove the hot pixels
    if (mustRemoveHotPixels)
        removeHotPixels(bitmap);

    // If necessary: compute the luminance of the image
    if (luminance)
    {
        astrophototoolbox::DoubleGrayBitmap* luminance = computeLuminanceBitmap(bitmap);
        delete bitmap;
        bitmap = luminance;
    }

    // If necessary: convert the image to grayscale
    if (gray)
    {
        astrophototoolbox::Bitmap* grayscale = nullptr;

        if (bitmap->channels() == 3)
            grayscale = new astrophototoolbox::UInt16GrayBitmap(bitmap);
        else
            grayscale = new astrophototoolbox::UInt8GrayBitmap(bitmap);

        delete bitmap;
        bitmap = grayscale;
    }

    // If necessary: only keep one channel
    if ((channel >= 0) && (channel < bitmap->channels()))
    {
        astrophototoolbox::Bitmap* channelBitmap = bitmap->channel(channel);
        delete bitmap;
        bitmap = channelBitmap;
    }

    // Save the image
    if (!astrophototoolbox::io::save(args.File(1), bitmap, true))
    {
        cerr << "Failed to save the file '" << args.File(1) << "'" << endl;
        delete bitmap;
        return 1;
    }

    delete bitmap;

    return 0;
}
