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
#include <astrophoto-toolbox/stacking/processing/registration.h>
#include <filesystem>
#include <string>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to perform all the registration-related operations (stars
    ///         detection and transformation from a reference frame computation)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class RegistrationThread : public Thread
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
        void setParameters(const star_list_t& stars, int luminancyThreshold);

        //--------------------------------------------------------------------------------
        /// @brief  Register the light frame file to use as the reference
        ///
        /// If not specified, the luminancy threshold is estimated on the reference frame
        /// and used as-is on all the other frames.
        ///
        /// If the destination file points to an existing FITS file, the list of detected
        /// stars is added to that file (which can be the same as the light frame one).
        ///
        /// It is expected that the light frame has been properly processed.
        //--------------------------------------------------------------------------------
        void processReferenceFrame(
            const std::string& lightFrame, int luminancyThreshold=-1
        );

        //--------------------------------------------------------------------------------
        /// @brief  Register a list of light frame files
        ///
        /// It is expected that the light frames have been properly processed.
        //--------------------------------------------------------------------------------
        void processFrames(const std::vector<std::string>& lightFrames);


    private:
        void process() override;


    private:
        StackingListener* listener;
        std::filesystem::path destFolder;

        RegistrationProcessor<BITMAP> processor;

        star_list_t stars;
        int luminancyThreshold = -1;
        std::string referenceFrame;
        std::vector<std::string> lightFrames;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/registration.hpp>
