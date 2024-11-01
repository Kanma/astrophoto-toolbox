/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/processing/stacking.h>
#include <fstream>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("(Stacking/Processing/Stacking) Fail to add missing light frame file", "[FramesStacker]")
{
    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_stacking"));

    {
        FramesStacker<UInt16ColorBitmap> stacker;
        stacker.setup(3, TEMP_DIR "tmp_stacking", 50000000);

        REQUIRE(!stacker.addFrame(DATA_DIR "missing.fits"));
    }

    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_stacking"));
}


TEST_CASE("(Stacking/Processing/Stacking) Stacking of 3 light frames", "[FramesStacker]")
{
    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_stacking"));

    FramesStacker<UInt16ColorBitmap> stacker;
    stacker.setup(3, TEMP_DIR "tmp_stacking", 50000000);

    REQUIRE(stacker.addFrame(TEMP_DIR "lightframes/light1.fits"));
    REQUIRE(stacker.addFrame(TEMP_DIR "lightframes/light2.fits"));
    REQUIRE(stacker.addFrame(TEMP_DIR "lightframes/light3.fits"));

    REQUIRE(std::filesystem::exists(TEMP_DIR "tmp_stacking"));

    Bitmap* stacked = stacker.process(TEMP_DIR "stacked.fits");
    REQUIRE(stacked);
    REQUIRE(stacked->width() == 1222);
    REQUIRE(stacked->height() == 864);

    delete stacked;

    REQUIRE(std::filesystem::exists(TEMP_DIR "stacked.fits"));
}
