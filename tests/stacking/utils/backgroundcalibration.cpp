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


TEST_CASE("BackgroundCalibration", "[BackgroundCalibration]")
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
    calibration.calibrate(&bitmap);

    for (unsigned int i = 0; i < ref.width() * ref.height() * 3; ++i)
        REQUIRE(data[i] == 100);
}
