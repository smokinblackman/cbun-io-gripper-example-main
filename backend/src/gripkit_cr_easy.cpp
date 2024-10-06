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

#include "weiss_gripkit/gripkit_cr_easy.h"
#include "weiss_gripkit/logging.h"

#include <kr2_program_api/api_v1/bundles/arg_provider_xml.h>
#include <kr2_program_api/api_v1/cbun/xmlrpc/xmlrpc_server.h>

using namespace kswx_weiss_gripkit;

// The class has to be registered, otherwise the robot user will not be able
// to create an instance of the device (ie. he would not be able to Apply it).
REGISTER_CLASS(kswx_weiss_gripkit::GripkitCrEasy)

GripkitCrEasy::GripkitCrEasy(boost::shared_ptr<kr2_program_api::ProgramInterface> a_api,
                                       const boost::property_tree::ptree &a_xml_bundle_node)
:   kr2_bundle_api::CustomDevice(a_api, a_xml_bundle_node),
    activated_(false),
    mounted_(false),
    shm_load_(SHM_GLOBAL_ID + std::string(".load")), 
    shm_status_(SHM_GLOBAL_ID + std::string(".status")),
    shm_action_(SHM_GLOBAL_ID + std::string(".action")),
    shm_request_id_(SHM_GLOBAL_ID + std::string(".request_increment")),
    value_monitor_(
        [this]() { return getStatus(); },
        [this] (GripkitCrEasyStatus newStatus) { onStatusChange(newStatus); },
        [this] (GripkitCrEasyStatus newStatus) { onTick(newStatus); }, 10
        )
{
    // load system's variables for tool load and payload 
    toolload_ = api_->variables_->allocSystemLoad("toolload", kr2rc_api::Load::SysId::LOAD_TOOL);
    payload_ = api_->variables_->allocSystemLoad("payload", kr2rc_api::Load::SysId::LOAD_PAYLOAD);

    // register methods so they can be called from the master thread
    REGISTER_RPC(&GripkitCrEasy::grip, this, ARG_BOOL(0), ARG_LOAD_OPT(1))
    REGISTER_RPC(&GripkitCrEasy::release, this, ARG_BOOL(0))


    kr2_xmlrpc::XmlRpcServer server;

    class SetGripperMethod : public kr2_xmlrpc::Method {
    public:

        GripkitCrEasy* device_;

        SetGripperMethod(GripkitCrEasy* device)
        : device_(device)
        {}

        kr2_xmlrpc::Value execute(const kr2_xmlrpc::Params& a_params) {
            LOG_INFO("RPC/Settings gripper to: " << (a_params.getBool(0) ? "grip" : "release"));

            try 
            {
                if (device_->activated_)
                {
                    if (a_params.getBool(0))
                    {
                        device_->grip(false, boost::optional<kr2_program_api::Load>(NULL));
                    }
                    else
                    {
                        device_->release(false);
                    }
                    return kr2_xmlrpc::Value::Int(1);
                }
                else
                {
                    return kr2_xmlrpc::Value::Int(0);
                }
            }
            catch (std::exception)
            {
                return kr2_xmlrpc::Value::Int(-1);
            }
        }
        
        
    };
    server.addMethod("setGripper", boost::shared_ptr<SetGripperMethod>(new SetGripperMethod(this)));

    class GetStatusMethod : public kr2_xmlrpc::Method {
    public:

        GripkitCrEasy* device_;

        GetStatusMethod(GripkitCrEasy* device)
        : device_(device)
        {}

        kr2_xmlrpc::Value execute(const kr2_xmlrpc::Params& a_params) {
            std::map<std::string, kr2_xmlrpc::Value> values;
            try
            {                
                GripkitCrEasyStatus status;
                if (device_->activated_)
                    status = device_->getStatusSharedMemory();
                else
                    status = GripkitCrEasyStatus::IDLE_OR_ERROR;

                int status_int;
                if (status == GripkitCrEasyStatus::IDLE_OR_ERROR)
                    status_int = 0;
                if (status == GripkitCrEasyStatus::HOLDING)
                    status_int = 1;
                if (status == GripkitCrEasyStatus::NO_PART)
                    status_int = 2;
                if (status == GripkitCrEasyStatus::RELEASED)
                    status_int = 3;

                values.emplace("success", kr2_xmlrpc::Value::Int(1));
                values.emplace("status", kr2_xmlrpc::Value::Int(status_int));
                values.emplace("activated", kr2_xmlrpc::Value::Int(device_->activated_ ? 1 : 0));
                values.emplace("mounted", kr2_xmlrpc::Value::Int(device_->mounted_ ? 1 : 0));

                return kr2_xmlrpc::Value::Struct(values);
            }
            catch(std::exception)
            {
                values.emplace("success", kr2_xmlrpc::Value::Int(0));
                return kr2_xmlrpc::Value::Struct(values);
            }
        }
        
        
    };
    server.addMethod("getStatus", boost::shared_ptr<GetStatusMethod>(new GetStatusMethod(this)));
}


