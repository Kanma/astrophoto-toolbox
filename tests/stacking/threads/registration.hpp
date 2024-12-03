/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/threads/registration.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;
using namespace astrophototoolbox::stacking::threads;


class RegistrationTestListener : public StackingListener
{
public:
    void masterDarkFrameComputed(const std::string& filename, bool success) override
    {
        REQUIRE(false);
    }

    void lightFrameProcessingStarted(const std::string& filename) override
    {
        REQUIRE(false);
    }

    void lightFrameProcessed(const std::string& filename, bool success) override
    {
        REQUIRE(false);
    }

    void lightFrameRegistrationStarted(const std::string& filename) override
    {
        started.push_back(filename);

        if (started.size() == 1)
            condition.notify_one();
    }

    void lightFrameRegistered(const std::string& filename, bool success) override
    {
        results[filename] = success;
    }

    void lightFramesStackingStarted(unsigned int nbFrames) override
    {
        REQUIRE(false);
    }

    void lightFramesStacked(const std::string& filename, unsigned int nbFrames) override
    {
        REQUIRE(false);
    }

    std::vector<std::string> started;
    std::map<std::string, bool> results;

    std::condition_variable condition;
    std::mutex mutex;
};


TEST_CASE("(Stacking/Threads/Registration) Not started", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1);
    thread.processFrames({
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

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


TEST_CASE("(Stacking/Threads/Registration) Cancel registration", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(thread.start());

    thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1);
    thread.processFrames({
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

    REQUIRE(listener.started.size() < 3);
    REQUIRE(listener.results.size() <= listener.started.size());
}


TEST_CASE("(Stacking/Threads/Registration) Reset registration", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(thread.start());

    thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1);
    thread.processFrames({
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

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


TEST_CASE("(Stacking/Threads/Registration) Registration", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    const auto readFile = [](const std::string& filename, star_list_t& stars, Transformation& transformation)
    {
        std::filesystem::path path(filename);
        REQUIRE(std::filesystem::exists(path));

        FITS fits;
        REQUIRE(fits.open(path));

        Bitmap* bitmap = fits.readBitmap();
        REQUIRE(bitmap);
        delete bitmap;

        stars = fits.readStars();
        transformation = fits.readTransformation();
    };

    REQUIRE(thread.start());

    thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1);
    thread.processFrames({
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

    REQUIRE(listener.started.size() == 3);
    REQUIRE(listener.started[0] == TEMP_DIR "threads/lightframes/light1.fits");
    REQUIRE(listener.started[1] == TEMP_DIR "threads/lightframes/light2.fits");
    REQUIRE(listener.started[2] == TEMP_DIR "threads/lightframes/light3.fits");

    REQUIRE(listener.results.size() == 3);
    REQUIRE(listener.results[TEMP_DIR "threads/lightframes/light1.fits"]);
    REQUIRE(listener.results[TEMP_DIR "threads/lightframes/light2.fits"]);
    REQUIRE(listener.results[TEMP_DIR "threads/lightframes/light3.fits"]);


    star_list_t stars;
    Transformation transformation;
    point_t point;

    readFile(TEMP_DIR "threads/lightframes/light1.fits", stars, transformation);
    REQUIRE(stars.size() == 38);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(200.0));
    REQUIRE(point.y == Approx(100.0));

    readFile(TEMP_DIR "threads/lightframes/light2.fits", stars, transformation);
    REQUIRE(stars.size() == 30);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(216.529).margin(0.001));
    REQUIRE(point.y == Approx(98.799).margin(0.001));

    readFile(TEMP_DIR "threads/lightframes/light3.fits", stars, transformation);
    REQUIRE(stars.size() == 34);

    point = transformation.transform(point_t(200, 100));
    REQUIRE(point.x == Approx(136.686).margin(0.001));
    REQUIRE(point.y == Approx(196.510).margin(0.001));
}
