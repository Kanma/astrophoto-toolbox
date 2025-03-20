/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <filesystem>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Interface to implement to monitor the progress of the stacking threads
    //------------------------------------------------------------------------------------
    class StackingListener
    {
    public:
        virtual void masterDarkFrameComputed(const std::filesystem::path& filename, bool success) = 0;

        virtual void lightFrameProcessingStarted(const std::filesystem::path& filename) = 0;
        virtual void lightFrameProcessed(const std::filesystem::path& filename, bool success) = 0;

        virtual void lightFrameRegistrationStarted(const std::filesystem::path& filename) = 0;
        virtual void lightFrameRegistered(const std::filesystem::path& filename, bool success) = 0;

        virtual void lightFramesStackingStarted(unsigned int nbFrames) = 0;
        virtual void lightFramesStacked(const std::filesystem::path& filename, unsigned int nbFrames) = 0;
    };

}
}
}
