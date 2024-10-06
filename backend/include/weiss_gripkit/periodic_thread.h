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

#ifndef KR2_CBUN_PERIODIC_THREAD
#define KR2_CBUN_PERIODIC_THREAD

#include <functional>
#include <atomic>
#include <thread>

namespace kswx_weiss_gripkit {
    
    /// @brief Class for representing a thread that runs periodically with specified initialization and cycle code.
    class PeriodicThread
    {
    public:
        /// @brief Construct object to represent a thread that runs periodically with specified initialization and cycle code.
        /// ==== { init_method(); while (true) { cycle_method(); sleep(sleep_ms); } } ====
        /// @param init_method method to call once during initialization inside the thread
        /// @param cycle_method method to call every cycle inside the thread
        /// @param sleep_ms sleep per cycle in milliseconds
        PeriodicThread(std::function<void()> init_method, std::function<void()> cycle_method, int sleep_ms);

        inline virtual ~PeriodicThread() {}

        /// @brief Start the periodic thread. Caller needs to maintain proper call sequence (start, stop, start, stop), not (start, start) for example.
        /// @param timeout_ms timeout in milliseconds to start the thread
        /// @return true if thread successfully initialized within timeout_ms milliseconds, false otherwise. Thread not joined if false.
        bool start(int timeout_ms);

        /// @brief Stop the periodic thread. Caller needs to maintain proper call sequence (start, stop, start, stop), not (start, start) for example.
        /// @param timeout_ms timeout in milliseconds to stop the thread
        /// @return true if thread acknowledged stop request within timeout_ms milliseconds, false otherwise. Thread not joined if false.
        bool stop(int timeout_ms);

    private:
        std::function<void()> init_method_;
        std::function<void()> cycle_method_;
        int sleep_ms_;

        std::thread periodic_thread_;
        std::atomic<bool> stop_request_;
        std::atomic<bool> initialized_;
    };

} // namespace kswx_weiss_gripkit

#endif // KR2_CBUN_PERIODIC_THREAD