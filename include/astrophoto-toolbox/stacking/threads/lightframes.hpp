/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

namespace astrophototoolbox {
namespace stacking {
namespace threads {


template<class BITMAP>
LightFrameThread<BITMAP>::LightFrameThread(
    StackingListener* listener, const std::filesystem::path& destFolder
)
: listener(listener), destFolder(destFolder)
{
    if (!std::filesystem::exists(destFolder))
        std::filesystem::create_directories(destFolder);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
LightFrameThread<BITMAP>::~LightFrameThread()
{
    if (thread.joinable())
    {
        cancel();
        thread.join();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LightFrameThread<BITMAP>::setMasterDark(const std::string& filename)
{
    if (thread.joinable())
        return false;

    return processor.setMasterDark(filename);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool LightFrameThread<BITMAP>::processReferenceFrame(const std::string& lightFrame)
{
    if (thread.joinable())
        return false;

    lightFrames.push_back(lightFrame);

    cancelled = false;
    terminate = false;

    thread = std::thread(&LightFrameThread<BITMAP>::processNextFrame, this, true);
    condition.notify_one();

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::processFrames(const std::vector<std::string>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    if (!thread.joinable())
    {
        cancelled = false;
        terminate = false;
        thread = std::thread(&LightFrameThread<BITMAP>::processNextFrame, this, false);
    }

    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::cancel()
{
    mutex.lock();
    lightFrames.clear();
    cancelled = true;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::wait()
{
    if (thread.joinable())
    {
        mutex.lock();
        terminate = true;
        mutex.unlock();
        condition.notify_one();

        thread.join();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::processNextFrame(bool reference)
{
    auto jobAvailable = [this]{ return !lightFrames.empty() || cancelled || terminate; };

    while (true)
    {
        // Wait for a frame to process
        std::unique_lock lock(mutex);
        if (!jobAvailable())
            condition.wait(lock, jobAvailable);

        if (cancelled)
            break;

        if (terminate && lightFrames.empty())
            break;

        const std::string filename = lightFrames[0];
        lightFrames.erase(lightFrames.begin());

        lock.unlock();

        // Process the frame
        std::string name = std::filesystem::path(filename).filename().string();
        std::string extension = std::filesystem::path(name).extension().string();
        std::string destName = name.replace(name.find(extension), extension.size(), ".fits");

        std::shared_ptr<BITMAP> bitmap = processor.process(filename, reference, destFolder / destName);

        listener->lightFrameProcessed(filename, (bool) bitmap);
        reference = false;
    }
}

}
}
}
