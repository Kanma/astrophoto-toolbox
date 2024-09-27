/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/catalogs/dsocatalog.h>
#include <fstream>
#include <regex>

using namespace astrophototoolbox;


/************************************** METHODS ****************************************/

bool DSOCatalog::load(const std::filesystem::path& folder)
{
    if (!std::filesystem::is_directory(folder))
        return false;

    loadOpenNGC(folder / "NGC.csv");
    loadOpenNGC(folder / "NGC_addendum.csv");

    return !entries.empty();
}

//-----------------------------------------------------------------------------

std::vector<DSOCatalog::match_t> DSOCatalog::search(
    const std::string& pattern, unsigned int limit
) const
{
    std::vector<DSOCatalog::match_t> result;

    static const std::regex regexSplit("^([a-zA-Z]+)(\\s*)([0]*).*");

    std::smatch match;
    bool found = std::regex_match(pattern, match, regexSplit);

    std::regex re;
    if (found)
    {
        std::string prefix = match[1];
        std::string suffix1 = match[2];
        std::string suffix2 = match[3];
        re = prefix + pattern.substr(prefix.size() + suffix1.size() + suffix2.size());
    }
    else
    {
        re = "^[a-zA-Z]*" + pattern;
    }

    for (const auto& entry : entries)
    {
        if (std::regex_search(entry.name, re))
            result.push_back(match_t{ entry.name, { entry.ra, entry.dec } });

        if (std::regex_search(entry.messier, re))
            result.push_back(match_t{ entry.messier, { entry.ra, entry.dec } });

        if ((limit > 0) && (result.size() == limit))
            break;
    }

    return result;
}


/********************************* INTERNAL METHODS ************************************/

bool DSOCatalog::loadOpenNGC(const std::filesystem::path& filename)
{
    if (!std::filesystem::exists(filename))
        return false;

    std::ifstream file;
    file.open(filename.c_str());

    if (!file.is_open())
        return false;

    std::string line;

    int nameIndex = -1;
    int raIndex = -1;
    int decIndex = -1;
    int messierIndex = -1;
    int maxIndex = -1;

    static const std::regex regexSplit("^([a-zA-Z]+)([0]*).*");
    std::smatch match;

    while (std::getline(file, line))
    {
        // Just in case
        if (line.empty())
            continue;

        std::vector<std::string> parts = splitCSVLine(line);

        if (nameIndex != -1)
        {
            entry_t entry;
            entry.name = parts[nameIndex];
            entry.ra = parts[raIndex];
            entry.dec = parts[decIndex];
            entry.messier = parts[messierIndex];

            if (std::regex_match(entry.name, match, regexSplit))
            {
                std::string prefix = match[1];
                std::string zeroes = match[2];
                entry.name = prefix + entry.name.substr(prefix.size() + zeroes.size());
            }

            if (entry.dec.starts_with("+"))
                entry.dec = entry.dec.substr(1);

            if (!entry.messier.empty())
            {
                entry.messier = "M" + entry.messier;

                if (std::regex_match(entry.messier, match, regexSplit))
                {
                    std::string prefix = match[1];
                    std::string zeroes = match[2];
                    entry.messier = prefix + entry.messier.substr(prefix.size() + zeroes.size());
                }
            }

            if (!entry.ra.empty() && !entry.dec.empty())
                entries.push_back(entry);
        }
        else
        {
            nameIndex = std::find(parts.begin(), parts.end(), "Name") - parts.begin();
            raIndex = std::find(parts.begin(), parts.end(), "RA") - parts.begin();
            decIndex = std::find(parts.begin(), parts.end(), "Dec") - parts.begin();
            messierIndex = std::find(parts.begin(), parts.end(), "M") - parts.begin();
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

std::vector<std::string> DSOCatalog::splitCSVLine(const std::string& line) const
{
    std::vector<std::string> result;

    size_t start = 0;

    while (true)
    {
        size_t end = line.find(';', start);
        if (end == -1)
            end = line.size();

        std::string value = line.substr(start, end - start);
        result.push_back(value);

        start = end + 1;
        if (start >= line.size())
            break;
    }

    return result;
}
