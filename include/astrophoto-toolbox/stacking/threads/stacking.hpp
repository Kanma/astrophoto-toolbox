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
    terminate = false;

    lightFrames.clear();

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
    {
        cancelled = false;
        terminate = false;
        thread = std::thread(&StackingThread<BITMAP>::process, this);
    }

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
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::wait()
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
void StackingThread<BITMAP>::process()
{
    auto jobAvailable = [this]{ return !lightFrames.empty() || cancelled || terminate; };

    while (true)
    {
        // Wait for frames to process
        std::unique_lock lock(mutex);
        if (!jobAvailable())
            condition.wait(lock, jobAvailable);

        if (cancelled)
            break;

        if (terminate && lightFrames.empty())
            break;

        std::vector<std::string> filenames = lightFrames;
        lightFrames.clear();

        lock.unlock();

        // Stack the frames
        for (const auto& filename : filenames)
        {
            stacker.addFrame(filename);

            if (cancelled)
                return;
        }

        BITMAP* bitmap = stacker.process(destFilename);
        if (bitmap)
        {
            listener->lightFramesStacked(destFilename, stacker.nbFrames());
            delete bitmap;
        }
    }
}

}
}
}
