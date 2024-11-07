/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/processing/registration.h>
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
    /// @brief  Thread allowing to perform all the registration-related operations (stars
    ///         detection and transformation from a reference frame computation)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class RegistrationThread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the caller when a frame was processed, and
        /// the destination folder (will be created if necessary) is where the processed
        /// frame files will be saved.
        //--------------------------------------------------------------------------------
        RegistrationThread(StackingListener* listener, const std::filesystem::path& destFolder);

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        ///
        /// If the thread is still running, the processing will be cancelled.
        //--------------------------------------------------------------------------------
        ~RegistrationThread();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Set the parameters to use for the registration
        ///
        /// This method is an aternative to 'processReferenceFrame()'.
        //--------------------------------------------------------------------------------
        bool setParameters(const star_list_t& stars, int luminancyThreshold);

        //--------------------------------------------------------------------------------
        /// @brief  Register the light frame file to use as the reference
        ///
        /// If not specified, the luminancy threshold is estimated on the reference frame
        /// and used as-is on all the other frames.
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file (which can be the same as the light frame one).
        ///
        /// It is expected that the light frame has been properly processed, and that the
        /// thread isn't already running.
        ///
        /// Calling this method will start the thread.
        //--------------------------------------------------------------------------------
        bool processReferenceFrame(
            const std::string& lightFrame, int luminancyThreshold=-1
        );

        //--------------------------------------------------------------------------------
        /// @brief  Register a list of light frame files
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
        void processNextFrame(bool reference, int luminancyThreshold=-1);


    private:
        StackingListener* listener;
        std::filesystem::path destFolder;

        RegistrationProcessor<BITMAP> processor;
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

#include <astrophoto-toolbox/stacking/threads/registration.hpp>
