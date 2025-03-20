/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <SimpleOpt.h>
#include <iostream>
#include <filesystem>

#include <astrophoto-toolbox/catalogs/dsocatalog.h>

using namespace std;
using namespace astrophototoolbox;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_CATALOG_FOLDER,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,             "-h",           SO_NONE },
    { OPT_HELP,             "--help",       SO_NONE },
    { OPT_VERBOSE,          "-v",           SO_NONE },
    { OPT_VERBOSE,          "--verbose",    SO_NONE },
    { OPT_CATALOG_FOLDER,   "--folder",     SO_REQ_SEP },
    
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "search-in-catalog" << endl
         << "Usage: " << strApplicationName << " [options] <pattern>" << endl
         << endl
         << "Search in the catalog of Deep-Space Objects for objects matching a pattern." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h        Display this help" << endl
         << "    --verbose, -v     Display more details" << endl
         << "    --folder          Folder from which load the catalog files (default: ./catalogs)" << endl
         << endl;
}


int main(int argc, char** argv)
{
    std::string outputFileName;
    bool verbose = false;
    std::filesystem::path catalogFolder;

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

                case OPT_CATALOG_FOLDER:
                    catalogFolder = args.OptionArg();
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
        cerr << "Require one pattern" << endl;
        return 1;
    }


    if (catalogFolder.empty())
    {
        std::filesystem::path path(argv[0]);
        path.remove_filename();
        catalogFolder = path / "catalogs";
    }


    // Load the catalogs
    DSOCatalog catalog;
    if (!catalog.load(catalogFolder))
    {
        cerr << "Failed to load the catalog from '" << catalogFolder << "'" << endl;
        return 1;
    }

    // Search using the pattern
    auto matches = catalog.search(args.File(0));

    if (matches.empty())
    {
        cout << "No match found" << std::endl;
        return 0;
    }

    for (auto it = matches.begin(); it != matches.end(); ++it)
        cout << it->name << ": " << it->coordinates.getRADECasHMSDMS() << std::endl;

    return 0;
}
