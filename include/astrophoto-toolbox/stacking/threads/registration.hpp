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
RegistrationThread<BITMAP>::RegistrationThread(
    StackingListener* listener, const std::filesystem::path& destFolder
)
: listener(listener), destFolder(destFolder)
{
    if (!std::filesystem::exists(destFolder))
        std::filesystem::create_directories(destFolder);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
RegistrationThread<BITMAP>::~RegistrationThread()
{
    if (thread.joinable())
    {
        cancel();
        thread.join();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool RegistrationThread<BITMAP>::processReferenceFrame(
    const std::string& lightFrame, int luminancyThreshold
)
{
    if (thread.joinable())
        return false;

    lightFrames.push_back(lightFrame);

    cancelled = false;
    terminate = false;

    thread = std::thread(
        &RegistrationThread<BITMAP>::processNextFrame, this, true, luminancyThreshold
    );

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::processFrames(const std::vector<std::string>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    if (!thread.joinable())
    {
        cancelled = false;
        terminate = false;
        thread = std::thread(&RegistrationThread<BITMAP>::processNextFrame, this, false, -1);
    }

    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::cancel()
{
    mutex.lock();
    lightFrames.clear();
    cancelled = true;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::wait()
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
void RegistrationThread<BITMAP>::processNextFrame(bool reference, int luminancyThreshold)
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

        // Register the frame
        std::string name = std::filesystem::path(filename).filename().string();
        std::string extension = std::filesystem::path(name).extension().string();
        std::string destName = name.replace(name.find(extension), extension.size(), ".fits");

        if (reference)
        {
            star_list_t stars = processor.processReference(filename, luminancyThreshold, destFolder / destName);
            listener->lightFrameRegistered(filename, !stars.empty());
            reference = false;
        }
        else
        {
            auto result = processor.process(filename, destFolder / destName);
            listener->lightFrameRegistered(filename, !get<0>(result).empty());
        }
    }
}

}
}
}
