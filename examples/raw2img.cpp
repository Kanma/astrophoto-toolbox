#include <SimpleOpt.h>
#include <iostream>
#include <string>

#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/images/bitmap.h>
#include <astrophoto-toolbox/images/pnm.h>

using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_INFO,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP, "-h",     SO_NONE },
    { OPT_HELP, "--help", SO_NONE },
    { OPT_INFO, "-i",     SO_NONE },
    { OPT_INFO, "--info", SO_NONE },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "raw2img" << endl
         << "Usage: " << strApplicationName << "<input> <output>" << endl
         << endl
         << "This program convert a RAW image file into a PPM or PGM one." << endl
         << endl;
}


int main(int argc, char** argv)
{
    astrophototoolbox::RawImage image;
    bool displayInfo = false;

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
        bitmap = new astrophototoolbox::UInt8ColorBitmap();

    if (!image.toBitmap(bitmap))
    {
        cerr << "Failed to convert the RAW image" << endl;
        delete bitmap;
        return 1;
    }

    // Save the PNM file
    if (!astrophototoolbox::pnm::save(args.File(1), bitmap))
    {
        cerr << "Failed to save the PNM file '" << args.File(1) << "'" << endl;
        delete bitmap;
        return 1;
    }

    delete bitmap;

    return 0;
}
