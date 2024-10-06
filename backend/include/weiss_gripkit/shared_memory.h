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

#ifndef KR2_CBUN_SHARED_MEMORY
#define KR2_CBUN_SHARED_MEMORY

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace kswx_weiss_gripkit {

    /// @brief Wrapper for shared memory object from boost library.
    /// @tparam data_t type of data to put in the shared memory, has to be default-constructible and can only contain data, not references.
    template <typename data_t>
    class SharedMemoryObject
    {
    public:
        /// @brief Wrapper for shared memory object from boost library.
        /// @param id string id for the shared memory
        inline SharedMemoryObject(std::string id) : id_(id), data_(NULL) {}

        /// @brief Create the shared memory object with id id_, to be called from the main process. Remove shared memory of the same name if it exists. data_t can only contain data, not references. 
        inline void create()
        {
            boost::interprocess::shared_memory_object::remove(id_.c_str());
            boost::interprocess::shared_memory_object shm_object(boost::interprocess::create_only, id_.c_str(), boost::interprocess::read_write);
            shm_object.truncate(sizeof(data_t));
            shm_region_ = boost::interprocess::mapped_region(shm_object, boost::interprocess::read_write);
            data_ = new (shm_region_.get_address()) data_t();
        }

        /// @brief Destroy the shared memory object with id id_, to be called from the main process.
        inline void destroy()
        {
            boost::interprocess::shared_memory_object::remove(id_.c_str());
        }

        /// @brief Open existing shared memory object with with id id_, to be called from other processes.
        inline void attach()
        {
            boost::interprocess::shared_memory_object shm_object(boost::interprocess::open_only, id_.c_str(), boost::interprocess::read_write);
            shm_object.truncate(sizeof(data_t));
            shm_region_ = boost::interprocess::mapped_region(shm_object, boost::interprocess::read_write);
            data_ = static_cast<data_t*>(shm_region_.get_address());
        }

        /// @brief Get data from shared memory. Should only be called after create (main process) or attach (other processes) and before destroy (main process).
        /// @return 
        inline data_t* getData()
        {
            return data_;
        }

    private:
        std::string id_;
        boost::interprocess::mapped_region shm_region_;
        data_t* data_;
    };

    /// @brief Access to data_t protected by boost's interprocess mutex.
    template <typename data_t>
    class SynchronizedData
    {
    public:
        /// @brief Get data, protected by interprocess mutex.
        inline data_t get()
        {
            {
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex_);
                return data_;
            }
        }

        /// @brief Set data, protected by interprocess mutex.
        inline void set(data_t data)
        {
            {
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex_);
                data_ = data;
            }
        }

        /// @brief Exchange data, protected by interprocess mutex.
        /// @param data new value to be set
        /// @return previous value
        inline data_t exchange(data_t data)
        {
            {
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex_);
                data_t ret_data = data_;
                data_ = data;
                return ret_data;
            }
        }

    private:
        data_t data_;
        boost::interprocess::interprocess_mutex mutex_;
    };

    /// @brief Counter protected by boost's interprocess mutex.
    class SynchronizedIncrement
    {
    public:
        /// @brief Counter protected by boost's interprocess mutex. Set value to 0.
        inline SynchronizedIncrement() : data_(0) {}

        /// @brief Increment and return new value, protected by interprocess mutex.
        inline uint64_t increment()
        {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex_);
            ++data_;
            return data_;
        }

        /// @brief Get current value, protected by interprocess mutex. 
        inline uint64_t get()
        {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex_);
            return data_;
        }

    private:
        uint64_t data_;
        boost::interprocess::interprocess_mutex mutex_;
    };

} // namespace kswx_weiss_gripkit

#endif // KR2_CBUN_SHARED_MEMORY