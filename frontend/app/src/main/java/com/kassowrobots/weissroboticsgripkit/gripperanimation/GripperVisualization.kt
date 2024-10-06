package com.kassowrobots.weissroboticsgripkit.gripperanimation

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.kassowrobots.weissroboticsgripkit.GripperStatus
import kotlin.math.abs

@Composable @Preview
fun GripperVisualizationPreview() {
    GripperVisualization(status = GripperStatus.Gripped, gripIn = false)
}

@Composable
fun GripperVisualization(status: GripperStatus, gripIn: Boolean, modifier: Modifier = Modifier, width: Dp = 400.dp) {
    val gripperValue = when (status) {
        GripperStatus.Gripped -> 0.5f
        GripperStatus.Released -> if (gripIn) 0f else 1f
        GripperStatus.Error -> 0f
        GripperStatus.NoPart -> if (gripIn) 1f else 0f
    }
    val fingerPosition: Float by animateFloatAsState(
        gripperValue,
        animationSpec = tween(durationMillis = 300, easing = LinearEasing),
        label = "fingerPosition"
    )

    val releasedNoPartColor = Color(red = 0, green = 157, blue = 255, alpha = 255)
    val activationColor: Color by animateColorAsState(
        when (status) {
            GripperStatus.Gripped -> Color.Green
            GripperStatus.Released -> releasedNoPartColor
            GripperStatus.Error -> Color.LightGray
            GripperStatus.NoPart -> releasedNoPartColor
        },
        label = "activationColor"
    )

    val height: Dp = (width.value * (1 - 1/10f + 1/2.5f)).dp  // accounts for the fingers
    Canvas(modifier = modifier.width(width).height(height)) {
        val cornerRadius = CornerRadius(size.width / 15, size.width / 15)

        val fingerColor = Color.Gray
        val fingerSize = Size(size.width / 3.5f, size.width / 2.5f)
        val fingerTop = size.width - size.width / 10

        val fingerOffset = (size.width - fingerSize.width * 2) / 2 * fingerPosition
        val fingerLeftTopLeft = Offset(0f, fingerTop) + Offset(fingerOffset, 0f)
        val fingerRightTopLeft = Offset(size.width - fingerSize.width, fingerTop) - Offset(fingerOffset, 0f)

        drawRoundRect(
            color = fingerColor,
            topLeft = fingerLeftTopLeft,
            size = fingerSize,
            cornerRadius = cornerRadius
        )
        drawRoundRect(
            color = fingerColor,
            topLeft = fingerRightTopLeft,
            size = fingerSize,
            cornerRadius = cornerRadius
        )



        val gripperTopLeft = Offset(0f, 0f)
        val gripperSize = Size(size.width, size.width)
        val gripperColor = Color.DarkGray

        drawRoundRect(
            color = gripperColor,
            topLeft = gripperTopLeft,
            size = gripperSize,
            cornerRadius = cornerRadius
        )



        val activationCenter = Offset(size.width / 2, size.width / 3.5f)
        val activationWidth = size.width / 22
        val activationStrokeWidth = size.width / 11
        val activationStart = activationCenter + Offset(-activationWidth, 0f)
        val activationEnd = activationCenter + Offset(activationWidth, 0f)

        drawLine(
            color = activationColor,
            start = activationStart,
            end = activationEnd,
            strokeWidth = activationStrokeWidth,
            cap = StrokeCap.Round
        )



        val arrowStroke = size.width / 60
        val arrowSize = size.width / 15
        val arrowColor = Color.Red
        val arrowObjectLineHalfLength = size.width / 10

        val arrowCenterY = gripperSize.height + (fingerTop + fingerSize.height - gripperSize.height) / 2
        val arrowCenter = Offset(size.width / 2, arrowCenterY)
        val arrowHalfLength = size.width / 2 - fingerOffset - fingerSize.width

        if (status == GripperStatus.Gripped && abs(fingerPosition - 0.5f) < 0.001) {
            if (gripIn) {
                val arrowOffset = Offset(arrowHalfLength - arrowStroke, 0f)
                val arrowLeft = arrowCenter - arrowOffset
                val arrowRight = arrowCenter + arrowOffset

                drawLine(
                    color = arrowColor,
                    start = arrowLeft + Offset(arrowSize, 0f),
                    end = arrowRight - Offset(arrowSize, 0f),
                    strokeWidth = arrowStroke
                )

                val arrowObjectXOffset = arrowHalfLength - arrowStroke / 2
                drawLine(
                    color = arrowColor,
                    start = arrowCenter - Offset(arrowObjectXOffset, arrowObjectLineHalfLength),
                    end = arrowCenter - Offset(arrowObjectXOffset, -arrowObjectLineHalfLength),
                    strokeWidth = arrowStroke
                )
                drawLine(
                    color = arrowColor,
                    start = arrowCenter + Offset(arrowObjectXOffset, arrowObjectLineHalfLength),
                    end = arrowCenter + Offset(arrowObjectXOffset, -arrowObjectLineHalfLength),
                    strokeWidth = arrowStroke
                )

                val arrowPath = Path().apply {
                    moveTo(arrowLeft.x, arrowLeft.y)
                    lineTo(arrowLeft.x + arrowSize, arrowLeft.y - arrowSize * 0.7f)
                    lineTo(arrowLeft.x + arrowSize, arrowLeft.y + arrowSize * 0.7f)
                    close()

                    moveTo(arrowRight.x, arrowRight.y)
                    lineTo(arrowRight.x - arrowSize, arrowRight.y - arrowSize * 0.7f)
                    lineTo(arrowRight.x - arrowSize, arrowRight.y + arrowSize * 0.7f)
                    close()
                }

                drawPath(
                    path = arrowPath,
                    color = arrowColor
                )
            } else {

                val outArrowLength = size.width / 7

                val outArrowObjectXOffset = arrowHalfLength + fingerSize.width + arrowStroke / 2
                drawLine(
                    color = arrowColor,
                    start = arrowCenter - Offset(outArrowObjectXOffset, arrowObjectLineHalfLength),
                    end = arrowCenter - Offset(outArrowObjectXOffset, -arrowObjectLineHalfLength),
                    strokeWidth = arrowStroke
                )
                drawLine(
                    color = arrowColor,
                    start = arrowCenter + Offset(outArrowObjectXOffset, arrowObjectLineHalfLength),
                    end = arrowCenter + Offset(outArrowObjectXOffset, -arrowObjectLineHalfLength),
                    strokeWidth = arrowStroke
                )

                val outArrowStartOffset = arrowHalfLength + fingerSize.width + arrowStroke
                val outArrowLeft = arrowCenter - Offset(outArrowStartOffset, 0f)
                val outArrowRight = arrowCenter + Offset(outArrowStartOffset, 0f)

                drawLine(
                    color = arrowColor,
                    start = outArrowLeft - Offset(arrowSize, 0f),
                    end = outArrowLeft - Offset(outArrowLength, 0f),
                    strokeWidth = arrowStroke
                )
                drawLine(
                    color = arrowColor,
                    start = outArrowRight + Offset(arrowSize, 0f),
                    end = outArrowRight + Offset(outArrowLength, 0f),
                    strokeWidth = arrowStroke
                )

                val outArrowPath = Path().apply {
                    moveTo(outArrowLeft.x, outArrowLeft.y)
                    lineTo(outArrowLeft.x - arrowSize, outArrowLeft.y - arrowSize * 0.7f)
                    lineTo(outArrowLeft.x - arrowSize, outArrowLeft.y + arrowSize * 0.7f)
                    close()

                    moveTo(outArrowRight.x, outArrowRight.y)
                    lineTo(outArrowRight.x + arrowSize, outArrowRight.y - arrowSize * 0.7f)
                    lineTo(outArrowRight.x + arrowSize, outArrowRight.y + arrowSize * 0.7f)
                    close()
                }

                drawPath(
                    path = outArrowPath,
                    color = arrowColor
                )
            }
        }
    }
}