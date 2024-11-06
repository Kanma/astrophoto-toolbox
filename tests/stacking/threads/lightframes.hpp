/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/threads/lightframes.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::threads;


class LightFrameTestListener : public StackingListener
{
public:
    void masterDarkFrameComputed(const std::string& filename, bool success) override
    {
    }

    void lightFrameProcessed(const std::string& filename, bool success) override
    {
        results[filename] = success;
    }

    void lightFrameRegistered(const std::string& filename, bool success) override
    {
    }

    void lightFramesStacked(const std::string& filename, unsigned int nbFrames) override
    {
    }

    std::map<std::string, bool> results;
};


TEST_CASE("(Stacking/Threads/LightFrames) Fail to use missing master dark frame file", "[LightFrameThread]")
{
    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");
    REQUIRE(!thread.setMasterDark(DATA_DIR "missing.fits"));
}


TEST_CASE("(Stacking/Threads/LightFrames) Cancel processing", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");
 
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.setMasterDark(TEMP_DIR "master_dark.fits"));
    REQUIRE(thread.processReferenceFrame(DATA_DIR "downloads/light1.fits"));
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() < 3);
}


TEST_CASE("(Stacking/Threads/LightFrames) Fail to process reference frame while already running", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");
 
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.setMasterDark(TEMP_DIR "master_dark.fits"));
    REQUIRE(thread.processReferenceFrame(DATA_DIR "downloads/light1.fits"));
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

    REQUIRE(!thread.processReferenceFrame(DATA_DIR "downloads/light1.fits"));

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() < 3);
}


TEST_CASE("(Stacking/Threads/LightFrames) Process light frames", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");
 
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.setMasterDark(TEMP_DIR "master_dark.fits"));
    REQUIRE(thread.processReferenceFrame(DATA_DIR "downloads/light1.fits"));
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

    thread.wait();

    REQUIRE(listener.results.size() == 3);
    REQUIRE(listener.results[DATA_DIR "downloads/light1.fits"]);
    REQUIRE(listener.results[DATA_DIR "downloads/light2.fits"]);
    REQUIRE(listener.results[DATA_DIR "downloads/light3.fits"]);

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));
}
