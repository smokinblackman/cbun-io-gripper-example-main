package com.kassowrobots.weissroboticsgripkit

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.Icon
import androidx.compose.material.Surface
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.kassowrobots.api.app.fragment.KRFragment
import com.kassowrobots.weissroboticsgripkit.extraelements.StatusMessage
import com.kassowrobots.weissroboticsgripkit.gripperanimation.GripperVisualization
import com.kassowrobots.weissroboticsgripkit.krelements.KRButton
import com.kassowrobots.weissroboticsgripkit.krelements.KRContent
import com.kassowrobots.weissroboticsgripkit.krelements.KRHeading2
import com.kassowrobots.weissroboticsgripkit.krelements.KRSwitch

class DashboardFragment : KRFragment() {
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setViewCompositionStrategy(
                ViewCompositionStrategy.DisposeOnLifecycleDestroyed(viewLifecycleOwner)
            )
            setContent {
                val viewModel = viewModel<DashboardViewModel>(factory =
                    DashboardViewModel.DashboardViewModelFactory(requireArguments().getString("device_name")!!)
                )
                FragmentUI(viewModel.gripperData, viewModel::onUserInput)
            }
        }
    }

    @Composable @Preview
    fun FragmentUIPreview() {
        FragmentUI(
            gripperData = GripperData(
                communicationOk = false,
                activated = false,
                mounted = false,
                status = GripperStatus.Gripped,
                gripDirection = GripperDirection.GripIn
            ),
            onUserInput = { })
    }

    @Composable
    fun FragmentUI(gripperData: GripperData, onUserInput: (UserInputEvent) -> Unit) {
        Surface(color = Color(0xFFE2E6F0)) {
            Column(modifier = Modifier
                .padding(20.dp)
                .fillMaxHeight()) {

                Row {
                    KRButton(
                        label = "RELEASE",
                        onClickCallback = { onUserInput(UserInputEvent.ReleaseClicked) },
                        modifier = Modifier.weight(1f),
                        enabled = gripperData.status != GripperStatus.Error
                    )
                    Spacer(modifier = Modifier.width(10.dp))
                    KRButton(
                            label = "GRIP",
                    onClickCallback = { onUserInput(UserInputEvent.GripClicked) },
                    modifier = Modifier.weight(1f),
                    enabled = gripperData.status != GripperStatus.Error
                    )
                }

                Spacer(modifier = Modifier.height(20.dp))

                Row(verticalAlignment = Alignment.CenterVertically) {
                    KRHeading2(text = "GRIP DIRECTION VISUALIZATION")

                    Spacer(Modifier.width(10.dp))

                    KRContent(text = "IN")
                    KRSwitch(gripperData.gripDirection == GripperDirection.GripOut, {
                        onUserInput(
                            UserInputEvent.GripDirectionChange(
                                if (it) GripperDirection.GripOut
                                else GripperDirection.GripIn
                            )
                        )
                    })
                    KRContent(text = "OUT")
                }

                Spacer(modifier = Modifier.height(20.dp))

                Box (modifier = Modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
                    KRHeading2(gripperData.status.toString())
                }

                Spacer(modifier = Modifier.height(20.dp))

                Column (
                    modifier = Modifier.fillMaxWidth(),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    GripperVisualization(
                        status = gripperData.status,
                        gripIn = gripperData.gripDirection == GripperDirection.GripIn,
                        width = 200.dp
                    )
                }

                Spacer(modifier = Modifier.height(20.dp))

                val errorColor = Color(220, 0, 0)
                val warningColor = Color(255, 152, 0, 255)

                if (!gripperData.communicationOk) {
                    StatusMessage(
                        imageVector = Icons.Filled.Error,
                        contentDescription = "error",
                        text = "Backend not responding",
                        color = errorColor
                    )
                }

                if (!gripperData.activated) {
                    StatusMessage(
                        imageVector = Icons.Filled.Warning,
                        contentDescription = "error",
                        text = "Not activated",
                        color = warningColor
                    )
                } else if (gripperData.status == GripperStatus.Error && gripperData.communicationOk) {
                    StatusMessage(
                        imageVector = Icons.Filled.Warning,
                        contentDescription = "error",
                        text = "Activation in progress, check parameter \"generation\"",
                        color = warningColor
                    )
                }

                if (!gripperData.mounted) {
                    StatusMessage(
                        imageVector = Icons.Filled.Warning,
                        contentDescription = "error",
                        text = "Not mounted",
                        color = warningColor
                    )
                }
            }
        }
    }
}