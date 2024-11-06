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
    StackingListener* listener, const std::filesystem::path& destFilename
)
: listener(listener), destFilename(destFilename)
{
}

//-----------------------------------------------------------------------------

template<class BITMAP>
MasterDarkThread<BITMAP>::~MasterDarkThread()
{
    if (thread.joinable())
    {
        cancel();
        thread.join();
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool MasterDarkThread<BITMAP>::processFrames(
    const std::vector<std::string>& darkFrames,
    const std::filesystem::path& tempFolder
)
{
    if (thread.joinable())
        return false;

    cancelled = false;

    thread = std::thread(&MasterDarkThread<BITMAP>::process, this, darkFrames, tempFolder);

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::cancel()
{
    cancelled = true;
    generator.cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::wait()
{
    if (thread.joinable())
        thread.join();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void MasterDarkThread<BITMAP>::process(
    const std::vector<std::string>& darkFrames,
    const std::filesystem::path& tempFolder
)
{
    if (cancelled)
    {
        listener->masterDarkFrameComputed(destFilename, false);
        return;
    }

    BITMAP* bitmap = generator.compute(darkFrames, destFilename, tempFolder);

    listener->masterDarkFrameComputed(destFilename, bitmap != nullptr);

    if (bitmap)
        delete bitmap;
}

}
}
}
