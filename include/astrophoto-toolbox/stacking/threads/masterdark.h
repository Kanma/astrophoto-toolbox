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
#include <astrophoto-toolbox/stacking/processing/masterdark.h>
#include <filesystem>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to to generate the master dark
    //------------------------------------------------------------------------------------
    template<class BITMAP>
    class MasterDarkThread : public Thread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// The listener will be used to notify the caller when the master dark frame is
        /// computed.
        //--------------------------------------------------------------------------------
        MasterDarkThread(
            StackingListener* listener, const std::filesystem::path& destFilename,
            const std::filesystem::path& tempFolder
        );

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
        //--------------------------------------------------------------------------------
        void processFrames(const std::vector<std::filesystem::path>& darkFrames);


    private:
        void process() override;

        void onCancel() override;
        void onReset() override;


    private:
        StackingListener* listener;
        std::filesystem::path destFilename;

        processing::MasterDarkGenerator<BITMAP> generator;
        std::filesystem::path tempFolder;

        std::vector<std::filesystem::path> darkFrames;
    };

}
}
}

#include <astrophoto-toolbox/stacking/threads/masterdark.hpp>
