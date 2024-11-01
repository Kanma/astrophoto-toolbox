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
StackingThread<BITMAP>::StackingThread(
    StackingListener* listener, const std::filesystem::path& destFilename
)
: listener(listener), destFilename(destFilename)
{
}

//-----------------------------------------------------------------------------

template<class BITMAP>
StackingThread<BITMAP>::~StackingThread()
{
    if (thread.joinable())
    {
        cancel();
        thread.join();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool StackingThread<BITMAP>::setup(
    unsigned int nbExpectedFrames, const std::filesystem::path& tempFolder,
    unsigned long maxFileSize
)
{
    if (thread.joinable())
        return false;

    cancelled = false;

    stacker.setup(nbExpectedFrames, tempFolder, maxFileSize);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::processFrames(const std::vector<std::string>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    if (!thread.joinable())
        thread = std::thread(&StackingThread<BITMAP>::process, this);

    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::cancel()
{
    mutex.lock();
    lightFrames.clear();
    stacker.cancel();
    cancelled = true;
    mutex.unlock();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::wait()
{
    if (thread.joinable())
        thread.join();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::process()
{
    while (!cancelled)
    {
        std::vector<std::string> filenames;

        mutex.lock();
        if (lightFrames.empty())
        {
            mutex.unlock();
            return;
        }

        filenames = lightFrames;
        lightFrames.clear();

        mutex.unlock();

        for (const auto& filename : filenames)
        {
            stacker.addFrame(filename);

            if (cancelled)
                return;
        }

        BITMAP* bitmap = stacker.process(destFilename);
        if (bitmap)
        {
            listener->lightFramesStacked(destFilename);
            delete bitmap;
        }
    }
}

}
}
}
