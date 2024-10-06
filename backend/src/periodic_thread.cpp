/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2023, KR Soft s.r.o.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Kassow Robots nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

#include "weiss_gripkit/periodic_thread.h"

#include <unistd.h>

using namespace kswx_weiss_gripkit;


PeriodicThread::PeriodicThread(std::function<void()> init_method, std::function<void()> cycle_method, int sleep_ms) : 
init_method_(init_method), cycle_method_(cycle_method), sleep_ms_(sleep_ms), stop_request_(true), initialized_(false) {}


bool PeriodicThread::start(int timeout_ms)
{
    initialized_ = false;
    stop_request_ = false;

    periodic_thread_ = std::thread([this]() {
        init_method_();
        initialized_ = true;
        while (!stop_request_)
        {
            cycle_method_();
            usleep(1000 * sleep_ms_);
        }
        initialized_ = false;
    });

    for (int i = 0; i < timeout_ms && !initialized_; ++i)
        usleep(1000);

    return initialized_;
}

bool PeriodicThread::stop(int timeout_ms)
{
    stop_request_ = true;
    for (int i = 0; i < timeout_ms && initialized_; ++i)
        usleep(1000);
    
    if (initialized_)
    {
        return false;
    }
    else
    {
        if (periodic_thread_.joinable())
            periodic_thread_.join();

        return true;
    }
}