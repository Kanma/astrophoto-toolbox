/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/processing/lightframes.h>
#include <astrophoto-toolbox/data/fits.h>
#include <fstream>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("Fail to use missing master dark frame file", "[LightFrames]")
{
    LightFrameProcessor<UInt16ColorBitmap> processor;
    REQUIRE(!processor.setMasterDark(DATA_DIR "missing.fits"));
}


TEST_CASE("Fail to process missing light frame file", "[LightFrames]")
{
    LightFrameProcessor<UInt16ColorBitmap> processor;
    REQUIRE(!processor.process(DATA_DIR "missing.fits"));
}


TEST_CASE("Process reference light frame without a master dark", "[LightFrames]")
{
    LightFrameProcessor<UInt16ColorBitmap> processor;

    std::shared_ptr<UInt16ColorBitmap> lightFrame = processor.process(
        DATA_DIR "downloads/light1.fits", true, TEMP_DIR "light1_processed.fits"
    );

    REQUIRE(lightFrame);
    REQUIRE(lightFrame->width() == 1300);
    REQUIRE(lightFrame->height() == 960);

    FITS fits;
    REQUIRE(fits.open(TEMP_DIR "light1_processed.fits"));

    Bitmap* bitmap = fits.readBitmap();
    REQUIRE(bitmap);

    UInt16ColorBitmap* converted = dynamic_cast<UInt16ColorBitmap*>(bitmap);
    REQUIRE(converted);
    REQUIRE(converted->width() == lightFrame->width());
    REQUIRE(converted->height() == lightFrame->height());

    for (unsigned int i = 0; i < lightFrame->width() * lightFrame->height() * 3; ++i)
        REQUIRE(converted->data()[i] == lightFrame->data()[i]);

    delete bitmap;
}


TEST_CASE("Process light frames", "[LightFrames]")
{
    REQUIRE(std::filesystem::create_directory(TEMP_DIR "lightframes"));

    LightFrameProcessor<UInt16ColorBitmap> processor;

    processor.setMasterDark(TEMP_DIR "master_dark.fits");

    std::shared_ptr<UInt16ColorBitmap> lightFrame = processor.process(
        DATA_DIR "downloads/light1.fits", true, TEMP_DIR "lightframes/light1.fits"
    );
    REQUIRE(lightFrame);

    lightFrame = processor.process(
        DATA_DIR "downloads/light2.fits", false, TEMP_DIR "lightframes/light2.fits"
    );
    REQUIRE(lightFrame);

    lightFrame = processor.process(
        DATA_DIR "downloads/light3.fits", false, TEMP_DIR "lightframes/light3.fits"
    );
    REQUIRE(lightFrame);
}
