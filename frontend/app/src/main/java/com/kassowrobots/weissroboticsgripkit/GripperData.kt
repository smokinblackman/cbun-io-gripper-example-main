package com.kassowrobots.weissroboticsgripkit

enum class GripperStatus {
    Gripped,
    Released,
    Error,
    NoPart
}

enum class GripperDirection {
    GripIn,
    GripOut
}

data class GripperData(
    var communicationOk: Boolean,
    var activated: Boolean,
    var mounted: Boolean,
    var status: GripperStatus,
    var gripDirection: GripperDirection
)