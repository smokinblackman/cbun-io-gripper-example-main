package com.kassowrobots.weissroboticsgripkit.krelements

import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable @Preview
fun KRHeading2Preview() {
    KRHeading2(text = "Hello world!")
}

@Composable
fun KRHeading2(text: String, modifier: Modifier = Modifier, color: Color = Color(0xFF393B3F)) {
    Text(
        text=text.uppercase(),
        fontSize = 14.sp,
        fontWeight = FontWeight.Bold,
        color = color,
        modifier = modifier
    )
    Spacer(modifier = Modifier.height(12.dp)) // applicable if used within the Column
}