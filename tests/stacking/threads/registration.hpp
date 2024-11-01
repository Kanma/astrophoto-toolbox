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
    void lightFrameProcessed(const std::string& filename, bool success) override
    {
    }

    void lightFrameRegistered(const std::string& filename, bool success) override
    {
        results[filename] = success;
    }

    void lightFramesStacked(const std::string& filename) override
    {
    }

    std::map<std::string, bool> results;
};


TEST_CASE("(Stacking/Threads/Registration) Cancel registration", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1));
    thread.processFrames({
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() < 3);
}


TEST_CASE("(Stacking/Threads/Registration) Fail to process reference frame while already running", "[RegistrationThread]")
{
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light1.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light2.fits"));
    REQUIRE(std::filesystem::exists(TEMP_DIR "threads/lightframes/light3.fits"));

    RegistrationTestListener listener;
    RegistrationThread<UInt16ColorBitmap> thread(&listener, TEMP_DIR "threads/lightframes");

    REQUIRE(thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1));
    thread.processFrames({
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    REQUIRE(!thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1));

    thread.cancel();
    thread.wait();

    REQUIRE(listener.results.size() < 3);
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

    REQUIRE(thread.processReferenceFrame(TEMP_DIR "threads/lightframes/light1.fits", -1));
    thread.processFrames({
        TEMP_DIR "threads/lightframes/light2.fits",
        TEMP_DIR "threads/lightframes/light3.fits"
    });

    thread.wait();

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
