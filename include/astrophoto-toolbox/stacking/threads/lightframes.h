/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/processing/lightframes.h>
#include <astrophoto-toolbox/stacking/threads/listener.h>
#include <filesystem>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to perform all the operations related to light frames
    ///         (background calibration, dark frame substraction, ...)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class LightFrameThread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the caller when a frame was processed, and
        /// the destination folder (will be created if necessary) is where the processed
        /// frame files will be saved.
        //--------------------------------------------------------------------------------
        LightFrameThread(StackingListener* listener, const std::filesystem::path& destFolder);

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        ///
        /// If the thread is still running, the processing will be cancelled.
        //--------------------------------------------------------------------------------
        ~LightFrameThread();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Set the master dark frame file to use
        ///
        /// It is expected that the thread isn't already running, and that the file is a
        /// FITS one containing a bitmap and a list of hot pixels.
        //--------------------------------------------------------------------------------
        bool setMasterDark(const std::string& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Set the parameters to use for background calibration
        ///
        /// This method is an aternative to 'processReferenceFrame()'.
        //--------------------------------------------------------------------------------
        void setParameters(
            const utils::background_calibration_parameters_t& parameters
        );

        //--------------------------------------------------------------------------------
        /// @brief  Process the reference frame
        ///
        /// It is expected that the thread isn't already running.
        ///
        /// Calling this method will start the thread.
        //--------------------------------------------------------------------------------
        bool processReferenceFrame(const std::string& lightFrame);

        //--------------------------------------------------------------------------------
        /// @brief  Process a list of light frame files
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
        void processNextFrame(bool reference);


    private:
        StackingListener* listener;
        std::filesystem::path destFolder;

        processing::LightFrameProcessor<BITMAP> processor;
        std::thread thread;

        std::vector<std::string> lightFrames;
        std::mutex mutex;
        std::condition_variable condition;

        bool cancelled = false;
        bool terminate = false;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/lightframes.hpp>