int GripkitCrEasy::onCreate()
{
    SUBSCRIBE(kr2_signal::HWReady, GripkitCrEasy::onHWReady);
    
    // create shared memory objects for interprocess communication
    shm_load_.create();
    shm_status_.create();
    shm_action_.create();
    shm_request_id_.create();
    
    SynchronizedData<GripkitAction>* shm_action_sync = shm_action_.getData();
    if (shm_action_sync)
    {
        shm_action_sync->set(GripkitAction::NONE);
    }
    
    return 0;
}

int GripkitCrEasy::onDestroy()
{
    onDeactivate();

    // destroy shared memory objects
    shm_load_.destroy();
    shm_status_.destroy();
    shm_action_.destroy();
    shm_request_id_.destroy();

    return 0;
}

int GripkitCrEasy::onBind()
{
    // attach to shared memory objects for interprocess communication
    shm_load_.attach();
    shm_status_.attach();
    shm_action_.attach();
    shm_request_id_.attach();

    // Program will only launch if CBun is activated, thus we know CBun is activated in onBind
    activated_ = true;

    return 0;
}

int GripkitCrEasy::onUnbind()
{
    activated_ = false;

    return 0;
}


void GripkitCrEasy::onHWReady(const kr2_signal::HWReady&)
{
    // automatically activate after reboot (if activated before)
    if (activation_tree_)
    {
        kr2_program_api::CmdResult<> result = activate(*activation_tree_);
        switch (result.result_)
        {
            case kr2_program_api::CmdResult<>::EXCEPTION:
                PUBLISH_EXCEPTION(result.code_, result.message_)
                break;
            case kr2_program_api::CmdResult<>::ERROR:
                PUBLISH_ERROR(result.code_, result.message_)
                break;
        }
    }

    // automatically mount after reboot (if mounted before)
    if (mounting_tree_)
    {
        kr2_program_api::CmdResult<> result = mount(*mounting_tree_);
        switch (result.result_)
        {
            case kr2_program_api::CmdResult<>::EXCEPTION:
                PUBLISH_EXCEPTION(result.code_, result.message_)
                break;
            case kr2_program_api::CmdResult<>::ERROR:
                PUBLISH_ERROR(result.code_, result.message_)
                break;
        }
    }
}

bool GripkitCrEasy::setDigitalOutput(kr2rc_api::DUID gpio_id, bool state, unsigned int config)
{
    std::array<kr2rc_api::IOData::GPIOInt64,1> digital_io {{ gpio_id, (state ? 1 : 0), config}};

    kr2rc_api::IOData::CmdTXGPIOParams params;
    kr2rc_api::CmdResult result = api_->rc_api_->iob_data_->cmd_TX_GPIO(params, nullptr, 0, digital_io.data(), digital_io.size(), nullptr, 0);

    if (result.err_code_ != 0)
        return false;

    return true;
}


