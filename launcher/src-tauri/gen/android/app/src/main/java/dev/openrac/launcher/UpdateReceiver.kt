// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.pm.PackageInstaller
import android.os.Build
import android.util.Log
import android.widget.Toast
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat

/**
 * What Android's installer says of a launcher update (OpenracPlugin.installApk): it asks the player
 * to confirm, which this shows, or it failed, which this says. On success the launcher is replaced
 * and its process ends. Android starts no activity for an app in the background: then the
 * confirmation waits in a notification, and for the launcher's return (MainActivity.onResume).
 */
class UpdateReceiver : BroadcastReceiver() {
  companion object {
    private const val CHANNEL = "update"
    private const val ID = 2
    @Volatile private var waiting: Intent? = null

    /** The launcher is in front again: the confirmation it missed shows now. */
    fun showWaiting(context: Context) {
      val confirm = waiting ?: return
      waiting = null
      NotificationManagerCompat.from(context).cancel(ID)
      try {
        context.startActivity(confirm)
      } catch (e: Exception) {
        Log.w("OpenRAC", "cannot show the update's confirmation: ${e.message}")
      }
    }

    private fun notify(context: Context, confirm: Intent) {
      val manager = context.getSystemService(NotificationManager::class.java)
      if (manager.getNotificationChannel(CHANNEL) == null) {
        manager.createNotificationChannel(
          NotificationChannel(CHANNEL, context.getString(R.string.update_channel), NotificationManager.IMPORTANCE_HIGH),
        )
      }
      val notification = NotificationCompat.Builder(context, CHANNEL)
        .setSmallIcon(R.drawable.ic_notification)
        .setContentTitle(context.getString(R.string.update_ready))
        .setContentText(context.getString(R.string.update_ready_text))
        .setAutoCancel(true)
        .setContentIntent(
          PendingIntent.getActivity(context, 1, confirm, PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT),
        )
        .build()
      try {
        NotificationManagerCompat.from(context).notify(ID, notification)
      } catch (e: SecurityException) {
        // Notifications declined: the confirmation shows when the launcher is opened again.
      }
    }
  }

  override fun onReceive(context: Context, intent: Intent) {
    val status = intent.getIntExtra(PackageInstaller.EXTRA_STATUS, PackageInstaller.STATUS_FAILURE)
    when (status) {
      PackageInstaller.STATUS_PENDING_USER_ACTION -> {
        val confirm = if (Build.VERSION.SDK_INT >= 33) {
          intent.getParcelableExtra(Intent.EXTRA_INTENT, Intent::class.java)
        } else {
          @Suppress("DEPRECATION") intent.getParcelableExtra(Intent.EXTRA_INTENT)
        }
        val start = confirm?.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK) ?: return
        if (MainActivity.inFront) {
          context.startActivity(start)
        } else {
          waiting = start
          notify(context, start)
        }
      }
      PackageInstaller.STATUS_SUCCESS -> Unit
      else -> {
        val message = when (status) {
          PackageInstaller.STATUS_FAILURE_ABORTED -> return
          PackageInstaller.STATUS_FAILURE_CONFLICT, PackageInstaller.STATUS_FAILURE_INCOMPATIBLE ->
            "The update is signed with another key than this launcher: uninstall it, then install the new one."
          PackageInstaller.STATUS_FAILURE_STORAGE -> "Not enough free space for the update."
          else -> "The update was not installed: " +
            (intent.getStringExtra(PackageInstaller.EXTRA_STATUS_MESSAGE) ?: "status $status")
        }
        Log.w("OpenRAC", message)
        Toast.makeText(context, message, Toast.LENGTH_LONG).show()
      }
    }
  }
}
