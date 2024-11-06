/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/processing/masterdark.h>
#include <astrophoto-toolbox/stacking/threads/listener.h>
#include <filesystem>
#include <string>
#include <thread>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to to generate the master dark
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class MasterDarkThread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the caller when the master dark frame is
        /// computed.
        //--------------------------------------------------------------------------------
        MasterDarkThread(StackingListener* listener, const std::filesystem::path& destFilename);

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        ///
        /// If the thread is still running, the processing will be cancelled.
        //--------------------------------------------------------------------------------
        ~MasterDarkThread();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Stack the provided list of dark frames
        ///
        /// It is expected that the dark frames have been properly processed.
        ///
        /// It is expected that the thread isn't already running. Calling this method will
        /// start the thread.
        //--------------------------------------------------------------------------------
        bool processFrames(
            const std::vector<std::string>& darkFrames,
            const std::filesystem::path& tempFolder
        );

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        //--------------------------------------------------------------------------------
        void cancel();

        //--------------------------------------------------------------------------------
        /// @brief  Wait for the thread to terminate its job (either because the master
        ///         dark frame was generated, or because the processing was cancelled)
        //--------------------------------------------------------------------------------
        void wait();


    private:
        void process(
            const std::vector<std::string>& darkFrames,
            const std::filesystem::path& tempFolder
        );


    private:
        StackingListener* listener;
        std::filesystem::path destFilename;

        processing::MasterDarkGenerator<BITMAP> generator;
        std::thread thread;

        bool cancelled;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/masterdark.hpp>
