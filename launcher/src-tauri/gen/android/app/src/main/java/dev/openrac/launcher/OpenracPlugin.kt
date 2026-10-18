// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.StatFs
import android.provider.OpenableColumns
import androidx.activity.result.ActivityResult
import app.tauri.annotation.ActivityCallback
import app.tauri.annotation.Command
import app.tauri.annotation.InvokeArg
import app.tauri.annotation.TauriPlugin
import app.tauri.plugin.Invoke
import app.tauri.plugin.JSArray
import app.tauri.plugin.JSObject
import app.tauri.plugin.Plugin
import dev.openrac.game.GameActivity
import java.io.File
import java.util.zip.ZipInputStream
import org.json.JSONObject

@InvokeArg
class LaunchArgs {
  lateinit var library: String
  var args: Array<String> = arrayOf()
}

@InvokeArg
class DriverArgs {
  var id: String? = null
}

/**
 * What only Android's own APIs can do for the launcher (src-tauri/src/android.rs calls these):
 * pick a file through the system's picker (an app sees no paths outside its own folders), start
 * the game's activity, and keep the player's GPU driver packages.
 *
 * A GPU driver package is the zip Android emulators take (Mesa's Turnip, Qualcomm's own drivers):
 * a meta.json naming the driver library ("libraryName") beside it. The launcher unpacks one into
 * files/gpu_drivers/<id>/; the one chosen is handed to the game, which loads it through
 * libadrenotools on Adreno (port/android/gpu_driver.cpp).
 */
@TauriPlugin
class OpenracPlugin(private val activity: Activity) : Plugin(activity) {
  private val drivers: File get() = File(activity.filesDir, "gpu_drivers")
  private val selectedFile: File get() = File(drivers, "selected")

  // ---- files from the system's picker ----

  /** The player picks their disc image; resolves to {fd, name, size} (an open file the Rust side
   *  copies and closes), or {} when they cancelled. */
  @Command
  fun pickDisc(invoke: Invoke) {
    startActivityForResult(invoke, openDocument(), "discPicked")
  }

  @ActivityCallback
  fun discPicked(invoke: Invoke, result: ActivityResult) {
    val uri = result.data?.data
    if (result.resultCode != Activity.RESULT_OK || uri == null) {
      invoke.resolve(JSObject())
      return
    }
    try {
      val (name, size) = describe(uri)
      // The copy goes into the app's own storage; a few hundred megabytes are left for the
      // levels the launcher writes next (about 1 GB, checked when they are written).
      val free = StatFs(activity.filesDir.path).availableBytes
      if (size > 0 && size + (256L shl 20) > free) {
        invoke.reject("$name needs ${size shr 20} MB and the device has ${free shr 20} MB free")
        return
      }
      val fd = activity.contentResolver.openFileDescriptor(uri, "r")?.detachFd()
        ?: throw IllegalStateException("the picker's file cannot be opened")
      val out = JSObject()
      out.put("fd", fd)
      out.put("name", name)
      out.put("size", size)
      invoke.resolve(out)
    } catch (e: Exception) {
      invoke.reject("cannot read the chosen file: ${e.message}")
    }
  }

  // ---- the game ----

  @Command
  fun launchGame(invoke: Invoke) {
    val args = invoke.parseArgs(LaunchArgs::class.java)
    val environment = OpenracSetup.environment(activity).toMutableMap()
    selectedDriver()?.let { (dir, meta) ->
      environment["OPENRAC_GPU_DRIVER_DIR"] = dir.path
      environment["OPENRAC_GPU_DRIVER_LIBRARY"] = meta.getString("libraryName")
    }
    val intent = Intent(activity, GameActivity::class.java)
      .putExtra(GameActivity.EXTRA_LIBRARY, args.library)
      .putExtra(GameActivity.EXTRA_ARGUMENTS, args.args)
      .putExtra(GameActivity.EXTRA_ENVIRONMENT, environment.map { (k, v) -> "$k=$v" }.toTypedArray())
    activity.startActivity(intent)
    invoke.resolve()
  }

  // ---- GPU drivers ----

  /** The packages unpacked, the one chosen, and what the device is. */
  @Command
  fun gpuDrivers(invoke: Invoke) {
    val list = JSArray()
    drivers.listFiles()?.sortedBy { it.name }?.forEach { dir ->
      meta(dir)?.let { list.put(view(dir.name, it)) }
    }
    val out = JSObject()
    out.put("drivers", list)
    out.put("selected", selectedDriver()?.first?.name)
    out.put("device", device())
    invoke.resolve(out)
  }

  /** The player picks a driver package's zip; it is checked, unpacked and listed. */
  @Command
  fun importDriver(invoke: Invoke) {
    startActivityForResult(invoke, openDocument(), "driverPicked")
  }

  @ActivityCallback
  fun driverPicked(invoke: Invoke, result: ActivityResult) {
    val uri = result.data?.data
    if (result.resultCode != Activity.RESULT_OK || uri == null) {
      invoke.resolve(JSObject())
      return
    }
    // Off the main thread: a package is tens of megabytes.
    Thread {
      try {
        val (id, meta) = unpackDriver(uri)
        val out = JSObject()
        out.put("driver", view(id, meta))
        invoke.resolve(out)
      } catch (e: Exception) {
        invoke.reject(e.message ?: "the package could not be read")
      }
    }.start()
  }

  /** {id}: the package the game loads from now on; no id: the system's driver. */
  @Command
  fun selectDriver(invoke: Invoke) {
    val id = invoke.parseArgs(DriverArgs::class.java).id
    if (id == null) {
      selectedFile.delete()
    } else {
      val dir = driverDir(id) ?: return invoke.reject("no driver package $id")
      drivers.mkdirs()
      selectedFile.writeText(dir.name)
    }
    invoke.resolve()
  }

