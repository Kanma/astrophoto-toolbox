/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/threads/masterdark.h>
#include <astrophoto-toolbox/stacking/threads/lightframes.h>
#include <astrophoto-toolbox/stacking/threads/registration.h>
#include <astrophoto-toolbox/stacking/threads/stacking.h>
#include <filesystem>
#include <vector>


namespace astrophototoolbox {
namespace stacking {


    //------------------------------------------------------------------------------------
    /// @brief  Contains infos about a light frame used in live stacking
    ///
    /// Sent to the user via a listener
    //------------------------------------------------------------------------------------
    struct live_stacking_light_frame_t
    {
        std::filesystem::path filename;
        bool calibrated = false;
        bool registered = false;
        bool stacked = false;
        bool valid = true;
        bool processing = false;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Contains infos about the progress of the live stacking
    ///
    /// Sent to the user via a listener
    //------------------------------------------------------------------------------------
    struct live_stacking_infos_t
    {
        unsigned int nbDarkFrames = 0;

        struct {
            unsigned int nbProcessed = 0;
            unsigned int nbRegistered = 0;
            unsigned int nbValid = 0;
            unsigned int nbStacking = 0;
            unsigned int nbStacked = 0;
            unsigned int nb = 0;
            std::vector<live_stacking_light_frame_t> entries;
        } lightFrames;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Class to implement to receive notifications about the progress of the
    ///         stacking
    //------------------------------------------------------------------------------------
    class LiveStackingListener
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Called each time the status of a frame changed (processed, registered,
        ///         stacked, ...)
        //--------------------------------------------------------------------------------
        virtual void progressNotification(const live_stacking_infos_t& infos) = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Called when a new stacked image is available
        //--------------------------------------------------------------------------------
        virtual void stackingDone(const std::filesystem::path& filename) = 0;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Allows to perform live stacking
    ///
    /// The implementation is based on several threads, each specialized in a specific
    /// part of the processing.
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class LiveStacking : public threads::StackingListener
    {
    public:
        ~LiveStacking();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Setup the live stacking with a specific folder to work in and the
        ///         listener to use
        ///
        /// During stacking, calibrated images, masters and temporary files will be stored
        /// in this folder.
        ///
        /// If not specified, the luminancy threshold is estimated on the reference frame
        /// and used as-is on all the other frames.
        //--------------------------------------------------------------------------------
        bool setup(
            LiveStackingListener* listener, const std::filesystem::path& folder,
            int luminancyThreshold=-1
        );

        //--------------------------------------------------------------------------------
        /// @brief  Load the list of images from a configuration file ('stacking.txt' in
        ///         the working folder)
        //--------------------------------------------------------------------------------
        bool load();

        //--------------------------------------------------------------------------------
        /// @brief  Save the list of images into a configuration file ('stacking.txt' in
        ///         the working folder)
        //--------------------------------------------------------------------------------
        bool save();

        //--------------------------------------------------------------------------------
        /// @brief  Add a dark frame
        ///
        /// Note: Calling this method when the stacking is running invalidates all the
        /// light frames already processed, which will need to be reprocessed again.
        //--------------------------------------------------------------------------------
        bool addDarkFrame(const std::filesystem::path& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Add a light frame
        ///
        /// By default, the first one is considered as the reference one.
        //--------------------------------------------------------------------------------
        bool addLightFrame(const std::filesystem::path& filename);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the infos about the progress of the stacking
        //--------------------------------------------------------------------------------
        inline live_stacking_infos_t getInfos()
        {
            std::lock_guard<std::mutex> lock(framesMutex);
            return infos;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Set the light frame to use as the reference during stacking
        ///
        /// Note: changing it invalidates all the light frames already processed, which
        /// will need to be reprocessed again.
        //--------------------------------------------------------------------------------
        void setReference(size_t index);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the index of the reference light frame
        //--------------------------------------------------------------------------------
        inline size_t getReference()
        {
            std::lock_guard<std::mutex> lock(framesMutex);
            return referenceFrame;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Set the luminancy threshold to use
        ///
        /// Note: changing it invalidates all the light frames already registered, which
        /// will need to be registered again.
        //--------------------------------------------------------------------------------
        void setLuminancyThreshold(int threshold);

        //--------------------------------------------------------------------------------
        /// @brief  Start the live stacking
        ///
        /// This method can be repeatedly called during live stacking.
        //--------------------------------------------------------------------------------
        bool start();

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the live stacking (blocking call)
        ///
        /// This method will block until no thread is working anymore. Once this method
        /// returns, the stacker can be used again (to continue the processing or start
        /// another one).
        //--------------------------------------------------------------------------------
        void cancel();

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the live stacking
        ///
        /// It is expected that no other method than 'wait()' is called after this one.
        //--------------------------------------------------------------------------------
        void cancelAsync();

        //--------------------------------------------------------------------------------
        /// @brief  Stop the live stacking (blocking call)
        ///
        /// This method will block until no thread is working anymore. Once this method
        /// returns, the stacker can be used again (to continue the processing or start
        /// another one).
        //--------------------------------------------------------------------------------
        void stop();

        //--------------------------------------------------------------------------------
        /// @brief  Stop the live stacking
        ///
        /// It is expected that no other method than 'wait()' is called after this one.
        //--------------------------------------------------------------------------------
        void stopAsync();

        //--------------------------------------------------------------------------------
        /// @brief  Wait for the live stacking to terminate (either because all frames
        ///         are processed, or because it was cancelled)
        ///
        /// This method will block until no thread is working anymore. Once this method
        /// returns, the stacker can be used again (to continue the processing or start
        /// another one).
        //--------------------------------------------------------------------------------
        void wait();


        //_____ Implementation of threads::StackingListener __________
    public:
        void masterDarkFrameComputed(const std::filesystem::path& filename, bool success) override;

        void lightFrameProcessingStarted(const std::filesystem::path& filename) override;
        void lightFrameProcessed(const std::filesystem::path& filename, bool success) override;

        void lightFrameRegistrationStarted(const std::filesystem::path& filename) override;
        void lightFrameRegistered(const std::filesystem::path& filename, bool success) override;

        void lightFramesStackingStarted(unsigned int nbFrames) override;
        void lightFramesStacked(const std::filesystem::path& filename, unsigned int nbFrames) override;


    private:
        void nextStep();

        const std::filesystem::path getInternalFilename(const std::filesystem::path& path) const;
        const std::filesystem::path getAbsoluteFilename(const std::filesystem::path& path) const;
        const std::filesystem::path getCalibratedFilename(const std::filesystem::path& path) const;


    private:
        struct dark_frame_t
        {
            std::filesystem::path filename;
            bool stacked = false;
            bool processing = false;
        };

        enum step_t
        {
            STEP_NONE,
            STEP_MASTER_DARK,
            STEP_STACKING,
        };


    private:
        LiveStackingListener* listener = nullptr;
        live_stacking_infos_t infos;

        std::filesystem::path folder;

        std::vector<dark_frame_t> darkFrames;
        std::mutex framesMutex;

        size_t referenceFrame = -1;
        int luminancyThreshold = -1;

        step_t step = STEP_NONE;
        bool running = false;

        threads::MasterDarkThread<BITMAP>* masterDarkThread = nullptr;
        threads::LightFrameThread<BITMAP>* lightFramesThread = nullptr;
        threads::RegistrationThread<BITMAP>* registrationThread = nullptr;
        threads::StackingThread<BITMAP>* stackingThread = nullptr;

        std::thread stopThread;
    };

}
}


#include <astrophoto-toolbox/stacking/livestacking.hpp>
