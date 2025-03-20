/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/processing/masterdark.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::processing;


TEST_CASE("(Stacking/Processing/MasterDark) Fail to compute master dark frame without dark frames", "[MasterDark]")
{
    std::vector<std::filesystem::path> darkFrames = {
    };

    MasterDarkGenerator<UInt16ColorBitmap> generator;
    UInt16ColorBitmap* masterDark = generator.compute(
        darkFrames, TEMP_DIR "master_dark.fits", TEMP_DIR "tmp_masterdark"
    );

    REQUIRE(!masterDark);

    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_masterdark"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "master_dark.fits"));
}


TEST_CASE("(Stacking/Processing/MasterDark) Fail to compute master dark frame with missing dark frames", "[MasterDark]")
{
    std::vector<std::filesystem::path> darkFrames = {
        DATA_DIR "downloads/missing1.fits",
        DATA_DIR "downloads/missing2.fits",
        DATA_DIR "downloads/missing3.fits"
    };

    MasterDarkGenerator<UInt16ColorBitmap> generator;
    UInt16ColorBitmap* masterDark = generator.compute(
        darkFrames, TEMP_DIR "master_dark.fits", TEMP_DIR "tmp_masterdark"
    );

    REQUIRE(!masterDark);

    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_masterdark"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "master_dark.fits"));
}


TEST_CASE("(Stacking/Processing/MasterDark) Master dark frame computation", "[MasterDark]")
{
    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_masterdark"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "master_dark.fits"));

    std::vector<std::filesystem::path> darkFrames = {
        DATA_DIR "downloads/dark1.fits",
        DATA_DIR "downloads/dark2.fits",
        DATA_DIR "downloads/dark3.fits"
    };

    MasterDarkGenerator<UInt16ColorBitmap> generator;
    UInt16ColorBitmap* masterDark = generator.compute(
        darkFrames, TEMP_DIR "master_dark.fits", TEMP_DIR "tmp_masterdark"
    );

    REQUIRE(masterDark);
    REQUIRE(masterDark->width() == 1300);
    REQUIRE(masterDark->height() == 960);

    point_list_t hotPixelsRef = generator.getHotPixels();
    REQUIRE(hotPixelsRef.size() == 274);

    REQUIRE(!std::filesystem::exists(TEMP_DIR "tmp_masterdark"));

    std::filesystem::path path(TEMP_DIR "master_dark.fits");
    REQUIRE(std::filesystem::exists(path));

    FITS fits;
    REQUIRE(fits.open(path));

    Bitmap* bitmap = fits.readBitmap();
    REQUIRE(bitmap);

    UInt16ColorBitmap* converted = dynamic_cast<UInt16ColorBitmap*>(bitmap);
    REQUIRE(converted);
    REQUIRE(converted->width() == masterDark->width());
    REQUIRE(converted->height() == masterDark->height());

    for (unsigned int i = 0; i < masterDark->width() * masterDark->height() * 3; ++i)
        REQUIRE(converted->data()[i] == masterDark->data()[i]);

    delete bitmap;

    point_list_t hotPixels = fits.readPoints();
    REQUIRE(hotPixels.size() == hotPixelsRef.size());

    for (unsigned int i = 0; i < hotPixelsRef.size(); ++i)
    {
        REQUIRE(hotPixels[i].x == Approx(hotPixelsRef[i].x));
        REQUIRE(hotPixels[i].y == Approx(hotPixelsRef[i].y));
    }
}
