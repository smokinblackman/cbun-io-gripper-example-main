package com.kassowrobots.weissroboticsgripkit.krelements

import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.sp

@Composable @Preview
fun ContentPreview() {
    KRContent(text = "Hello world!")
}

@Composable
fun KRContent(text: String, modifier: Modifier = Modifier) {
    Text(
        text=text,
        fontSize = 13.sp,
        color = Color(0xFF616265),
        modifier = modifier
    )
}