CBUN_PCALL GripkitCrEasy::onActivate(const boost::property_tree::ptree &a_param_tree)
{    

    // process activation params
    if (!processActivationParams(a_param_tree)) {
        LOG_ERR("Invalid activation params");
        CBUN_PCALL_RET_ERROR(-1, "Invalid activation parameters.");
    }

    // enable power output and set to true
    if (!setDigitalOutput(gpio_setup_.duid_out_power_, true, gpio_setup_.config_enabled_))
    {
        LOG_ERR("Unable to set digital output for power (VCC) to true.");
        CBUN_PCALL_RET_ERROR(-1, "Unable to enable power supply.");
    }

    // enable activation pin and set to true
    if (!setDigitalOutput(gpio_setup_.duid_out_activation_, true, gpio_setup_.config_enabled_))
    {
        LOG_ERR("Unable to set digital output for activation (IN0) to true.");
        CBUN_PCALL_RET_ERROR(-1, "Unable to activate device.");
    }

    // enable grip pin and set to false
    if (!setDigitalOutput(gpio_setup_.duid_out_grip_, false, gpio_setup_.config_enabled_))
    {
        LOG_ERR("Unable to set digital output for grip (IN1) to false.");
        CBUN_PCALL_RET_ERROR(-1, "Unable to activate device.");
    }

    // start status monitoring thread
    if (!value_monitor_.start(500))
    {
        LOG_ERR("Unable to start monitor thread.");
        CBUN_PCALL_RET_ERROR(-1, "Unable to activate device.");
    }

    activated_ = true;
    
    CBUN_PCALL_RET_OK;
}

CBUN_PCALL GripkitCrEasy::onDeactivate()
{    
    activated_ = false;

    // stop status monitoring thread
    if (!value_monitor_.stop(500))
    {
        LOG_ERR("Unable to stop monitor thread.");
    }

    // disable grip pin and set to false
    if (!setDigitalOutput(gpio_setup_.duid_out_grip_, false, gpio_setup_.config_disabled_))
    {
        LOG_ERR("Unable to set digital output for grip (IN1) to false.");
    }

    // disable activation pin and set to false
    if (!setDigitalOutput(gpio_setup_.duid_out_activation_, false, gpio_setup_.config_disabled_))
    {
        LOG_ERR("Unable to set digital output for activation (IN0) to false.");
    }

    // disable power pin and set to false
    if (!setDigitalOutput(gpio_setup_.duid_out_power_, false, gpio_setup_.config_disabled_))
    {
        LOG_ERR("Unable to set digital output for power (VCC) to false.");
    }
    
    CBUN_PCALL_RET_OK;
}

void GripkitCrEasy::onStatusChange(GripkitCrEasyStatus newStatus)
{
    // set payload no none if gripper is released or detected no part
    if (newStatus == GripkitCrEasyStatus::NO_PART || newStatus == GripkitCrEasyStatus::RELEASED)
    {
        *payload_ = NO_LOAD;
    }

    // set payload to load from shared memory if gripper started holding a part
    if (newStatus == GripkitCrEasyStatus::HOLDING)
    {
        SynchronizedData<LoadData>* shm_load_sync = shm_load_.getData();
        if (shm_load_sync)
        {
            *payload_ = shm_load_sync->get().toLoad();
        }
        else
        {
            LOG_ERR("shm_load_sync not initialized");
        }
    }
}

void GripkitCrEasy::onTick(GripkitCrEasyStatus newStatus)
{
    // update status in shared memory
    SynchronizedData<GripkitCrEasyStatus>* status = shm_status_.getData(); 
    if (status) 
    {
        status->set(newStatus);
    }

    // read from shared memory and perform requested action: grip/release
    SynchronizedData<GripkitAction>* shm_action_sync = shm_action_.getData();
    if (shm_action_sync)
    {
        GripkitAction requestedAction = shm_action_sync->exchange(GripkitAction::NONE);
        if (requestedAction == GripkitAction::GRIP || requestedAction == GripkitAction::RELEASE)
        {
            if (!setDigitalOutput(gpio_setup_.duid_out_grip_, requestedAction == GripkitAction::GRIP, gpio_setup_.config_enabled_))
            {
                LOG_ERR("Unable to set digital output for grip (IN1) to " << ((requestedAction == GripkitAction::GRIP) ? "true." : "false."));
            }
        }
    }
}

CBUN_PCALL GripkitCrEasy::onMount(const boost::property_tree::ptree &a_param_tree)
{
    // get load from parameters
    kr2_bundle_api::ArgProviderXml arg_provider(a_param_tree);
    kr2_program_api::Load load = arg_provider.getLoad(0);

    // set the system tool load
    *toolload_ = load;

    mounted_ = true;
    
    CBUN_PCALL_RET_OK;
}

CBUN_PCALL GripkitCrEasy::onUnmount()
{
    *toolload_ = NO_LOAD;

    mounted_ = false;
    
    CBUN_PCALL_RET_OK;
}

