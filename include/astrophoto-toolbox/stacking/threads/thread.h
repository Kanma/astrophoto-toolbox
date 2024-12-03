/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <latch>


namespace astrophototoolbox {
namespace stacking {
namespace threads {

    //------------------------------------------------------------------------------------
    /// @brief  Thread allowing to perform all the registration-related operations (stars
    ///         detection and transformation from a reference frame computation)
    //------------------------------------------------------------------------------------
    class Thread
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        ///
        /// If the thread is still running, the processing will be cancelled.
        //--------------------------------------------------------------------------------
        ~Thread();


    public:
        //--------------------------------------------------------------------------------
        /// @brief  Start the thread
        //--------------------------------------------------------------------------------
        bool start(std::latch* latch = nullptr);

        //--------------------------------------------------------------------------------
        /// @brief  Reset the pending jobs (& cancel the current one if possible)
        //--------------------------------------------------------------------------------
        bool reset();

        //--------------------------------------------------------------------------------
        /// @brief  Cancel the processing
        ///
        /// The thread will be stopped as soon as possible, and pending jobs will be
        /// cleared.
        //--------------------------------------------------------------------------------
        bool cancel(std::latch* latch = nullptr);

        //--------------------------------------------------------------------------------
        /// @brief  Stop the processing
        ///
        /// The thread will be stopped once all pending jobs are done.
        //--------------------------------------------------------------------------------
        bool stop(std::latch* latch = nullptr);

        //--------------------------------------------------------------------------------
        /// @brief  Wait for the thread to terminate (either because all jobs are done,
        ///         or because the processing was cancelled)
        //--------------------------------------------------------------------------------
        void join();


    protected:
        void run();
        virtual void process() = 0;

        virtual void onCancel() {};
        virtual void onReset() {};


    protected:
        enum state_t
        {
            STATE_IDLE,
            STATE_STARTING,
            STATE_RUNNING,
            STATE_RESETTING,
            STATE_CANCELLING,
            STATE_STOPPING,
        };


    protected:
        std::thread thread;

        std::mutex mutex;
        std::condition_variable condition;
        std::latch* latch = nullptr;

        state_t state = STATE_IDLE;
    };

}
}
}
