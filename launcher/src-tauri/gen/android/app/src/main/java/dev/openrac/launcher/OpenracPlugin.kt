// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.app.Activity
import android.app.ActivityManager
import android.app.PendingIntent
import android.content.Intent
import android.content.pm.PackageInstaller
import android.net.Uri
import android.os.Build
import android.os.StatFs
import android.provider.OpenableColumns
import android.provider.Settings
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
import java.net.HttpURLConnection
import java.net.URL
import java.util.zip.ZipInputStream
import org.json.JSONObject

@InvokeArg
class LaunchArgs {
  lateinit var library: String
  var args: Array<String> = arrayOf()
  /** NAME=value: what the action adds (the detailed log, developer tools, Enhanced graphics). */
  var env: Array<String> = arrayOf()
}

@InvokeArg
class StatusArgs {
  var title: String? = null
  var text: String = ""
  var percent: Int? = null
}

@InvokeArg
class DownloadArgs {
  lateinit var url: String
  lateinit var path: String
  // The download downloadProgress reports (the update's APK), not a manifest read beside it.
  var progress: Boolean = false
  // The media type asked for, when not empty: GitHub's API for a mod's releases (remote.rs).
  var accept: String = ""
}

@InvokeArg
class SaveArgs {
  lateinit var path: String
  lateinit var name: String
  var mime: String = "application/zip"
}

@InvokeArg
class FolderArgs {
  var path: String? = null
}

@InvokeArg
class DriverArgs {
  var id: String? = null
}

@InvokeArg
class RendererArgs {
  var renderer: String? = null
}

