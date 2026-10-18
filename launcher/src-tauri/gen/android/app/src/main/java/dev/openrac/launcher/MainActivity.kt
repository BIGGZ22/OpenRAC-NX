package dev.openrac.launcher

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.webkit.WebView
import androidx.activity.OnBackPressedCallback
import androidx.activity.enableEdgeToEdge
import androidx.core.view.ViewCompat
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat

/**
 * The launcher's window: full screen (the system's bars hidden, a swipe shows them for a moment),
 * and driven by a controller as well as by touch. A controller's buttons and sticks are handed to
 * the page (src/lib/pad.ts, `window.__openracPad`), which moves the focus and presses what is
 * focused; Back asks the page first (`window.__openracBack`), then leaves the launcher.
 */
class MainActivity : TauriActivity() {
  companion object {
    private const val NOTIFICATIONS = 0x4f52

    /** The launcher's window is in front: Android lets it start an activity (UpdateReceiver). */
    @Volatile var inFront = false
      private set
  }

  private var webView: WebView? = null

  // The launcher's own Back, not Tauri's (which goes back in the page's history).
  override val handleBackNavigation: Boolean = false

  override fun onCreate(savedInstanceState: Bundle?) {
    enableEdgeToEdge()
    // The environment the Rust side reads, before it starts; OpenRAC's tools are unpacked beside
    // it when they have to be (the page waits for them: toolsReady).
    OpenracSetup.prepare(this)
    super.onCreate(savedInstanceState)
    hideSystemBars()
    askForNotifications()
    // The page stays clear of a camera's cut-out, and of the keyboard while it is up. Not of the
    // system's bars: they are hidden, and the moment a swipe shows them they lie over the page
    // (making room for them moved the whole page down and back up, a flash at every swipe).
    ViewCompat.setOnApplyWindowInsetsListener(findViewById<View>(android.R.id.content)) { view, insets ->
      val clear = insets.getInsets(WindowInsetsCompat.Type.displayCutout() or WindowInsetsCompat.Type.ime())
      view.setPadding(clear.left, clear.top, clear.right, clear.bottom)
      WindowInsetsCompat.CONSUMED
    }
    onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
      override fun handleOnBackPressed() {
        val page = webView ?: return leave()
        page.evaluateJavascript("window.__openracBack ? window.__openracBack() : false") { handled ->
          if (handled != "true") leave()
        }
      }
    })
  }

  /** Android 13 and later show an app's notifications only once the player allows them: asked
   *  once, when the launcher first opens, so that a set-up's progress (SetupService.kt) shows. */
  private fun askForNotifications() {
    if (Build.VERSION.SDK_INT < 33) return
    if (checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) return
    val settings = getSharedPreferences("launcher", MODE_PRIVATE)
    if (settings.getBoolean("askedForNotifications", false)) return
    settings.edit().putBoolean("askedForNotifications", true).apply()
    requestPermissions(arrayOf(Manifest.permission.POST_NOTIFICATIONS), NOTIFICATIONS)
  }

  @Deprecated("Deprecated in Java")
  override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    if (requestCode == NOTIFICATIONS && grantResults.firstOrNull() == PackageManager.PERMISSION_GRANTED) {
      SetupService.refresh(this)
    }
  }

  override fun onResume() {
    super.onResume()
    inFront = true
    UpdateReceiver.showWaiting(this)
  }

  override fun onPause() {
    inFront = false
    super.onPause()
  }

  override fun onWebViewCreate(webView: WebView) {
    this.webView = webView
  }

  override fun onWindowFocusChanged(hasFocus: Boolean) {
    super.onWindowFocusChanged(hasFocus)
    if (hasFocus) {
      hideSystemBars()
    } else {
      // What is held now is let go elsewhere (the game, a picker, a system dialog): the page and
      // the stick's directions start again from nothing held.
      heldX = null
      heldY = null
      webView?.evaluateJavascript("window.__openracPadRelease && window.__openracPadRelease()", null)
    }
  }

  private fun hideSystemBars() {
    WindowCompat.getInsetsController(window, window.decorView).apply {
      systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
      hide(WindowInsetsCompat.Type.systemBars())
    }
  }

  /** Back on the library with nothing open: the launcher goes to the background, as Home does,
   *  so that a set-up in progress goes on. */
  private fun leave() {
    moveTaskToBack(true)
  }

  // ---- a controller ----

  private fun send(event: String) {
    webView?.evaluateJavascript("window.__openracPad && window.__openracPad('$event')", null)
  }

  private fun button(keyCode: Int): String? = when (keyCode) {
    KeyEvent.KEYCODE_DPAD_UP -> "up"
    KeyEvent.KEYCODE_DPAD_DOWN -> "down"
    KeyEvent.KEYCODE_DPAD_LEFT -> "left"
    KeyEvent.KEYCODE_DPAD_RIGHT -> "right"
    KeyEvent.KEYCODE_BUTTON_A, KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_BUTTON_START -> "accept"
    KeyEvent.KEYCODE_BUTTON_B -> "back"
    KeyEvent.KEYCODE_BUTTON_L1 -> "prev"
    KeyEvent.KEYCODE_BUTTON_R1 -> "next"
    else -> null
  }

  private fun fromController(event: KeyEvent): Boolean {
    val source = event.source
    return source and InputDevice.SOURCE_GAMEPAD == InputDevice.SOURCE_GAMEPAD ||
      source and InputDevice.SOURCE_JOYSTICK == InputDevice.SOURCE_JOYSTICK ||
      source and InputDevice.SOURCE_DPAD == InputDevice.SOURCE_DPAD
  }

  override fun dispatchKeyEvent(event: KeyEvent): Boolean {
    val name = if (fromController(event)) button(event.keyCode) else null
    if (name == null || webView == null) return super.dispatchKeyEvent(event)
    // Held buttons repeat in the page, at its own pace.
    when (event.action) {
      KeyEvent.ACTION_DOWN -> if (event.repeatCount == 0) send("down:$name")
      KeyEvent.ACTION_UP -> send("up:$name")
    }
    return true
  }

  // The direction the hat or the left stick points in, as the d-pad's buttons.
  private var heldX: String? = null
  private var heldY: String? = null

  private fun direction(value: Float, negative: String, positive: String): String? = when {
    value <= -0.5f -> negative
    value >= 0.5f -> positive
    else -> null
  }

  override fun dispatchGenericMotionEvent(event: MotionEvent): Boolean {
    val joystick = event.source and InputDevice.SOURCE_JOYSTICK == InputDevice.SOURCE_JOYSTICK
    if (!joystick || event.actionMasked != MotionEvent.ACTION_MOVE || webView == null) {
      return super.dispatchGenericMotionEvent(event)
    }
    val hatX = event.getAxisValue(MotionEvent.AXIS_HAT_X)
    val hatY = event.getAxisValue(MotionEvent.AXIS_HAT_Y)
    val x = direction(if (hatX != 0f) hatX else event.getAxisValue(MotionEvent.AXIS_X), "left", "right")
    val y = direction(if (hatY != 0f) hatY else event.getAxisValue(MotionEvent.AXIS_Y), "up", "down")
    if (x != heldX) {
      heldX?.let { send("up:$it") }
      x?.let { send("down:$it") }
      heldX = x
    }
    if (y != heldY) {
      heldY?.let { send("up:$it") }
      y?.let { send("down:$it") }
      heldY = y
    }
    return true
  }
}
