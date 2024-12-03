/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/threads/thread.h>
#include <astrophoto-toolbox/stacking/threads/listener.h>
#include <astrophoto-toolbox/stacking/processing/stacking.h>
#include <filesystem>
#include <string>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to to stack processed light frames
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class StackingThread : public Thread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the caller when a frame is stacked.
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
        //--------------------------------------------------------------------------------
        void processFrames(const std::vector<std::string>& lightFrames);


    private:
        void process() override;

        void onCancel() override;
        void onReset() override;


    private:
        StackingListener* listener;
        std::filesystem::path destFilename;

        processing::FramesStacker<BITMAP> stacker;

        std::vector<std::string> lightFrames;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/stacking.hpp>