CBUN_PCALL GripkitCrEasy::performActionCommon(GripkitAction action, bool blocking, boost::optional<kr2_program_api::Load> payload)
{
    // check activation
    if (!activated_)
    {
        LOG_ERR("CBun not activated.");
        CBUN_PCALL_RET_ERROR(-1, "CBun not activated. Activate CBun.");
    }

    // set requested load to shared memory for action==GRIP
    if (action == GripkitAction::GRIP)
    {
        SynchronizedData<LoadData>* shm_load_sync = shm_load_.getData();
        if (shm_load_sync)
        {
            if (payload && payload->valid())
                shm_load_sync->set(LoadData(*payload));
            else
                shm_load_sync->set(LoadData(NO_LOAD));
        }
        else
        {
            LOG_ERR("shm_load_sync not initialized");
            CBUN_PCALL_RET_EXCEPTION(-1, "Internal error");
        }
    }

    // get request number and notify other processes of a new request (interrupt running blocking calls)
    uint64_t request_number;
    SynchronizedIncrement* shm_request_id_sync = shm_request_id_.getData();
    if (shm_request_id_sync)
    {
        request_number = shm_request_id_sync->increment();
    }
    else
    {
        LOG_ERR("shm_request_id_sync not initialized");
        CBUN_PCALL_RET_EXCEPTION(-1, "Internal error");
    }

    // request action
    SynchronizedData<GripkitAction>* shm_action_sync = shm_action_.getData();
    if (shm_action_sync)
    {
        shm_action_sync->set(action);
    }
    else
    {
        LOG_ERR("shm_action_sync not initialized");
        CBUN_PCALL_RET_EXCEPTION(-1, "Internal error");
    }
    
    // wait for finish if blocking
    if (blocking)
    {
        int error_count = 0;
        while (true)
        {
            // stop blocking call if a new request came from another process
            if (request_number != shm_request_id_sync->get())
                CBUN_PCALL_RET_OK;

            // keep checking status until the move is finished
            SynchronizedData<GripkitCrEasyStatus>* shm_status_sync = shm_status_.getData();
            if (!shm_status_sync)
            {
                LOG_ERR("shm_status_sync not initialized.");
                CBUN_PCALL_RET_EXCEPTION(-1, "Bad status");
            }
            else
            {
                GripkitCrEasyStatus status = shm_status_sync->get();
                if (action == GripkitAction::GRIP && (status == GripkitCrEasyStatus::HOLDING || status == GripkitCrEasyStatus::NO_PART))
                {
                    CBUN_PCALL_RET_OK;
                }
                else if (action == GripkitAction::RELEASE && status == GripkitCrEasyStatus::RELEASED)
                {
                    CBUN_PCALL_RET_OK;
                }
                else if (status == GripkitCrEasyStatus::IDLE_OR_ERROR || status == GripkitCrEasyStatus::STATUS_ERROR)
                {
                    // return exception only if the error status repeats several times
                    // gripper sometimes returns error for a few short moments when switching from RELEASED to NO_PART
                    ++error_count;
                    LOG_INFO("Status error on grip, count: " << error_count);
                    if (error_count >= MIN_CONTINUOUS_ERROR_COUNT)
                    {
                        LOG_ERR("Status error on grip, repeated too many times")
                        CBUN_PCALL_RET_EXCEPTION(-1, "Bad status");
                    }
                }
                else
                {
                    error_count = 0;
                }
            }
            usleep(US_SLEEP_GRIP_RELEASE);
        }
    }

    CBUN_PCALL_RET_OK;
}

CBUN_PCALL GripkitCrEasy::grip(bool blocking, boost::optional<kr2_program_api::Load> payload)
{
    return performActionCommon(GripkitAction::GRIP, blocking, payload);
}

CBUN_PCALL GripkitCrEasy::release(bool blocking)
{
    return performActionCommon(GripkitAction::RELEASE, blocking, NO_LOAD);
}

