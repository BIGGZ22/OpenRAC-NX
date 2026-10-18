// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.app.ServiceCompat
import androidx.core.content.ContextCompat

/**
 * Keeps the launcher's process running while it sets a game up (a disc's copy, the extractor, the
 * levels), so that the player can leave the launcher, and shows how far it is in a notification.
 * The launcher's UI says what to show (OpenracPlugin.backgroundStatus); the work itself stays where
 * it runs, in the Rust side and the processes it starts.
 */
class SetupService : Service() {
  companion object {
    private const val CHANNEL = "setup"
    private const val ID = 1
    private const val TAG = "OpenRAC"
    @Volatile private var running = false

    /** Shows the status, starting the service the first time (only possible while the launcher is
     *  in front, which it is when a set-up starts). */
    fun show(context: Context, title: String, text: String, percent: Int?) {
      val notification = notification(context, title, text, percent)
      if (running) {
        try {
          NotificationManagerCompat.from(context).notify(ID, notification)
        } catch (e: SecurityException) {
          // Notifications declined: the service still runs, silently.
        }
        return
      }
      val intent = Intent(context, SetupService::class.java)
        .putExtra("title", title).putExtra("text", text).putExtra("percent", percent ?: -1)
      try {
        ContextCompat.startForegroundService(context, intent)
        running = true
      } catch (e: Exception) {
        Log.w(TAG, "cannot keep the set-up running in the background: ${e.message}")
      }
    }

    fun hide(context: Context) {
      if (running) {
        running = false
        context.stopService(Intent(context, SetupService::class.java))
      }
    }

    private fun notification(context: Context, title: String, text: String, percent: Int?) =
      NotificationCompat.Builder(context, channel(context))
        .setSmallIcon(R.drawable.ic_notification)
        .setContentTitle(title)
        .setContentText(text)
        .setProgress(100, percent ?: 0, percent == null)
        .setOngoing(true)
        .setOnlyAlertOnce(true)
        .setSilent(true)
        .setCategory(NotificationCompat.CATEGORY_PROGRESS)
        .setForegroundServiceBehavior(NotificationCompat.FOREGROUND_SERVICE_IMMEDIATE)
        .setContentIntent(
          PendingIntent.getActivity(
            context, 0,
            Intent(context, MainActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT,
          ),
        )
        .build()

    private fun channel(context: Context): String {
      val manager = context.getSystemService(NotificationManager::class.java)
      if (manager.getNotificationChannel(CHANNEL) == null) {
        manager.createNotificationChannel(
          NotificationChannel(CHANNEL, context.getString(R.string.setup_channel), NotificationManager.IMPORTANCE_LOW),
        )
      }
      return CHANNEL
    }
  }

  override fun onBind(intent: Intent?): IBinder? = null

  override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
    val percent = intent?.getIntExtra("percent", -1)?.takeIf { it >= 0 }
    val notification = notification(
      this, intent?.getStringExtra("title") ?: getString(R.string.app_name), intent?.getStringExtra("text") ?: "", percent,
    )
    try {
      ServiceCompat.startForeground(this, ID, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC)
    } catch (e: Exception) {
      Log.w(TAG, "cannot keep the set-up running in the background: ${e.message}")
      running = false
      stopSelf()
    }
    // Not restarted if Android stops the process: the set-up it carried is gone with it.
    return START_NOT_STICKY
  }

  override fun onTimeout(startId: Int, fgsType: Int) {
    // Android 15's limit on data-sync services: stop as it asks, the set-up runs on in front.
    running = false
    stopSelf()
  }

  override fun onDestroy() {
    running = false
    super.onDestroy()
  }
}