/**
 * What only Android's own APIs can do for the launcher (src-tauri/src/android.rs calls these):
 * pick a disc image and save a file through the system's screens (an app sees no paths outside its
 * own folders), wait for the tools it carries (OpenracSetup.kt), show a set-up's progress while it
 * is in the background (SetupService.kt), download and install its own updates (UpdateReceiver.kt),
 * start and stop the game's activity, and keep the player's GPU driver packages.
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
  // "vulkan" when the game draws with its Vulkan renderer; absent: OpenGL ES.
  private val rendererFile: File get() = File(activity.filesDir, "renderer")
  // The library of the game started last, while it runs.
  @Volatile private var playing: String? = null

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
      // levels the launcher writes next (about 1 GB, checked when they are written). A CHD is
      // unpacked into a larger plain image: the Rust side checks that size against `free`.
      val free = StatFs(activity.filesDir.path).availableBytes
      val packed = name.endsWith(".chd", ignoreCase = true)
      if (!packed && size > 0 && size + (256L shl 20) > free) {
        invoke.reject("$name needs ${size shr 20} MB and the device has ${free shr 20} MB free")
        return
      }
      val fd = activity.contentResolver.openFileDescriptor(uri, "r")?.detachFd()
        ?: throw IllegalStateException("the picker's file cannot be opened")
      val out = JSObject()
      out.put("fd", fd)
      out.put("name", name)
      out.put("size", size)
      out.put("free", free)
      invoke.resolve(out)
    } catch (e: Exception) {
      invoke.reject("cannot read the chosen file: ${e.message}")
    }
  }

  // ---- saving a file where the player chooses ----

  /** {path, name, mime}: asks where to save the file at `path` (Android's own "save as" screen,
   *  `name` offered) and copies it there; {saved: the name it got}, or {} when the player backs out. */
  @Command
  fun saveFileAs(invoke: Invoke) {
    val args = invoke.parseArgs(SaveArgs::class.java)
    val intent = Intent(Intent.ACTION_CREATE_DOCUMENT)
      .addCategory(Intent.CATEGORY_OPENABLE)
      .setType(args.mime)
      .putExtra(Intent.EXTRA_TITLE, args.name)
    startActivityForResult(invoke, intent, "saveFilePicked")
  }

  @ActivityCallback
  fun saveFilePicked(invoke: Invoke, result: ActivityResult) {
    val uri = result.data?.data
    if (result.resultCode != Activity.RESULT_OK || uri == null) {
      invoke.resolve(JSObject())
      return
    }
    val source = File(invoke.parseArgs(SaveArgs::class.java).path)
    Thread {
      try {
        val out = activity.contentResolver.openOutputStream(uri, "wt")
          ?: throw IllegalStateException("the chosen place cannot be written")
        out.use { o -> source.inputStream().use { it.copyTo(o, 1 shl 20) } }
        val saved = JSObject()
        saved.put("saved", describe(uri).first)
        invoke.resolve(saved)
      } catch (e: Exception) {
        invoke.reject("the file could not be saved there: ${e.message}")
      }
    }.start()
  }

  // ---- the set-up, in the background ----

  /** Resolves once the tools the launcher carries are unpacked (OpenracSetup.prepare), with
   *  {error} when they could not be. */
  /** {path}: shows the folder in the Files app, through OpenRAC's place there (FilesProvider.kt);
   *  OpenRAC's place itself for a folder outside it, or no path. */
  @Command
  fun showFolder(invoke: Invoke) {
    val path = invoke.parseArgs(FolderArgs::class.java).path
    if (FilesProvider.show(activity, path?.let(::File))) {
      invoke.resolve()
    } else {
      invoke.reject("this device has no Files app to show the folder in")
    }
  }

  @Command
  fun toolsReady(invoke: Invoke) {
    Thread {
      val out = JSObject()
      OpenracSetup.awaitReady()?.let { out.put("error", it) }
      invoke.resolve(out)
    }.start()
  }

  /** {title, text, percent}: what the set-up's notification shows (SetupService.kt); no title:
   *  the set-up is over and the notification goes. MainActivity asks for notifications. */
  @Command
  fun backgroundStatus(invoke: Invoke) {
    val args = invoke.parseArgs(StatusArgs::class.java)
    val title = args.title
    if (title == null) {
      SetupService.hide(activity)
    } else {
      SetupService.show(activity, title, args.text, args.percent?.coerceIn(0, 100))
    }
    invoke.resolve()
  }

  // ---- the launcher's own updates (src-tauri/src/android.rs checks what is downloaded) ----

  @Volatile private var downloaded = 0L
  @Volatile private var downloadTotal = -1L

  /** {url, path, progress}: downloads an https address into a file, through a file beside it;
   *  with progress, it is the one downloadProgress reports. */
  @Command
  fun download(invoke: Invoke) {
    val args = invoke.parseArgs(DownloadArgs::class.java)
    if (!args.url.startsWith("https://")) return invoke.reject("not an https address: ${args.url}")
    if (args.progress) {
      downloaded = 0L
      downloadTotal = -1L
    }
    Thread {
      val dest = File(args.path)
      val partial = File(dest.path + ".partial")
      try {
        dest.parentFile?.mkdirs()
        val connection = URL(args.url).openConnection() as HttpURLConnection
        connection.connectTimeout = 15_000
        connection.readTimeout = 30_000
        connection.setRequestProperty("User-Agent", "OpenRAC-Launcher")
        if (args.accept.isNotEmpty()) {
          connection.setRequestProperty("Accept", args.accept)
          connection.setRequestProperty("X-GitHub-Api-Version", "2022-11-28")
        }
        try {
          // Redirects are followed within https only (GitHub's downloads are redirected once).
          if (connection.responseCode != HttpURLConnection.HTTP_OK || connection.url.protocol != "https") {
            throw IllegalStateException("${args.url}: HTTP ${connection.responseCode}")
          }
          if (args.progress) downloadTotal = connection.contentLengthLong
          connection.inputStream.use { input ->
            partial.outputStream().use { out ->
              val buffer = ByteArray(1 shl 16)
              while (true) {
                val n = input.read(buffer)
                if (n < 0) break
                out.write(buffer, 0, n)
                if (args.progress) downloaded += n
              }
            }
          }
        } finally {
          connection.disconnect()
        }
        if (!partial.renameTo(dest)) throw IllegalStateException("cannot write $dest")
        invoke.resolve()
      } catch (e: Exception) {
        partial.delete()
        invoke.reject("the download failed: ${e.message}")
      }
    }.start()
  }

  /** {downloaded, total}: how far the download is; total -1 while it is not known. */
  @Command
  fun downloadProgress(invoke: Invoke) {
    val out = JSObject()
    out.put("downloaded", downloaded)
    out.put("total", downloadTotal)
    invoke.resolve(out)
  }

  /** {path}: hands a checked APK to Android's installer, which asks the player to confirm
   *  (UpdateReceiver.kt). The first time, the player allows the launcher to install apps in
   *  Android's settings, and the install goes on when they come back: the APK is downloaded once. */
  @Command
  fun installApk(invoke: Invoke) {
    if (!activity.packageManager.canRequestPackageInstalls()) {
      val settings =
        Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES, Uri.parse("package:${activity.packageName}"))
      return startActivityForResult(invoke, settings, "installAllowed")
    }
    install(invoke)
  }

  /** Back from the settings: the screen answers nothing, so what counts is whether it is allowed now. */
  @ActivityCallback
  fun installAllowed(invoke: Invoke, @Suppress("UNUSED_PARAMETER") result: ActivityResult) {
    if (!activity.packageManager.canRequestPackageInstalls()) {
      return invoke.reject("OpenRAC Launcher may not install apps: allow it in Android's settings to update.")
    }
    install(invoke)
  }

  private fun install(invoke: Invoke) {
    val apk = File(invoke.parseArgs(DownloadArgs::class.java).path)
    Thread {
      val installer = activity.packageManager.packageInstaller
      var id = -1
      try {
        val params = PackageInstaller.SessionParams(PackageInstaller.SessionParams.MODE_FULL_INSTALL)
        params.setAppPackageName(activity.packageName)
        params.setSize(apk.length())
        id = installer.createSession(params)
        installer.openSession(id).use { session ->
          session.openWrite("base.apk", 0, apk.length()).use { out ->
            apk.inputStream().use { it.copyTo(out, 1 shl 20) }
            session.fsync(out)
          }
          // Mutable: the installer fills in its status.
          val flags = PendingIntent.FLAG_UPDATE_CURRENT or
            (if (Build.VERSION.SDK_INT >= 31) PendingIntent.FLAG_MUTABLE else 0)
          val status = PendingIntent.getBroadcast(
            activity, id, Intent(activity, UpdateReceiver::class.java).setPackage(activity.packageName), flags,
          )
          session.commit(status.intentSender)
        }
        invoke.resolve()
      } catch (e: Exception) {
        // A session left open keeps the APK's copy: the next try makes a new one.
        if (id >= 0) {
          try {
            installer.abandonSession(id)
          } catch (e: Exception) {
            // Already committed or gone.
          }
        }
        invoke.reject("the update could not be handed to Android's installer: ${e.message}")
      }
    }.start()
  }

  // ---- the game ----

  @Command
  fun launchGame(invoke: Invoke) {
    val args = invoke.parseArgs(LaunchArgs::class.java)
    val environment = OpenracSetup.environment(activity).toMutableMap()
    // A package loads only on Adreno's kernel driver; elsewhere the system's driver, whatever is
    // chosen (the choice cannot be changed on such a device).
    selectedDriver()?.takeIf { device().getString("gpu") == "adreno" }?.let { (dir, meta) ->
      environment["OPENRAC_GPU_DRIVER_DIR"] = dir.path
      environment["OPENRAC_GPU_DRIVER_LIBRARY"] = meta.getString("libraryName")
    }
    // The renderer: Vulkan's falls back to OpenGL ES by itself where the device has no Vulkan 1.3
    // (port/platform/window.cpp).
    if (renderer() == "vulkan") environment["OPENRAC_GL_API"] = "vulkan"
    // What the launcher's action adds; Enhanced graphics' OPENRAC_GL_API=vulkan among them, as
    // only the Vulkan renderer has it.
    args.env.forEach { entry ->
      val at = entry.indexOf('=')
      if (at > 0) environment[entry.substring(0, at)] = entry.substring(at + 1)
    }
    // Already playing: back to the game as it is, not a second start (unknown which runs, it is
    // the same one). Another game waits for it to end: both run in one process, which ends with
    // its game (GameActivity.onDestroy), so a start now would end with it.
    gameTask()?.let {
      if (playing != null && playing != args.library) {
        return invoke.reject("Another game is running: stop it first (Stop the game, in the menu on its page).")
      }
      it.moveToFront()
      return invoke.resolve()
    }
    playing = args.library
    // In a task of its own (the manifest's taskAffinity): the game and the launcher are two
    // windows in Recents, and going back to the launcher leaves the game where it was.
    val intent = Intent(activity, GameActivity::class.java)
      .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      .putExtra(GameActivity.EXTRA_LIBRARY, args.library)
      .putExtra(GameActivity.EXTRA_ARGUMENTS, args.args)
      .putExtra(GameActivity.EXTRA_ENVIRONMENT, environment.map { (k, v) -> "$k=$v" }.toTypedArray())
    activity.startActivity(intent)
    invoke.resolve()
  }

  /** The game's task, while its activity runs. */
  private fun gameTask(): ActivityManager.AppTask? =
    activity.getSystemService(ActivityManager::class.java).appTasks.firstOrNull { task ->
      try {
        val info = task.taskInfo ?: return@firstOrNull false
        info.isRunning && info.baseActivity?.className == GameActivity::class.java.name
      } catch (e: IllegalArgumentException) {
        false // the task went as it was read
      }
    }

  /** {running, library}: whether the game is running, and which (unknown after the launcher's
   *  own process was restarted while the game ran). */
  @Command
  fun gameStatus(invoke: Invoke) {
    val running = gameTask() != null
    if (!running) playing = null
    val out = JSObject()
    out.put("running", running)
    out.put("library", playing)
    invoke.resolve(out)
  }

  @Command
  fun returnToGame(invoke: Invoke) {
    val task = gameTask() ?: return invoke.reject("the game is not running")
    task.moveToFront()
    invoke.resolve()
  }

  /** Ends the game: its activity finishes, and its process with it (GameActivity.onDestroy). */
  @Command
  fun stopGame(invoke: Invoke) {
    gameTask()?.finishAndRemoveTask()
    playing = null
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
    out.put("renderer", renderer())
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

  /** {renderer}: "vulkan" or "gles", what the game draws with from its next start. */
  @Command
  fun selectRenderer(invoke: Invoke) {
    when (invoke.parseArgs(RendererArgs::class.java).renderer) {
      "vulkan" -> rendererFile.writeText("vulkan")
      "gles" -> rendererFile.delete()
      else -> return invoke.reject("no such renderer")
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

  private fun renderer(): String =
    if (rendererFile.isFile && rendererFile.readText().trim() == "vulkan") "vulkan" else "gles"

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
    try {
      unpackInto(uri, partial, canonical)
    } catch (e: Exception) {
      partial.deleteRecursively()
      throw e
    }
    if (!partial.renameTo(dir)) {
      partial.deleteRecursively()
      throw IllegalStateException("cannot keep the package in $dir")
    }
    return Pair(id, found)
  }

  private fun unpackInto(uri: Uri, partial: File, canonical: String) {
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
    } ?: throw IllegalStateException("the chosen file cannot be opened")
  }
}
