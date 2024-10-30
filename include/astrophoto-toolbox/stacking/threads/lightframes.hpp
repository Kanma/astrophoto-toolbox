/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is essentially a reimplementation of parts of 'DeepSkyStacker',
 * which is released under a BSD 3-Clause license,
 * Copyright (c) 2006-2019, LucCoiffier
 * Copyright (c) 2018-2023,
 *      David C. Partridge, Tony Cook, Mat Draper,
 *      Simon C. Smith, Vitali Pelenjow, Michal Schulz, Martin Toeltsch
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

    thread = std::thread(&LightFrameThread<BITMAP>::processNextFrame, this, true);

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
        thread = std::thread(&LightFrameThread<BITMAP>::processNextFrame, this, false);

    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::cancel()
{
    mutex.lock();
    lightFrames.clear();
    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::wait()
{
    if (thread.joinable())
        thread.join();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void LightFrameThread<BITMAP>::processNextFrame(bool reference)
{
    while (true)
    {
        mutex.lock();
        if (lightFrames.empty())
        {
            mutex.unlock();
            return;
        }

        const std::string filename = lightFrames[0];
        lightFrames.erase(lightFrames.begin());

        mutex.unlock();

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
