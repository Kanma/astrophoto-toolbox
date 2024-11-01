/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/processing/stacking.h>
#include <astrophoto-toolbox/stacking/threads/listener.h>
#include <filesystem>
#include <string>
#include <thread>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to to stack processed light frames
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class StackingThread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the called when a frame is stacked.
        //--------------------------------------------------------------------------------
        StackingThread(StackingListener* listener, const std::filesystem::path& destFilename);

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        ///
        /// If the thread is still running, the processing will be cancelled.
        //--------------------------------------------------------------------------------
        ~StackingThread();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Setup the stacker
        ///
        /// It is expected that the thread isn't already running.
        //--------------------------------------------------------------------------------
        bool setup(
            unsigned int nbExpectedFrames,
            const std::filesystem::path& tempFolder,
            unsigned long maxFileSize = 50000000L
        );

        //--------------------------------------------------------------------------------
        /// @brief  Add a list of light frame files to the stack
        ///
        /// It is expected that the light frame has been properly processed.
        ///
        /// Calling this method will start the thread (if not already running).
        //--------------------------------------------------------------------------------
        void processFrames(const std::vector<std::string>& lightFrames);

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        //--------------------------------------------------------------------------------
        void cancel();

        //--------------------------------------------------------------------------------
        /// @brief  Wait for the thread to terminate its job (either because all frames
        ///         are processed, or because the processing was cancelled)
        //--------------------------------------------------------------------------------
        void wait();


    private:
        void process();


    private:
        StackingListener* listener;
        std::filesystem::path destFilename;

        FramesStacker<BITMAP> stacker;
        std::thread thread;

        std::vector<std::string> lightFrames;
        std::mutex mutex;

        bool cancelled = false;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/stacking.hpp>
