/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <string>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Interface to implement to monitor the progress of the stacking threads
    //------------------------------------------------------------------------------------
    class StackingListener
    {
    public:
        virtual void lightFrameProcessed(const std::string& filename, bool success) = 0;
        virtual void lightFrameRegistered(const std::string& filename, bool success) = 0;
        virtual void lightFramesStacked(const std::string& filename) = 0;
    };

}
}
}
