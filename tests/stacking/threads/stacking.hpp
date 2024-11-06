/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/threads/stacking.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::threads;


class StackingTestListener : public StackingListener
{
public:
    void masterDarkFrameComputed(const std::string& filename, bool success) override
    {
    }

    void lightFrameProcessed(const std::string& filename, bool success) override
    {
    }

    void lightFrameRegistered(const std::string& filename, bool success) override
    {
    }

    void lightFramesStacked(const std::string& filename, unsigned int nbFrames) override
    {
        results.push_back(filename);
    }

    std::vector<std::string> results;
};


TEST_CASE("(Stacking/Threads/Stacking) Cancel stacking", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.empty());
}


TEST_CASE("(Stacking/Threads/Stacking) Fail to setup while already running", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    REQUIRE(!thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.empty());
}


TEST_CASE("(Stacking/Threads/Stacking) Stacking", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "threads/stacked.fits"));

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    thread.wait();

    REQUIRE(listener.results.size() == 1);
    REQUIRE(listener.results[0] == TEMP_DIR "threads/stacked.fits");

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/stacked.fits"));
}
