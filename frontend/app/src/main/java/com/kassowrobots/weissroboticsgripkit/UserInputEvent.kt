package com.kassowrobots.weissroboticsgripkit

sealed class UserInputEvent {
    object GripClicked : UserInputEvent()
    object ReleaseClicked : UserInputEvent()
    data class GripDirectionChange(val gripperDirection: GripperDirection) : UserInputEvent()
}