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

    stacker.setup(nbExpectedFrames, tempFolder, maxFileSize);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::processFrames(const std::vector<std::filesystem::path>& lightFrames)
{
    mutex.lock();

    for (const auto& lightFrame : lightFrames)
        this->lightFrames.push_back(lightFrame);

    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::process()
{
    auto jobAvailable = [this]{
        return !lightFrames.empty() || (state == STATE_CANCELLING) ||
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
            lightFrames.clear();
            stacker.clear();
            state = STATE_RUNNING;
            latch->count_down();
            continue;
        }

        if (state == STATE_CANCELLING)
        {
            lightFrames.clear();
            break;
        }

        if ((state == STATE_STOPPING) && lightFrames.empty())
            break;

        auto filenames = lightFrames;
        lightFrames.clear();

        lock.unlock();

        listener->lightFramesStackingStarted(stacker.nbFrames() + filenames.size());

        // Stack the frames
        for (const auto& filename : filenames)
        {
            stacker.addFrame(filename);

            if ((state == STATE_CANCELLING) || (state == STATE_RESETTING))
                break;
        }

        if ((state == STATE_CANCELLING) || (state == STATE_RESETTING))
            continue;

        BITMAP* bitmap = stacker.process(destFilename);
        if (bitmap)
        {
            if ((state != STATE_CANCELLING) && (state != STATE_RESETTING))
                listener->lightFramesStacked(destFilename, stacker.nbFrames());

            delete bitmap;
        }
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::onCancel()
{
    stacker.cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void StackingThread<BITMAP>::onReset()
{
    stacker.cancel();
}

}
}
}
