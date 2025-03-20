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
#include <astrophoto-toolbox/stacking/processing/lightframes.h>
#include <filesystem>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to perform all the operations related to light frames
    ///         (background calibration, dark frame substraction, ...)
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class LightFrameThread : public Thread
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
        /// It is expected that the file is a FITS one containing a bitmap and a list of
        /// hot pixels.
        //--------------------------------------------------------------------------------
        void setMasterDark(const std::filesystem::path& filename);

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
        //--------------------------------------------------------------------------------
        void processReferenceFrame(const std::filesystem::path& lightFrame);

        //--------------------------------------------------------------------------------
        /// @brief  Process a list of light frame files
        ///
        /// It is expected that a reference frame was already processed
        //--------------------------------------------------------------------------------
        void processFrames(const std::vector<std::filesystem::path>& lightFrames);


    protected:
        void process() override;


    private:
        StackingListener* listener;
        std::filesystem::path destFolder;

        processing::LightFrameProcessor<BITMAP> processor;

        std::filesystem::path masterDark;
        utils::background_calibration_parameters_t parameters;
        bool parametersValid = false;
        std::filesystem::path referenceFrame;
        std::vector<std::filesystem::path> lightFrames;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/lightframes.hpp>
