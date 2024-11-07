/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/utils/backgroundcalibration.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::utils;


TEST_CASE("Background calibration with bitmap as reference", "[BackgroundCalibration]")
{
    UInt16ColorBitmap ref(10, 3);
    UInt16ColorBitmap bitmap(10, 3);

    uint16_t* refData = ref.data();
    uint16_t* data = bitmap.data();
    for (unsigned int i = 0; i < ref.width() * ref.height() * 3; ++i)
    {
        refData[i] = 100;
        data[i] = 200 + (i % 3) * 10;
    }

    BackgroundCalibration<UInt16ColorBitmap> calibration;
    calibration.setReference(&ref);

    auto parameters = calibration.getParameters();
    REQUIRE(parameters.redBackground == Approx(101.0));
    REQUIRE(parameters.greenBackground == Approx(101.0));
    REQUIRE(parameters.blueBackground == Approx(101.0));
    REQUIRE(parameters.redMax == Approx(100.0));
    REQUIRE(parameters.greenMax == Approx(100.0));
    REQUIRE(parameters.blueMax == Approx(100.0));

    calibration.calibrate(&bitmap);

    for (unsigned int i = 0; i < ref.width() * ref.height() * 3; ++i)
        REQUIRE(data[i] == 100);
}


TEST_CASE("Background calibration with parameters", "[BackgroundCalibration]")
{
    UInt16ColorBitmap bitmap(10, 3);

    uint16_t* data = bitmap.data();
    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * 3; ++i)
        data[i] = 200 + (i % 3) * 10;

    background_calibration_parameters_t parameters;
    parameters.redBackground = 101.0;
    parameters.greenBackground = 101.0;
    parameters.blueBackground = 101.0;
    parameters.redMax = 100.0;
    parameters.greenMax = 100.0;
    parameters.blueMax = 100.0;

    BackgroundCalibration<UInt16ColorBitmap> calibration;
    calibration.setParameters(parameters);

    calibration.calibrate(&bitmap);

    for (unsigned int i = 0; i < bitmap.width() * bitmap.height() * 3; ++i)
        REQUIRE(data[i] == 100);
}
