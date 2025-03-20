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
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::setMasterDark(const std::filesystem::path& filename)
{
    mutex.lock();
    masterDark = filename;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::setParameters(
    const utils::background_calibration_parameters_t& parameters
)
{
    mutex.lock();
    this->parameters = parameters;
    parametersValid = true;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::processReferenceFrame(const std::filesystem::path& lightFrame)
{
    mutex.lock();
    referenceFrame = lightFrame;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::processFrames(const std::vector<std::filesystem::path>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::process()
{
    auto jobAvailable = [this]{
        return !masterDark.empty() || parametersValid || !referenceFrame.empty() ||
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
            masterDark = "";
            parametersValid = false;
            referenceFrame = "";
            lightFrames.clear();
            state = STATE_RUNNING;
            latch->count_down();
            continue;
        }

        if (state == STATE_CANCELLING)
        {
            masterDark = "";
            parametersValid = false;
            referenceFrame = "";
            lightFrames.clear();
            break;
        }

        if (!masterDark.empty())
        {
            processor.setMasterDark(masterDark);
            masterDark = "";
        }

        if (parametersValid)
        {
            processor.setParameters(parameters);
            parametersValid = false;
        }

        if ((state == STATE_STOPPING) && referenceFrame.empty() && lightFrames.empty())
            break;

        if (referenceFrame.empty() && lightFrames.empty())
            continue;

        bool reference = !referenceFrame.empty();
        const std::filesystem::path filename = reference ? referenceFrame : lightFrames[0];

        if (!reference)
            lightFrames.erase(lightFrames.begin());
        else
            referenceFrame = "";

        lock.unlock();

        // Process the frame
        std::string name = std::filesystem::path(filename).filename().string();
        std::string extension = std::filesystem::path(name).extension().string();
        std::string destName = name.replace(name.find(extension), extension.size(), ".fits");

        listener->lightFrameProcessingStarted(filename);

        std::shared_ptr<BITMAP> bitmap = processor.process(filename, reference, destFolder / destName);

        if ((state != STATE_CANCELLING) && (state != STATE_RESETTING))
            listener->lightFrameProcessed(filename, (bool) bitmap);
    }
}

}
}
}
