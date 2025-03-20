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
    void masterDarkFrameComputed(const std::filesystem::path& filename, bool success) override
    {
        REQUIRE(false);
    }

    void lightFrameProcessingStarted(const std::filesystem::path& filename) override
    {
        started.push_back(filename);

        if (started.size() == 1)
            condition.notify_one();
    }

    void lightFrameProcessed(const std::filesystem::path& filename, bool success) override
    {
        results[filename] = success;
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

    std::vector<std::filesystem::path> started;
    std::map<std::filesystem::path, bool> results;

    std::condition_variable condition;
    std::mutex mutex;
};


TEST_CASE("(Stacking/Threads/LightFrames) Not started", "[LightFrameThread]")
{
    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    thread.setMasterDark(DATA_DIR "missing.fits");
    thread.processReferenceFrame(DATA_DIR "downloads/light1.fits");
    thread.processFrames({ DATA_DIR "downloads/light2.fits" });

    SECTION("no latch")
    {
        REQUIRE(!thread.cancel());
        REQUIRE(!thread.stop());

        thread.join();

        REQUIRE(listener.started.empty());
        REQUIRE(listener.results.empty());
    }

    SECTION("cancel with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.cancel(&latch));
        REQUIRE(latch.try_wait());

        thread.join();

        REQUIRE(listener.started.empty());
        REQUIRE(listener.results.empty());
    }

    SECTION("stop with latch")
    {
        std::latch latch(1);

        REQUIRE(!thread.stop(&latch));
        REQUIRE(latch.try_wait());

        thread.join();

        REQUIRE(listener.started.empty());
        REQUIRE(listener.results.empty());
    }
}


TEST_CASE("(Stacking/Threads/LightFrames) Cancel processing", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.start());

    thread.setMasterDark(TEMP_DIR "master_dark.fits");
    thread.processReferenceFrame(DATA_DIR "downloads/light1.fits");
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

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

    REQUIRE(listener.started.size() < 3);
    REQUIRE(listener.results.size() <= listener.started.size());
}


TEST_CASE("(Stacking/Threads/LightFrames) Reset processing", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.start());

    thread.setMasterDark(TEMP_DIR "master_dark.fits");
    thread.processReferenceFrame(DATA_DIR "downloads/light1.fits");
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    lock.unlock();

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

    REQUIRE(listener.started.size() < 3);
    REQUIRE(listener.results.size() <= listener.started.size());
}


TEST_CASE("(Stacking/Threads/LightFrames) Process light frames", "[LightFrameThread]")
{
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light1.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light2.fits");
    std::filesystem::remove(TEMP_DIR "threads/lightframes/light3.fits");

    LightFrameTestListener listener;
    LightFrameThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes"));

    REQUIRE(thread.start());

    thread.setMasterDark(TEMP_DIR "master_dark.fits");
    thread.processReferenceFrame(DATA_DIR "downloads/light1.fits");
    thread.processFrames({ DATA_DIR "downloads/light2.fits", DATA_DIR "downloads/light3.fits" });

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

    REQUIRE(listener.started.size() == 3);
    REQUIRE(listener.started[0] == DATA_DIR "downloads/light1.fits");
    REQUIRE(listener.started[1] == DATA_DIR "downloads/light2.fits");
    REQUIRE(listener.started[2] == DATA_DIR "downloads/light3.fits");

    REQUIRE(listener.results.size() == 3);
    REQUIRE(listener.results[DATA_DIR "downloads/light1.fits"]);
    REQUIRE(listener.results[DATA_DIR "downloads/light2.fits"]);
    REQUIRE(listener.results[DATA_DIR "downloads/light3.fits"]);

    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));
}
