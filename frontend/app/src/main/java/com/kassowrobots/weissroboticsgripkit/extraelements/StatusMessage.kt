package com.kassowrobots.weissroboticsgripkit.extraelements

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.width
import androidx.compose.material.Icon
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import com.kassowrobots.weissroboticsgripkit.krelements.KRHeading2

@Composable
fun StatusMessage(imageVector: ImageVector, contentDescription: String?, text: String, color: Color) {
    Row (verticalAlignment = Alignment.CenterVertically) {
        Icon(imageVector, contentDescription, tint = color)
        Spacer(modifier = Modifier.width(5.dp))
        KRHeading2(text = text, color = color)
    }
}