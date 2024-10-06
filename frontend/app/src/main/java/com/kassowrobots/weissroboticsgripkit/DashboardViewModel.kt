package com.kassowrobots.weissroboticsgripkit

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.kassowrobots.api.robot.xmlrpc.Fault
import com.kassowrobots.api.robot.xmlrpc.Params
import com.kassowrobots.api.robot.xmlrpc.XmlRpcClient
import com.kassowrobots.api.robot.xmlrpc.values.Value
import com.kassowrobots.api.robot.xmlrpc.values.ValueBool
import com.kassowrobots.api.robot.xmlrpc.values.ValueInt
import com.kassowrobots.api.robot.xmlrpc.values.ValueStruct
import com.kassowrobots.api.util.KRLog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class XmlRpcWrapper(private val xmlClient: XmlRpcClient) {
    @Synchronized
    @Throws(Fault::class)
    fun xmlRpc(aMethodName: String, aParams: Params): Value {
        return xmlClient.execute(aMethodName, aParams, 2.0)
    }
}

data class GripperXmlData(
    val valid: Boolean,
    val status: GripperStatus,
    val activated: Boolean,
    val mounted: Boolean
)

object StatusXmlRpcReaderUseCase {
    private fun parseXmlRpcStatus(statusInt: Int): GripperStatus {
        return when (statusInt) {
            0 -> GripperStatus.Error
            1 -> GripperStatus.Gripped
            2 -> GripperStatus.NoPart
            3 -> GripperStatus.Released
            else -> GripperStatus.Error
        }
    }

    private suspend fun getGripperDataFromRpc(xmlClient: XmlRpcWrapper): ValueStruct? {
        return withContext(Dispatchers.IO) {
            try {
                xmlClient.xmlRpc("getStatus", Params()) as ValueStruct
            } catch (e: Exception) {
                null
            }
        }
    }

    private fun parseGripperDataFromRpc(gripperDataXml: ValueStruct?): GripperXmlData {
        return if (gripperDataXml == null || gripperDataXml.getValueInt("success").int == 0)
                GripperXmlData(
                    valid = false,
                    status = GripperStatus.Error,
                    activated = true,
                    mounted = true
                )
            else
                GripperXmlData(
                    valid = true,
                    status = parseXmlRpcStatus(gripperDataXml.getValueInt("status").int),
                    activated = gripperDataXml.getValueInt("activated").int == 1,
                    mounted = gripperDataXml.getValueInt("mounted").int == 1
                )
    }

    operator fun invoke(xmlClient: XmlRpcWrapper, delayMs: Long) : Flow<GripperXmlData> {
        return flow {
            var lastGripperStatusData = parseGripperDataFromRpc(getGripperDataFromRpc(xmlClient))
            emit(lastGripperStatusData)

            while (true) {
                val gripperStatusData = parseGripperDataFromRpc(getGripperDataFromRpc(xmlClient))

                if (lastGripperStatusData != gripperStatusData) {
                    lastGripperStatusData = gripperStatusData
                    emit(gripperStatusData)
                }

                delay(timeMillis = delayMs)
            }
        }
    }
}

class DashboardViewModel(private val xmlClient: XmlRpcWrapper): ViewModel() {

    var gripperData by mutableStateOf(GripperData(
        communicationOk = false,
        activated = true,
        mounted = true,
        status = GripperStatus.Error,
        gripDirection = GripperDirection.GripIn
    ))
        private set

    init {
        viewModelScope.launch {
            StatusXmlRpcReaderUseCase(xmlClient = xmlClient, delayMs = 100).collect {
                    value ->
                    gripperData = gripperData.copy(
                        communicationOk = value.valid,
                        activated = value.activated,
                        mounted = value.mounted,
                        status = value.status
                    )
            }
        }
    }

    fun onUserInput(event: UserInputEvent) {
        when (event) {
            is UserInputEvent.GripClicked -> requestSetGripper(true)
            is UserInputEvent.ReleaseClicked -> requestSetGripper(false)
            is UserInputEvent.GripDirectionChange -> gripperData = gripperData.copy(gripDirection = event.gripperDirection)
        }
    }

    private fun requestSetGripper(grip: Boolean) {
        try {
            // new instance for parameters sent
            val params = Params()

            // adds gripper width as double
            params.add(ValueBool(grip))

            //executes the method using xmlRpc service
            xmlClient.xmlRpc("setGripper", params)

        } catch (e: Fault) {
            KRLog.e("Robot", "Failed to set configuration: " + e.description)
        }
    }

    @Suppress("UNCHECKED_CAST")
    class DashboardViewModelFactory(private val deviceName: String) : ViewModelProvider.NewInstanceFactory() {
        override fun <T : ViewModel> create(modelClass: Class<T>): T {
            val xmlClient = XmlRpcClient(deviceName)
            return DashboardViewModel(XmlRpcWrapper(xmlClient)) as T
        }
    }

}