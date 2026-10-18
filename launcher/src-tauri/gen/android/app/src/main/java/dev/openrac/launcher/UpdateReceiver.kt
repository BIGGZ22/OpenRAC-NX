// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.pm.PackageInstaller
import android.os.Build
import android.util.Log
import android.widget.Toast

/**
 * What Android's installer says of a launcher update (OpenracPlugin.installApk): it asks the player
 * to confirm, which this shows, or it failed, which this says. On success the launcher is replaced
 * and its process ends.
 */
class UpdateReceiver : BroadcastReceiver() {
  override fun onReceive(context: Context, intent: Intent) {
    val status = intent.getIntExtra(PackageInstaller.EXTRA_STATUS, PackageInstaller.STATUS_FAILURE)
    when (status) {
      PackageInstaller.STATUS_PENDING_USER_ACTION -> {
        val confirm = if (Build.VERSION.SDK_INT >= 33) {
          intent.getParcelableExtra(Intent.EXTRA_INTENT, Intent::class.java)
        } else {
          @Suppress("DEPRECATION") intent.getParcelableExtra(Intent.EXTRA_INTENT)
        }
        confirm?.let { context.startActivity(it.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)) }
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
