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
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::setParameters(const star_list_t& stars, int luminancyThreshold)
{
    mutex.lock();
    this->stars = stars;
    this->luminancyThreshold = luminancyThreshold;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::processReferenceFrame(
    const std::string& lightFrame, int luminancyThreshold
)
{
    mutex.lock();
    referenceFrame = lightFrame;
    this->luminancyThreshold = luminancyThreshold;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::processFrames(const std::vector<std::string>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void RegistrationThread<BITMAP>::process()
{
    auto jobAvailable = [this]{
        return !stars.empty() || !referenceFrame.empty() ||
               !lightFrames.empty() || (state == STATE_CANCELLING) ||
               (state == STATE_STOPPING) || (state == STATE_RESETTING);
    };

    while (true)
    {
        // Wait for a frame to process
        std::unique_lock lock(mutex);
        if (!jobAvailable())
            condition.wait(lock, jobAvailable);

        if (state == STATE_RESETTING)
        {
            stars.clear();
            referenceFrame = "";
            luminancyThreshold = -1;
            lightFrames.clear();
            state = STATE_RUNNING;
            latch->count_down();
            continue;
        }

        if (state == STATE_CANCELLING)
        {
            stars.clear();
            referenceFrame = "";
            luminancyThreshold = -1;
            lightFrames.clear();
            break;
        }

        if (!stars.empty())
        {
            processor.setParameters(stars, luminancyThreshold);
            stars.clear();
            luminancyThreshold = -1;
        }

        if ((state == STATE_STOPPING) && referenceFrame.empty() && lightFrames.empty())
            break;

        if (referenceFrame.empty() && lightFrames.empty())
            continue;

        bool reference = !referenceFrame.empty();
        const std::string filename = reference ? referenceFrame : lightFrames[0];

        if (!reference)
            lightFrames.erase(lightFrames.begin());
        else
            referenceFrame = "";

        lock.unlock();

        // Process the frame
        std::string name = std::filesystem::path(filename).filename().string();
        std::string extension = std::filesystem::path(name).extension().string();
        std::string destName = name.replace(name.find(extension), extension.size(), ".fits");

        listener->lightFrameRegistrationStarted(filename);

        if (reference)
        {
            star_list_t stars = processor.processReference(filename, luminancyThreshold, destFolder / destName);

            if ((state != STATE_CANCELLING) && (state != STATE_RESETTING))
                listener->lightFrameRegistered(filename, !stars.empty());
        }
        else
        {
            auto result = processor.process(filename, destFolder / destName);

            if ((state != STATE_CANCELLING) && (state != STATE_RESETTING))
                listener->lightFrameRegistered(filename, !get<0>(result).empty());
        }
    }
}

}
}
}
