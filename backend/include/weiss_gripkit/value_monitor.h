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

#ifndef KR2_CBUN_VALUE_MONITOR
#define KR2_CBUN_VALUE_MONITOR

#include "weiss_gripkit/periodic_thread.h"

#include <functional>
#include <atomic>
#include <thread>
#include <iostream>
#include <unistd.h>

namespace kswx_weiss_gripkit {
    
    /// @brief Class for monitoring a value and calling on_value_changed method on value change. Starts a separate thread.
    /// @tparam value_t type of value to monitor
    template <typename value_t>
    class ValueMonitor
    {
    public:
        /// @brief Construct a thread that periodically checks a value using get_value and reports changes using on_value_changed.
        /// @param get_value_function periodically called to get value
        /// @param on_value_changed_method called with new value when value changed since last get_value call
        /// @param tick_method called each cycle with value from get_value call
        /// @param sleep_ms milliseconds to sleep each cycle
        ValueMonitor(std::function<value_t()> get_value_function, std::function<void(value_t)> on_value_changed_method, std::function<void(value_t)> tick_method, int sleep_ms);

        inline virtual ~ValueMonitor() {}

        /// @brief Start the monitoring thread with specified timeout. Caller needs to maintain proper call sequence (start, stop, start, stop), not (start, start) for example.
        /// @param timeout_ms timeout in milliseconds to start the thread
        /// @return true if thread successfully initialized within timeout_ms milliseconds, false otherwise. Thread not joined if false.
        inline bool start(int timeout_ms) { return periodic_thread_.start(timeout_ms); }

        /// @brief Stop the monitoring thread with specified timeout. Caller needs to maintain proper call sequence (start, stop, start, stop), not (start, start) for example.
        /// @param timeout_ms timeout in milliseconds to stop the thread
        /// @return true if thread acknowledged stop request within timeout_ms milliseconds, false otherwise. Thread not joined if false.
        inline bool stop(int timeout_ms) { return periodic_thread_.stop(timeout_ms); }

    private:
        /// @brief Helper method for PeriodicThread.
        void init();

        /// @brief Helper method for PeriodicThread.
        void cycle();

        std::function<value_t()> get_value_function_;
        std::function<void(value_t)> on_value_changed_method_;
        std::function<void(value_t)> tick_method_;
        
        PeriodicThread periodic_thread_;
        value_t last_value_;
    };

    template <typename value_t>
    void ValueMonitor<value_t>::init()
    {
        last_value_ = get_value_function_();   
    }

    template <typename value_t>
    void ValueMonitor<value_t>::cycle()
    {
        value_t act_value = get_value_function_();
        tick_method_(act_value);
        if (last_value_ != act_value)
        {
            on_value_changed_method_(act_value);
        }
        last_value_ = act_value;  
    }

    template <typename value_t>
    ValueMonitor<value_t>::ValueMonitor(std::function<value_t()> get_value_function, std::function<void(value_t)> on_value_changed_method, std::function<void(value_t)> tick_method, int sleep_ms) :
    get_value_function_(get_value_function), on_value_changed_method_(on_value_changed_method), tick_method_(tick_method),
    periodic_thread_([this]() { init(); }, [this]() { cycle(); }, sleep_ms) {}

} // namespace kswx_weiss_gripkit

#endif // KR2_CBUN_VALUE_MONITOR