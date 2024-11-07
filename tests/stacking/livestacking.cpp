/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <catch.hpp>
#include <astrophoto-toolbox/stacking/livestacking.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;
using namespace astrophototoolbox::stacking;


TEST_CASE("(LiveStacking) Master dark frame computation only", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            stacking->cancel();
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(false);
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;
    };


    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    REQUIRE(stacking.start());

    stacking.wait();

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/master_dark.fits"));
}


TEST_CASE("(LiveStacking) Cancelling during master dark frame computation", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(false);
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(false);
        }
    };


    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener;

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    REQUIRE(stacking.start());

    stacking.cancel();
    stacking.wait();

    REQUIRE(!std::filesystem::exists(TEMP_DIR "livestacking/master_dark.fits"));
}


TEST_CASE("(LiveStacking) Full process", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            REQUIRE(infos.lightFrames.nb == 3);
            REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);
            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
    };

    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.start());

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(stacking.getReference() == 0);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Save config file", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(false);
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(false);
        }
    };


    REQUIRE(!std::filesystem::exists(TEMP_DIR "livestacking/stacking.txt"));

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener;

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.save());

    std::filesystem::path config(TEMP_DIR "livestacking/stacking.txt");
    REQUIRE(std::filesystem::exists(config));

    std::ifstream input(config, std::ios::in);
    std::string line;

    REQUIRE(std::getline(input, line));
    REQUIRE(line == "DARKFRAMES");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/dark1.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/dark2.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/dark3.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == "---");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == "LIGHTFRAMES");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/light1.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/light2.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == DATA_DIR "downloads/light3.fits");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == "REF 0");

    REQUIRE(std::getline(input, line));
    REQUIRE(line == "---");

    REQUIRE(!std::getline(input, line));
}


TEST_CASE("(LiveStacking) Load and process in populated folder", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            REQUIRE(infos.lightFrames.nb == 3);
            REQUIRE(infos.lightFrames.nbProcessed == 3);
            REQUIRE(infos.lightFrames.nbRegistered == 3);
            REQUIRE(infos.lightFrames.nbValid == 3);
            REQUIRE(((infos.lightFrames.nbStacked == 0) || (infos.lightFrames.nbStacked == 3)));

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;

            ++counter;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
        int counter = 0;
    };


    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacking.txt"));

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));
    REQUIRE(stacking.load());
    REQUIRE(stacking.start());

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(listener.counter == 1);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Load and process in empty folder", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            REQUIRE(infos.lightFrames.nb == 3);
            REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);
            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
    };

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacking.txt"));
    std::filesystem::remove_all(TEMP_DIR "livestacking/calibrated");
    std::filesystem::remove(TEMP_DIR "master_dark.fits");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    REQUIRE(stacking.load());
    REQUIRE(stacking.start());

    stacking.wait();

    REQUIRE(listener.stackingComplete);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Add dark frame during processing", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            if (darkFrameAdded)
                REQUIRE(infos.nbDarkFrames == 3);
            else
                REQUIRE(infos.nbDarkFrames == 2);

            REQUIRE(infos.lightFrames.nb == 3);
            REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);
            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (!darkFrameAdded && (infos.lightFrames.nbProcessed == 1))
            {
                darkFrameAdded = true;
                condition.notify_one();
            }

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
        bool darkFrameAdded = false;
        std::condition_variable condition;
        std::mutex mutex;
    };

    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.start());

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");
    lock.unlock();

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(listener.darkFrameAdded);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Add light frame during processing", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);

            if (lightFrameAdded)
                REQUIRE(infos.lightFrames.nb == 3);
            else
                REQUIRE(infos.lightFrames.nb == 2);

            REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);
            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (!lightFrameAdded && (infos.lightFrames.nbProcessed == 1))
            {
                lightFrameAdded = true;
                condition.notify_one();
            }

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
        bool lightFrameAdded = false;
        std::condition_variable condition;
        std::mutex mutex;
    };

    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");

    REQUIRE(stacking.start());

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");
    lock.unlock();

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(listener.lightFrameAdded);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Change reference light frame during processing", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            REQUIRE(infos.lightFrames.nb == 3);
            REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);
            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (!referenceChanged && (infos.lightFrames.nbRegistered == 1))
            {
                referenceChanged = true;
                condition.notify_one();
            }

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
        bool referenceChanged = false;
        std::condition_variable condition;
        std::mutex mutex;
    };

    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.start());

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    stacking.setReference(2);
    lock.unlock();

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(listener.referenceChanged);
    REQUIRE(stacking.getReference() == 2);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}


TEST_CASE("(LiveStacking) Change luminancy threshold during processing", "[LiveStacking]")
{
    class Listener : public LiveStackingListener
    {
    public:
        Listener(LiveStacking<UInt16ColorBitmap>* stacking)
        : stacking(stacking)
        {
        }

        void progressNotification(const live_stacking_infos_t& infos) override
        {
            REQUIRE(infos.nbDarkFrames == 3);
            REQUIRE(infos.lightFrames.nb == 3);

            if (thresholdChanged)
                REQUIRE(infos.lightFrames.nbProcessed == 3);
            else
                REQUIRE(infos.lightFrames.nbProcessed <= infos.lightFrames.nb);

            REQUIRE(infos.lightFrames.nbRegistered <= infos.lightFrames.nbProcessed);
            REQUIRE(infos.lightFrames.nbValid <= infos.lightFrames.nbRegistered);
            REQUIRE(infos.lightFrames.nbStacked <= infos.lightFrames.nbRegistered);

            if (!thresholdChanged && (infos.lightFrames.nbProcessed == 3) && (infos.lightFrames.nbRegistered > 0))
            {
                thresholdChanged = true;
                condition.notify_one();
            }

            if (infos.lightFrames.nbStacked == 3)
                stackingComplete = true;
        }

        void stackingDone(const std::string& filename) override
        {
            REQUIRE(filename == TEMP_DIR "livestacking/stacked.fits");
        }

    private:
        LiveStacking<UInt16ColorBitmap>* stacking = nullptr;

    public:
        bool stackingComplete = false;
        bool thresholdChanged = false;
        std::condition_variable condition;
        std::mutex mutex;
    };

    std::filesystem::remove_all(TEMP_DIR "livestacking");

    LiveStacking<UInt16ColorBitmap> stacking;
    Listener listener(&stacking);

    REQUIRE(stacking.setup(&listener, TEMP_DIR "livestacking"));

    stacking.addDarkFrame(DATA_DIR "downloads/dark1.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark2.fits");
    stacking.addDarkFrame(DATA_DIR "downloads/dark3.fits");

    stacking.addLightFrame(DATA_DIR "downloads/light1.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light2.fits");
    stacking.addLightFrame(DATA_DIR "downloads/light3.fits");

    REQUIRE(stacking.start());

    std::unique_lock<std::mutex> lock(listener.mutex);
    listener.condition.wait(lock);
    stacking.setLuminancyThreshold(3);
    lock.unlock();

    stacking.wait();

    REQUIRE(listener.stackingComplete);
    REQUIRE(listener.thresholdChanged);

    REQUIRE(std::filesystem::exists(TEMP_DIR "livestacking/stacked.fits"));
}
