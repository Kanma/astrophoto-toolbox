/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <astrophoto-toolbox/data/coordinates.h>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Catalog of Deep-Space Objects
    ///
    /// Data comes from the OpenNGC project (https://github.com/mattiaverga/OpenNGC)
    //------------------------------------------------------------------------------------
    class DSOCatalog
    {
        //_____ Types __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Result of the searches
        //--------------------------------------------------------------------------------
        struct match_t
        {
            std::string name;
            Coordinates coordinates;
        };


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Load the catalog files from the specified folder
        //--------------------------------------------------------------------------------
        bool load(const std::filesystem::path& folder);

        //--------------------------------------------------------------------------------
        /// @brief  Search in the catalog for objects with names corresponding to the
        ///         given pattern (ie. "NGC194" or "M 51")
        //--------------------------------------------------------------------------------
        std::vector<match_t> search(
            const std::string& pattern, unsigned int limit = 0
        ) const;


        //_____ Internal methods __________
    private:
        bool loadOpenNGC(const std::filesystem::path& filename);
        std::vector<std::string> splitCSVLine(const std::string& line) const;


        //_____ Internal types __________
    private:
        struct entry_t
        {
            std::string name;
            std::string ra;
            std::string dec;
            std::string messier;
        };


        //_____ Attributes __________
    private:
        std::vector<entry_t> entries;
    };

}