GripkitCrEasyStatus GripkitCrEasy::getStatus()
{
    int gripped = -1;
    int no_error = -1;

    // prepare values to be read
    api_->rc_api_->spin();

    // read values
    int gpio_float_count = api_->rc_api_->iob_data_->read_N_GPIOFloat();
    for (int i = 0; i < gpio_float_count; ++i)
    {
        const kr2rc_api::IOData::GPIOFloat* data = api_->rc_api_->iob_data_->read_GPIOFloat(i);
        
        if (data)
        {
            if (data->gpio_id_ == gpio_setup_.duid_in_gripped_)
            {
                gripped = (data->value_ > INPUT_HIGH_VOLTAGE) ? 1 : 0;
            }

            if (data->gpio_id_ == gpio_setup_.duid_in_no_error_)
            {
                no_error = (data->value_ > INPUT_HIGH_VOLTAGE) ? 1 : 0;
            }
        }
    }

    // if values not found, return error
    if (gripped == -1 || no_error == -1)
    {
        LOG_ERR("Invalid status, gripped: " << gripped << " no_error: " << no_error)
        return GripkitCrEasyStatus::STATUS_ERROR;
    }

    // return status
    if (gripped == 1)
    {
        if (no_error == 1)
            return GripkitCrEasyStatus::HOLDING;
        else
            return GripkitCrEasyStatus::NO_PART;
    }
    else
    {
        if (no_error == 1)
            return GripkitCrEasyStatus::RELEASED;
        else
            return GripkitCrEasyStatus::IDLE_OR_ERROR;
    }
}

GripkitCrEasyStatus GripkitCrEasy::getStatusSharedMemory()
{
    SynchronizedData<GripkitCrEasyStatus>* shm_status_sync = shm_status_.getData();
    if (!shm_status_sync)
    {
        LOG_ERR("shm_status_sync not initialized.");
        throw GripkitException("Internal error.");
    }
    else
    {
        GripkitCrEasyStatus status = shm_status_sync->get();
        if (status == GripkitCrEasyStatus::STATUS_ERROR)
        {
            LOG_ERR("Status could not be read.")
            throw GripkitException("Internal error.");
        }
        return status;
    }
}

kr2_program_api::Number GripkitCrEasy::isCommon(GripkitCrEasyStatus checkedStatus)
{
    return (getStatusSharedMemory() == checkedStatus) ? 1L : 0L;
}

kr2_program_api::Number GripkitCrEasy::isReleased()
{
    return isCommon(GripkitCrEasyStatus::RELEASED);
}

kr2_program_api::Number GripkitCrEasy::isHolding()
{
    return isCommon(GripkitCrEasyStatus::HOLDING);
}

kr2_program_api::Number GripkitCrEasy::isNoPart()
{
    return isCommon(GripkitCrEasyStatus::NO_PART);
}

kr2_program_api::Number GripkitCrEasy::isError()
{
    return isCommon(GripkitCrEasyStatus::IDLE_OR_ERROR);
}

bool GripkitCrEasy::processActivationParams(const boost::property_tree::ptree &tree)
{
    kr2_bundle_api::ArgProviderXml arg_provider(tree);

    const int EXPECTED_PARAMS = 1;
    if (arg_provider.getArgCount() != EXPECTED_PARAMS) {
        LOG_ERR("Unexpected param count: actual=" << arg_provider.getArgCount() << ", expected=" << EXPECTED_PARAMS);
        return false;
    }
    
    int robot_generation = arg_provider.getInt(0);
    return setupGPIO(robot_generation);
}

bool GripkitCrEasy::setupGPIO(int robot_generation)
{
    switch (robot_generation)
    {
        case 1:
            gpio_setup_.duid_out_power_ = 3178770;
            gpio_setup_.duid_out_activation_ = 3178755;
            gpio_setup_.duid_out_grip_ = 3178756;

            gpio_setup_.duid_in_gripped_ = 3152404;
            gpio_setup_.duid_in_no_error_ = 3152403;

            gpio_setup_.config_enabled_ = 134283288;
            gpio_setup_.config_disabled_ = 134283276;

            return true;

        case 2:
            gpio_setup_.duid_out_power_ = 3178770;
            gpio_setup_.duid_out_activation_ = 3178772;
            gpio_setup_.duid_out_grip_ = 3178771;

            gpio_setup_.duid_in_gripped_ = 3152406;
            gpio_setup_.duid_in_no_error_ = 3152407;

            gpio_setup_.config_enabled_ = 134283288;
            gpio_setup_.config_disabled_ = 134283264;

            return true;

        default:
            return false;
    }
}