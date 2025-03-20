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
    void masterDarkFrameComputed(const std::filesystem::path& filename, bool success) override
    {
        results[filename] = success;
    }

    void lightFrameProcessingStarted(const std::filesystem::path& filename) override
    {
        REQUIRE(false);
    }

    void lightFrameProcessed(const std::filesystem::path& filename, bool success) override
    {
        REQUIRE(false);
    }

    void lightFrameRegistrationStarted(const std::filesystem::path& filename) override
    {
        REQUIRE(false);
    }

    void lightFrameRegistered(const std::filesystem::path& filename, bool success) override
    {
        REQUIRE(false);
    }

    void lightFramesStackingStarted(unsigned int nbFrames) override
    {
        REQUIRE(false);
    }

    void lightFramesStacked(const std::filesystem::path& filename, unsigned int nbFrames) override
    {
        REQUIRE(false);
    }

    std::map<std::filesystem::path, bool> results;
};


TEST_CASE("(Stacking/Threads/MasterDark) Not started", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(
        &listener, TEMP_DIR "threads/masterdark.fits", TEMP_DIR "threads/tmp_darkframe"
    );

    thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        }
    );

    SECTION("no latch")
    {
        REQUIRE(!thread.cancel());
        REQUIRE(!thread.stop());

        thread.join();

        REQUIRE(listener.results.empty());
    }

    SECTION("cancel with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.cancel(&latch));
        REQUIRE(latch.try_wait());

        thread.join();

        REQUIRE(listener.results.empty());
    }

    SECTION("stop with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.stop(&latch));
        REQUIRE(latch.try_wait());

        thread.join();

        REQUIRE(listener.results.empty());
    }
}


TEST_CASE("(Stacking/Threads/MasterDark) Cancel processing", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(
        &listener, TEMP_DIR "threads/masterdark.fits", TEMP_DIR "threads/tmp_darkframe"
    );

    REQUIRE(thread.start());

    thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        }
    );

    SECTION("without latch")
    {
        REQUIRE(thread.cancel());
        thread.join();
    }

    SECTION("with latch")
    {
        std::latch latch(1);

        REQUIRE(thread.cancel(&latch));

        latch.wait();
        thread.join();
    }

    REQUIRE(listener.results.empty());
}


TEST_CASE("(Stacking/Threads/MasterDark) Reset processing", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(
        &listener, TEMP_DIR "threads/masterdark.fits", TEMP_DIR "threads/tmp_darkframe"
    );

    REQUIRE(thread.start());

    thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        }
    );

    REQUIRE(thread.reset());

    SECTION("without latch")
    {
        REQUIRE(thread.stop());
        thread.join();
    }

    SECTION("with latch")
    {
        std::latch latch(1);

        REQUIRE(thread.stop(&latch));

        latch.wait();
        thread.join();
    }

    REQUIRE(listener.results.empty());
}


TEST_CASE("(Stacking/Threads/MasterDark) Generate master dark", "[MasterDarkThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/masterdark.fits");

    MasterDarkTestListener listener;
    MasterDarkThread<UInt16ColorBitmap> thread(
        &listener, TEMP_DIR "threads/masterdark.fits", TEMP_DIR "threads/tmp_darkframe"
    );
 
    REQUIRE(thread.start());

    thread.processFrames(
        {
            DATA_DIR "downloads/dark1.fits",
            DATA_DIR "downloads/dark2.fits",
            DATA_DIR "downloads/dark3.fits",
        }
    );

    SECTION("without latch")
    {
        REQUIRE(thread.stop());
        thread.join();
    }

    SECTION("with latch")
    {
        std::latch latch(1);

        REQUIRE(thread.stop(&latch));

        latch.wait();
        thread.join();
    }

    REQUIRE(listener.results.size() == 1);
    REQUIRE(listener.results[TEMP_DIR "threads/masterdark.fits"]);

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/masterdark.fits"));
    REQUIRE(!std::filesystem::exists(TEMP_DIR "threads/tmp_darkframe"));
}
