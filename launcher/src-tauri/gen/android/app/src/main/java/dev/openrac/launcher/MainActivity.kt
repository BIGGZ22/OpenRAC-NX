package dev.openrac.launcher

import android.os.Bundle
import androidx.activity.enableEdgeToEdge

class MainActivity : TauriActivity() {
  override fun onCreate(savedInstanceState: Bundle?) {
    enableEdgeToEdge()
    // OpenRAC's tools and the environment the Rust side reads, before it starts.
    OpenracSetup.prepare(this)
    super.onCreate(savedInstanceState)
  }
}
