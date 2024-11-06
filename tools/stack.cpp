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

#include <astrophoto-toolbox/stacking/stacking.h>
#include <astrophoto-toolbox/images/io.h>

using namespace std;
using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,     "-h",           SO_NONE },
    { OPT_HELP,     "--help",       SO_NONE },
    { OPT_VERBOSE,  "-v",           SO_NONE },
    { OPT_VERBOSE,  "--verbose",    SO_NONE },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "stack" << endl
         << "Usage: " << strApplicationName << " [options] <folder> <output>" << endl
         << endl
         << "This program performs stacking of images, according the the instructions in" << endl
         << "the 'stacking.txt' file found in the specified folder." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h     Display this help" << endl
         << "    --verbose, -v  Display details" << endl
         << endl;
}


int main(int argc, char** argv)
{
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
        cerr << "Require a folder and an output file name" << endl;
        return 1;
    }

    std::filesystem::path folder(args.File(0));
    if (!std::filesystem::exists(folder / "stacking.txt"))
    {
        cerr << "File '" << (folder / "stacking.txt") << "' not found" << endl;
        return 1;
    }


    // Stack the images
    Stacking<UInt16ColorBitmap> stacking;
    stacking.setup(folder);

    if (!stacking.load())
    {
        cerr << "Failed to load the file '" << (folder / "stacking.txt") << "'" << endl;
        return 1;
    }

    if (verbose)
    {
        if (stacking.nbDarkFrames() != 0)
            cout << stacking.nbDarkFrames() << " dark frames" << endl;
        else
            cout << "No dark frame" << endl;

        if (stacking.nbLightFrames() != 0)
            cout << stacking.nbLightFrames() << " light frames" << endl;
        else
            cout << "No light frame" << endl;
    }

    if (stacking.nbLightFrames() == 0)
        return 0;


    if (verbose)
        cout << "Stacking..." << endl;

    UInt16ColorBitmap* bitmap = stacking.process();
    if (!bitmap)
    {
        cerr << "Failed to stack the images" << endl;
        return 1;
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
