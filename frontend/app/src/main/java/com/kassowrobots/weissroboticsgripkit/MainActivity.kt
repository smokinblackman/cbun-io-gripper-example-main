package com.kassowrobots.weissroboticsgripkit

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setFullscreen()
        setContentView(R.layout.activity_main)

        val args = Bundle()
        args.putString("device_name", "test_name")
        val fragment = DashboardFragment()
        fragment.arguments = args

        supportFragmentManager
            .beginTransaction()
            .add(R.id.container, fragment)
            .commit()
    }

    private fun setFullscreen() {
        window?.run {
            WindowCompat.setDecorFitsSystemWindows(this, false)
        }
        WindowInsetsControllerCompat(window, window.decorView).apply {
            hide(WindowInsetsCompat.Type.navigationBars())
        }
    }


}