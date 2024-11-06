/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/threads/masterdark.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::threads;


class MasterDarkTestListener : public StackingListener
{
public:
    void masterDarkFrameComputed(const std::string& filename, bool success) override
    {
        results[filename] = success;
    }

    void lightFrameProcessed(const std::string& filename, bool success) override
    {
    }

    void lightFrameRegistered(const std::string& filename, bool success) override
    {
    }

    void lightFramesStacked(const std::string& filename, unsigned int nbFrames) override
    {
    }

    std::map<std::string, bool> results;
};


TEST_CASE("(Stacking/Threads/MasterDark) Cancel processing", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/masterdark.fits");
 
    REQUIRE(thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        },
        TEMP_DIR "threads/tmp_darkframe"
    ));

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() == 1);
    REQUIRE(!listener.results[TEMP_DIR "threads/masterdark.fits"]);
}


TEST_CASE("(Stacking/Threads/MasterDark) Fail to process frames while already running", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/masterdark.fits");
 
    REQUIRE(thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
        },
        TEMP_DIR "threads/tmp_darkframe"
    ));

    REQUIRE(!thread.processFrames(
        {
            DATA_DIR "downloads/dark3.fits",
        },
        TEMP_DIR "threads/tmp_darkframe"
    ));

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() == 1);
    REQUIRE(!listener.results[TEMP_DIR "threads/masterdark.fits"]);
}


TEST_CASE("(Stacking/Threads/MasterDark) Generate master dark", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/masterdark.fits");
 
    REQUIRE(thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        },
        TEMP_DIR "threads/tmp_darkframe"
    ));

    thread.wait();

    REQUIRE(listener.results.size() == 1);
    REQUIRE(listener.results[TEMP_DIR "threads/masterdark.fits"]);

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/masterdark.fits"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "threads/tmp_darkframe"));
}
