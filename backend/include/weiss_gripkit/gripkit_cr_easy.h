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

#ifndef KR2_CBUN_GRIPKIT_CR_EASY
#define KR2_CBUN_GRIPKIT_CR_EASY

#include "weiss_gripkit/value_monitor.h"
#include "weiss_gripkit/shared_memory.h"

#include <kr2_program_api/api_v1/bundles/custom_device.h>
#include <atomic>

#define INPUT_HIGH_VOLTAGE 12
#define SHM_GLOBAL_ID "kswx_weiss_gripkit.gkeasy"
#define NO_LOAD kr2_program_api::Load(0.0, kr2_program_api::Position(0.0, 0.0, 0.0), kr2_program_api::Imx(0.001, 0.001, 0.001, 0.0, 0.0, 0.0))
#define MIN_CONTINUOUS_ERROR_COUNT 4
#define US_SLEEP_GRIP_RELEASE 10000

namespace kswx_weiss_gripkit {
    
    /// @brief Structure containing load data, can be used in shared memory.
    struct LoadData
    {
        /// @brief Constructor for empty load (small values in inertial matrix).
        inline LoadData() : LoadData(NO_LOAD) {}

        /// @brief Construct LoadData from data in a specified Load.
        explicit inline LoadData(kr2_program_api::Load load)
        {
            mass = load.mass().d();

            x = load.cog().x().d();
            y = load.cog().y().d();
            z = load.cog().z().d();

            xx = load.imx().xx().d();
            yy = load.imx().yy().d();
            zz = load.imx().zz().d();
            xy = load.imx().xy().d();
            xz = load.imx().xz().d();
            yz = load.imx().yz().d();
        }

        /// @brief Create Load object based on saved LoadData.
        kr2_program_api::Load toLoad()
        {
            return kr2_program_api::Load(mass, kr2_program_api::Position(x, y, z), kr2_program_api::Imx(xx, yy, zz, xy, xz, yz));
        }

        double mass;
        double x, y, z;
        double xx, yy, zz, xy, xz, yz;
    };

    /// @brief Gripper actions.
    enum class GripkitAction { GRIP, RELEASE, NONE };

    /// @brief Gripkit cbun exception.
    class GripkitException : public std::exception {
    public:

        /// @brief Create the gripkit cbun exception.
        /// @param msg exception message
        inline GripkitException(const std::string &msg)
        {
            msg_ = std::string("kswx_weiss_gripkit::GripkitException: " + msg);
        }
        
        /// @brief Get the exception message.
        inline const char* what() const throw() {
            return msg_.c_str();
        }
        
    protected:
        std::string msg_;
    };

    /// @brief Status of the CrEasy gripper and CBun.
    enum class GripkitCrEasyStatus
    {
        /// @brief gripper deactivated or gripper error present
        IDLE_OR_ERROR,

        /// @brief workpiece released
        RELEASED,

        /// @brief no workpiece detected
        NO_PART,

        /// @brief workpiece gripped
        HOLDING,

        /// @brief error while reading the status
        STATUS_ERROR
    };

    /// @brief Class implementing the Gripkit CrEasy gripper device.
    class GripkitCrEasy : public kr2_bundle_api::CustomDevice {
    public:

        GripkitCrEasy(boost::shared_ptr<kr2_program_api::ProgramInterface> api, const boost::property_tree::ptree &xml_bundle_node);
        virtual ~GripkitCrEasy() = default;




        // fixed bundle class methods

        /// @brief Create and initialize shared memory. Subscribe to HWReady for re-activation after restart.
        /// @return always 0
        virtual int onCreate();

        /// @brief Deactivate the CBun and destroy shared memory.
        /// @return always 0
        virtual int onDestroy();

        /// @brief Attach to shared memory, set activated to true.
        /// @return always 0
        virtual int onBind();

        /// @brief Set activated to false.
        /// @return always 0
        virtual int onUnbind();

        /// @brief Re-activate and re-mount after restart. Publish error/exception if any are received during activation/mounting.
        void onHWReady(const kr2_signal::HWReady&);




        // custom methods

        /// @brief Move to the predefined grip (no part limit) position. Optionally set "payload" as the system payload variable (LOAD2) if gripper detects
        /// a part (no part limit not reached), otherwise clear LOAD2. Load set in relation to the tool flange.
        /// @param blocking block the method until the gripper is in state HOLDING or NO_PART or sooner if the call was interrupted by another call or IDLE_OR_ERROR state.
        /// @param payload optional payload, the value is used to update the LOAD2 system variable if and when gripper detects a part.
        /// @return ok on success, error if not activated, exception if internal error or bad status occurred 
        virtual CBUN_PCALL grip(bool blocking, boost::optional<kr2_program_api::Load> payload);

        /// @brief Move to the predefined release position. Clear the system payload variable (LOAD2).
        /// @param blocking block the method until the gripper is in state RELEASED or sooner if the call was interrupted by another call or IDLE_OR_ERROR state.
        /// @return ok on success, error if not activated, exception if internal error or bad status occurred 
        virtual CBUN_PCALL release(bool blocking);




        // custom functions

        /// @brief Return 1 if gripper status is RELEASED, 0 otherwise.
        virtual kr2_program_api::Number isReleased();

        /// @brief Return 1 if gripper status is HOLDING, 0 otherwise.
        virtual kr2_program_api::Number isHolding();

