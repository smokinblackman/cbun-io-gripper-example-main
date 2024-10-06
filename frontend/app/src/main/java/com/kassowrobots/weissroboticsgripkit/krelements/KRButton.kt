package com.kassowrobots.weissroboticsgripkit.krelements

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.Button
import androidx.compose.material.ButtonDefaults
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable @Preview
fun KRButtonPreview() {
    KRButton(label = "Click me!", onClickCallback = { })
}

@Composable
fun KRButton(
    label: String,
    onClickCallback: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true
) {
    Button(
        onClick = onClickCallback,
        enabled = enabled,
        shape = RoundedCornerShape(5.dp),
        colors = ButtonDefaults.buttonColors(
            backgroundColor = Color.White,
            disabledBackgroundColor = Color.White
        ),
        modifier = modifier
            .height(50.dp)
            .background(
                color = Color.White,
                shape = RoundedCornerShape(5.dp)
            )
    ) {
        Text(
            text = label,
            fontSize = 14.sp,
            fontWeight = FontWeight.Bold,
            color =  if (enabled) Color(0xFF616265) else Color(0xFFC9CBCD),
        )
    }
}