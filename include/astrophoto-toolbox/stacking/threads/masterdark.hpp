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
MasterDarkThread<BITMAP>::MasterDarkThread(
    StackingListener* listener, const std::filesystem::path& destFilename,
    const std::filesystem::path& tempFolder
)
: listener(listener), destFilename(destFilename), tempFolder(tempFolder)
{
}

//-----------------------------------------------------------------------------

template<class BITMAP>
MasterDarkThread<BITMAP>::~MasterDarkThread()
{
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::processFrames(const std::vector<std::string>& darkFrames)
{
    mutex.lock();
    this->darkFrames = darkFrames;
    mutex.unlock();
    condition.notify_one();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::process()
{
    auto jobAvailable = [this]{
        return !darkFrames.empty() || (state == STATE_CANCELLING) ||
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
            darkFrames.clear();
            state = STATE_RUNNING;
            latch->count_down();
            continue;
        }

        if (state == STATE_CANCELLING)
        {
            darkFrames.clear();
            break;
        }

        if ((state == STATE_STOPPING) && darkFrames.empty())
            break;

        auto filenames = darkFrames;
        darkFrames.clear();

        lock.unlock();

        BITMAP* bitmap = generator.compute(filenames, destFilename, tempFolder);

        if ((state != STATE_CANCELLING) && (state != STATE_RESETTING))
            listener->masterDarkFrameComputed(destFilename, (bool) bitmap);

        delete bitmap;
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::onCancel()
{
    generator.cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::onReset()
{
    generator.cancel();
}

}
}
}