        /// @brief Return 1 if gripper status is NO_PART, 0 otherwise.
        virtual kr2_program_api::Number isNoPart();

        /// @brief Return 1 if gripper status is IDLE_OR_ERROR, 0 otherwise.
        virtual kr2_program_api::Number isError();



        
    protected:
    
        /// @brief Process activation parameters, enable and set digital pin outputs for gripper communication, start status monitoring thread 
        /// and set activated to true.
        /// @return error on error, ok otherwise
        virtual CBUN_PCALL onActivate(const boost::property_tree::ptree &param_tree);

        /// @brief Set activated to false, stop monitoring thread, disable and unset digital pin outputs.
        /// @return error on error, ok otherwise
        virtual CBUN_PCALL onDeactivate();

        /// @brief Set toolload from parameters.
        /// @return always ok
        virtual CBUN_PCALL onMount(const boost::property_tree::ptree &param_tree);

        /// @brief Reset toolload to no load.
        /// @return always ok
        virtual CBUN_PCALL onUnmount();
    
    private:

        /// @brief Parse activation parameters.
        bool processActivationParams(const boost::property_tree::ptree &tree);

        /// @brief Common method for performing GRIP or RELEASE. Check activation, share load to shared memory for GRIP to be set on move finished,
        /// increment request id, send request (processed in onTick called from value_monitor_), and if blocking wait for finish or interrupt.
        /// @param action action to perform, GRIP or RELEASE
        /// @param blocking True for a blocking call, returns after move is finished or sooner if interrupted by another grip/release call.
        /// @param payload Payload to set if gripper detects part - will be set after the move finishes, which can be after non-blocking call returns.
        /// @return ok on success, error if not activated, exception if internal error or bad status occurred 
        CBUN_PCALL performActionCommon(GripkitAction action, bool blocking, boost::optional<kr2_program_api::Load> payload);

        /// @brief Read gripper status from shared memory and return in; Throw GripkitException on failure to access shared memory.
        GripkitCrEasyStatus getStatusSharedMemory();

        /// @brief Common method for status requests. Read gripper status from shared memory and return 1 if it matches checkedStatus, 0 otherwise.
        kr2_program_api::Number isCommon(GripkitCrEasyStatus checkedStatus);

        /// @brief Read gripped and no_error pins from gpio. Return STATUS_ERROR, if the values were not found. Otherwise return the actual status.
        /// If called from multiple processes, method may fail - only called in the status monitoring thread.
        /// @return gripper status or STATUS_ERROR if status could not be read
        GripkitCrEasyStatus getStatus();

        /// @brief Called by value_monitor_ when gripper status changes. Set payload from shared memory if status changed to HOLDING.
        /// Set no payload if status changed to NO_PART or RELEASED.
        void onStatusChange(GripkitCrEasyStatus newStatus);

        /// @brief Called by value_monitor_ in every loop cycle. Update gripper status in shared memory and 
        /// process gripper action requests (GRIP/RELEASE or NONE for no request).
        void onTick(GripkitCrEasyStatus newStatus);

        /// @brief Set digital output identified by its DUID to specified state and configuration.
        /// @param gpio_id DUID, id of the gpio pin
        /// @param state true for on, false for off
        /// @param config pin configuration (0/24V, 0/12V, disabled, ...)
        /// @return true on success, false otherwise
        bool setDigitalOutput(kr2rc_api::DUID gpio_id, bool state, unsigned int config);

        /// @brief Set GPIO DUIDs based on robot generation.
        /// @return true on success (generation supported), false otherwise
        bool setupGPIO(int robot_generation);






        /// @brief pointer to the system tool load variable
        boost::shared_ptr<kr2_program_api::Load> toolload_;

        /// @brief pointer to the system payload variable
        boost::shared_ptr<kr2_program_api::Load> payload_;



        /// @brief thread-safe activated flag, true after onActivate (activates gripper communication) and 
        /// onBind (CBun must be activated for program to start), false after onDeactivate and onUnbind.
        std::atomic<bool> activated_;

        /// @brief thread-safe mounted flag
        std::atomic<bool> mounted_;



        /// @brief shared memory for sharing load from sequences to master instance, 
        /// master instance sets load on gripper status change, so that non-blocking sequence calls can return
        SharedMemoryObject<SynchronizedData<LoadData>> shm_load_;

        /// @brief shared memory for sharing status from master instance (reads status periodically in value_monitor_) to sequences
        SharedMemoryObject<SynchronizedData<GripkitCrEasyStatus>> shm_status_;

        /// @brief shared memory for requesting action (GRIP/RELEASE), sequence requests and master instance processes the request.
        SharedMemoryObject<SynchronizedData<GripkitAction>> shm_action_;

        /// @brief shared memory for current request id, sequences use this to find out when their request has been interrupted.
        SharedMemoryObject<SynchronizedIncrement> shm_request_id_;



        // DUIDs and config ids for gpio communication with gripper.
        struct {
            uint32_t duid_out_power_;
            uint32_t duid_out_activation_;
            uint32_t duid_out_grip_;

            uint32_t duid_in_gripped_;
            uint32_t duid_in_no_error_;

            unsigned int config_enabled_;
            unsigned int config_disabled_;
        } gpio_setup_;

        /// @brief status monitoring thread
        ValueMonitor<GripkitCrEasyStatus> value_monitor_;
    };

} // namespace kswx_weiss_gripkit

#endif // KR2_CBUN_GRIPKIT_CR_EASY