/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/bitmapstacker.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("Adding bitmaps populates temporary files", "[Stacker]")
{
    const unsigned int NB_IMAGES = 3;
    const unsigned int WIDTH = 10;
    const unsigned int HEIGHT = 3;

    BitmapStacker stacker;
    stacker.setup(NB_IMAGES, TEMP_DIR "stacking1", WIDTH * HEIGHT * NB_IMAGES * sizeof(double));

    DoubleColorBitmap bitmap1(WIDTH, HEIGHT);
    DoubleColorBitmap bitmap2(WIDTH, HEIGHT);
    DoubleColorBitmap bitmap3(WIDTH, HEIGHT);

    for (int i = 0; i < WIDTH * HEIGHT * 3; ++i)
    {
        *(bitmap1.data() + i) = i;
        *(bitmap2.data() + i) = (i + 100);
        *(bitmap3.data() + i) = (i + 200);
    }

    REQUIRE(stacker.addBitmap(&bitmap1));
    REQUIRE(stacker.addBitmap(&bitmap2));
    REQUIRE(stacker.addBitmap(&bitmap3));

    std::filesystem::path folder(TEMP_DIR "stacking1");
    REQUIRE(std::filesystem::exists(folder));

    const char* filenames[NB_IMAGES] = {
        "part0000.dat",
        "part0001.dat",
        "part0002.dat",
    };

    for (int j = 0; j < NB_IMAGES; ++j)
        REQUIRE(std::filesystem::exists(folder / filenames[j]));

    REQUIRE(!std::filesystem::exists(folder / "part0003.dat"));

    double buffer[WIDTH * NB_IMAGES * 3];

    for (int j = 0; j < NB_IMAGES; ++j)
    {
        FILE* f = std::fopen((folder / filenames[j]).c_str(), "rb");
        REQUIRE(f);

        fread(buffer, sizeof(double), WIDTH * NB_IMAGES * 3, f);
        fclose(f);

        double* ptr = buffer;
        double start = j * WIDTH * 3;

        for (int i = 0; i < WIDTH * 3; ++i)
        {
            REQUIRE(ptr[i] == Approx(start + i));
            REQUIRE(ptr[i + WIDTH * 3] == Approx(start + i + 100));
            REQUIRE(ptr[i + WIDTH * 3 * 2] == Approx(start + i + 200));
        }
    }

    stacker.clear();
    REQUIRE(!std::filesystem::exists(folder / "part0000.dat"));
}


TEST_CASE("Stacking bitmaps", "[Stacker]")
{
    const unsigned int NB_IMAGES = 3;
    const unsigned int WIDTH = 10;
    const unsigned int HEIGHT = 3;

    BitmapStacker stacker;

    SECTION("using small temporary files")
    {
        stacker.setup(NB_IMAGES, TEMP_DIR "stacking2", WIDTH * HEIGHT * NB_IMAGES * sizeof(double));
    }

    SECTION("using big temporary files")
    {
        stacker.setup(NB_IMAGES, TEMP_DIR "stacking3");
    }

    DoubleColorBitmap bitmap1(WIDTH, HEIGHT);
    DoubleColorBitmap bitmap2(WIDTH, HEIGHT);
    DoubleColorBitmap bitmap3(WIDTH, HEIGHT);

    for (int i = 0; i < WIDTH * HEIGHT * 3; ++i)
    {
        *(bitmap1.data() + i) = double(i) / 512.0;
        *(bitmap2.data() + i) = double(i + 100) / 512.0;
        *(bitmap3.data() + i) = double(i + 200) / 512.0;
    }

    REQUIRE(stacker.addBitmap(&bitmap1));
    REQUIRE(stacker.addBitmap(&bitmap2));
    REQUIRE(stacker.addBitmap(&bitmap3));

    DoubleColorBitmap* stacked = stacker.process();

    REQUIRE(stacked);
    REQUIRE(stacked->width() == WIDTH);
    REQUIRE(stacked->height() == HEIGHT);

    for (int j = 0; j < HEIGHT; ++j)
    {
        double* ptr = stacked->data(j);
        double* ref = bitmap2.data(j);

        for (int i = 0; i < WIDTH * 3; ++i)
            REQUIRE(ptr[i] == Approx(ref[i]).margin(0.001));
    }

    delete stacked;
}