  @Command
  fun removeDriver(invoke: Invoke) {
    val id = invoke.parseArgs(DriverArgs::class.java).id ?: return invoke.reject("no driver named")
    val dir = driverDir(id) ?: return invoke.reject("no driver package $id")
    if (selectedDriver()?.first == dir) {
      selectedFile.delete()
    }
    dir.deleteRecursively()
    invoke.resolve()
  }

  private fun openDocument(): Intent =
    Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("*/*")

  private fun describe(uri: Uri): Pair<String, Long> {
    var name = uri.lastPathSegment ?: "file"
    var size = -1L
    activity.contentResolver.query(uri, null, null, null, null)?.use { cursor ->
      if (cursor.moveToFirst()) {
        val n = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        val s = cursor.getColumnIndex(OpenableColumns.SIZE)
        if (n >= 0 && !cursor.isNull(n)) name = cursor.getString(n)
        if (s >= 0 && !cursor.isNull(s)) size = cursor.getLong(s)
      }
    }
    return Pair(name.substringAfterLast('/'), size)
  }

  private fun driverDir(id: String): File? {
    if (!id.matches(Regex("[A-Za-z0-9._-]+")) || id == "selected") return null
    val dir = File(drivers, id)
    return if (meta(dir) != null) dir else null
  }

  private fun meta(dir: File): JSONObject? {
    val file = File(dir, "meta.json")
    if (!file.isFile) return null
    return try {
      val meta = JSONObject(file.readText())
      if (meta.optString("libraryName").isNotEmpty()) meta else null
    } catch (e: Exception) {
      null
    }
  }

  private fun selectedDriver(): Pair<File, JSONObject>? {
    if (!selectedFile.isFile) return null
    val dir = driverDir(selectedFile.readText().trim()) ?: return null
    return meta(dir)?.let { Pair(dir, it) }
  }

  private fun view(id: String, meta: JSONObject): JSObject {
    val out = JSObject()
    out.put("id", id)
    out.put("name", meta.optString("name", id))
    out.put("description", meta.optString("description"))
    out.put("author", meta.optString("author"))
    out.put("version", meta.optString("packageVersion"))
    out.put("vendor", meta.optString("vendor"))
    out.put("driverVersion", meta.optString("driverVersion"))
    out.put("minApi", meta.optInt("minApi", 0))
    out.put("library", meta.optString("libraryName"))
    return out
  }

  private fun device(): JSObject {
    val out = JSObject()
    out.put("model", "${Build.MANUFACTURER} ${Build.MODEL}")
    out.put("api", Build.VERSION.SDK_INT)
    out.put("soc", if (Build.VERSION.SDK_INT >= 31) "${Build.SOC_MANUFACTURER} ${Build.SOC_MODEL}" else "")
    // Which kernel GPU driver the device has: packages load only on Adreno's (kgsl).
    out.put("gpu", when {
      File("/dev/kgsl-3d0").exists() -> "adreno"
      File("/dev/mali0").exists() -> "mali"
      else -> "other"
    })
    return out
  }

  /** Checks the package (a meta.json naming a library that is in it), then unpacks it whole. */
  private fun unpackDriver(uri: Uri): Pair<String, JSONObject> {
    val names = mutableSetOf<String>()
    var meta: JSONObject? = null
    activity.contentResolver.openInputStream(uri)?.use { input ->
      ZipInputStream(input.buffered()).use { zip ->
        while (true) {
          val entry = zip.nextEntry ?: break
          names.add(entry.name)
          if (entry.name == "meta.json") {
            meta = JSONObject(zip.readBytes().toString(Charsets.UTF_8))
          }
        }
      }
    } ?: throw IllegalStateException("the chosen file cannot be opened")
    val found = meta ?: throw IllegalArgumentException("not a driver package: it has no meta.json")
    val library = found.optString("libraryName")
    if (library.isEmpty() || library !in names) {
      throw IllegalArgumentException("not a driver package: meta.json names no library that is in it")
    }
    val minApi = found.optInt("minApi", 0)
    if (Build.VERSION.SDK_INT < minApi) {
      throw IllegalArgumentException("this package needs Android API $minApi; this device has ${Build.VERSION.SDK_INT}")
    }
    val base = (found.optString("name", "driver") + "-" + found.optString("packageVersion"))
      .replace(Regex("[^A-Za-z0-9._-]+"), "-").trim('-').ifEmpty { "driver" }
    var id = base
    var n = 2
    while (File(drivers, id).exists()) id = "$base-${n++}"
    val dir = File(drivers, id)
    val partial = File(drivers, ".$id.partial")
    partial.deleteRecursively()
    partial.mkdirs()
    val canonical = partial.canonicalPath + File.separator
    activity.contentResolver.openInputStream(uri)?.use { input ->
      ZipInputStream(input.buffered()).use { zip ->
        while (true) {
          val entry = zip.nextEntry ?: break
          val dest = File(partial, entry.name)
          if (!dest.canonicalPath.startsWith(canonical)) {
            throw SecurityException("the package names a file outside it: ${entry.name}")
          }
          if (entry.isDirectory) {
            dest.mkdirs()
          } else {
            dest.parentFile?.mkdirs()
            dest.outputStream().use { zip.copyTo(it) }
          }
        }
      }
    }
    if (!partial.renameTo(dir)) {
      partial.deleteRecursively()
      throw IllegalStateException("cannot keep the package in $dir")
    }
    return Pair(id, found)
  }
}
