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
    void masterDarkFrameComputed(const std::filesystem::path& filename, bool success) override
    {
        REQUIRE(false);
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
        nbFramesAtStart = nbFrames;
        condition.notify_one();
    }

    void lightFramesStacked(const std::filesystem::path& filename, unsigned int nbFrames) override
    {
        this->filename = filename;
        nbFramesStacked = nbFrames;
    }

    unsigned int nbFramesAtStart = 0;
    unsigned int nbFramesStacked = 0;
    std::filesystem::path filename;

    std::condition_variable condition;
    std::mutex mutex;
};


TEST_CASE("(Stacking/Threads/Stacking) Not started", "[StackingThread]")
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

    SECTION("no latch")
    {
        REQUIRE(!thread.cancel());
        REQUIRE(!thread.stop());

        thread.join();
    }

    SECTION("cancel with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.cancel(&latch));
        REQUIRE(latch.try_wait());

        thread.join();
    }

    SECTION("stop with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.stop(&latch));
        REQUIRE(latch.try_wait());

        thread.join();
    }

    REQUIRE(listener.nbFramesAtStart == 0);
    REQUIRE(listener.nbFramesStacked == 0);
    REQUIRE(listener.filename.empty());
}


TEST_CASE("(Stacking/Threads/Stacking) Cancel stacking", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    REQUIRE(thread.start());

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    lock.unlock();

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

    REQUIRE(listener.nbFramesAtStart == 3);
    REQUIRE(listener.nbFramesStacked == 0);
    REQUIRE(listener.filename.empty());
}


TEST_CASE("(Stacking/Threads/Stacking) Fail to setup while already running", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    REQUIRE(thread.start());

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    lock.unlock();

    REQUIRE(!thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    REQUIRE(thread.cancel());
    thread.join();

    REQUIRE(listener.nbFramesAtStart == 3);
    REQUIRE(listener.nbFramesStacked == 0);
    REQUIRE(listener.filename.empty());
}


TEST_CASE("(Stacking/Threads/Stacking) Stacking", "[StackingThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    std::filesystem::remove(TEMP_DIR "threads/stacked.fits");

    StackingTestListener listener;
    StackingThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/stacked.fits");

    REQUIRE(thread.setup(3, TEMP_DIR "threads/tmp_stacking"));

    REQUIRE(thread.start());

    thread.processFrames({
        TEMP_DIR "threads/lightframes/light1.fits",
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

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

    REQUIRE(listener.nbFramesAtStart == 3);
    REQUIRE(listener.nbFramesStacked == 3);
    REQUIRE(listener.filename == TEMP_DIR "threads/stacked.fits");

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/stacked.fits"));
}
