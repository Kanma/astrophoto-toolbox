/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/catalogs/dsocatalog.h>

using namespace astrophototoolbox;


TEST_CASE("Load from invalid catalog folder", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(!catalog.load("unknown"));
}


TEST_CASE("Load from valid catalog folder", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(catalog.load(CATALOGS_FOLDER));
}


TEST_CASE("Search for a specific object", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(catalog.load(CATALOGS_FOLDER));

    for (auto name : { "M51", "M051", "M00051", "M 51", "M   51" })
    {
        auto result = catalog.search(name);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].name == "M51");
        REQUIRE(result[0].coordinates.getRADECasHMSDMS() == "13:29:52.710, 47°11'42.600\"");
    }
}


TEST_CASE("Search for an unknown object", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(catalog.load(CATALOGS_FOLDER));

    auto result = catalog.search("UNK27");
    REQUIRE(result.size() == 0);
}


TEST_CASE("Search for a list of objects", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(catalog.load(CATALOGS_FOLDER));

    auto result = catalog.search("NGC19");
    REQUIRE(result.size() == 114);

    for (auto match : result)
        REQUIRE(match.name.starts_with("NGC19"));
}


TEST_CASE("Search for a limited list of objects", "[DSOCatalog]")
{
    DSOCatalog catalog;

    REQUIRE(catalog.load(CATALOGS_FOLDER));

    auto result = catalog.search("NGC19", 5);
    REQUIRE(result.size() == 5);

    for (auto match : result)
        REQUIRE(match.name.starts_with("NGC19"));
}
