/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/stacking/threads/thread.h>

using namespace astrophototoolbox;
using namespace stacking;
using namespace threads;


Thread::~Thread()
{
    mutex.lock();

    if (state == STATE_IDLE)
    {
        mutex.unlock();
        return;
    }

    if ((state == STATE_STARTING) || (state == STATE_RUNNING))
    {
        mutex.unlock();
        cancel();
    }
    else
    {
        mutex.unlock();
    }

    join();
}

//-----------------------------------------------------------------------------

bool Thread::start(std::latch* latch)
{
    mutex.lock();

    if (state != STATE_IDLE)
    {
        mutex.unlock();

        if (latch)
            latch->count_down();

        return false;
    }

    state = STATE_STARTING;
    this->latch = latch;

    mutex.unlock();

    thread = std::thread(&Thread::run, this);

    return true;
}

//-----------------------------------------------------------------------------

bool Thread::reset()
{
    mutex.lock();

    if ((state != STATE_STARTING) && (state != STATE_RUNNING))
    {
        mutex.unlock();
        return false;
    }

    state = STATE_RESETTING;

    onReset();

    std::latch latch(1);
    this->latch = &latch;

    mutex.unlock();
    condition.notify_one();

    latch.wait();
    this->latch = nullptr;

    return true;
}

//-----------------------------------------------------------------------------

bool Thread::cancel(std::latch* latch)
{
    mutex.lock();

    if ((state != STATE_STARTING) && (state != STATE_RUNNING))
    {
        mutex.unlock();

        if (latch)
            latch->count_down();

        return false;
    }

    state = STATE_CANCELLING;
    this->latch = latch;

    onCancel();

    mutex.unlock();
    condition.notify_one();

    return true;
}

//-----------------------------------------------------------------------------

bool Thread::stop(std::latch* latch)
{
    mutex.lock();

    if ((state != STATE_STARTING) && (state != STATE_RUNNING))
    {
        mutex.unlock();

        if (latch)
            latch->count_down();

        return false;
    }

    state = STATE_STOPPING;
    this->latch = latch;

    mutex.unlock();
    condition.notify_one();

    return true;
}

//-----------------------------------------------------------------------------

void Thread::join()
{
    mutex.lock();
    if (state == STATE_IDLE)
    {
        mutex.unlock();
        return;
    }
    mutex.unlock();

    if (thread.joinable())
        thread.join();

    state = STATE_IDLE;
}

//-----------------------------------------------------------------------------

void Thread::run()
{
    mutex.lock();

    if (state == STATE_STARTING)
    {
        if (latch)
        {
            latch->count_down();
            latch = nullptr;
        }

        state = STATE_RUNNING;
    }

    mutex.unlock();

    process();

    if (latch)
    {
        latch->arrive_and_wait();
        latch = nullptr;
    }
}
