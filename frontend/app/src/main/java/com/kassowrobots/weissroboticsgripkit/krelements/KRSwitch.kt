package com.kassowrobots.weissroboticsgripkit.krelements

import androidx.compose.material.Switch
import androidx.compose.material.SwitchDefaults
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview


@Composable @Preview
fun KRSwitchPreview() {
    KRSwitch(checked = false, onCheckedChangeCallback = {})
}

@Composable
fun KRSwitch(
    checked: Boolean,
    onCheckedChangeCallback: (Boolean) -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true
) {
    Switch(
        checked = checked,
        onCheckedChange = onCheckedChangeCallback,
        enabled = enabled,
        colors = SwitchDefaults.colors(
            checkedThumbColor = Color.White,
            uncheckedThumbColor = Color.White,
            checkedTrackColor = Color(0xFF21C95F),
            disabledCheckedTrackColor = Color(0xFF8FE4B0),
            uncheckedTrackColor = Color(0xFFED2843),
            disabledUncheckedTrackColor = Color(0xFFF291A2),
            checkedTrackAlpha = 1F,
            uncheckedTrackAlpha = 1F
        ),
        modifier = modifier
    )
}