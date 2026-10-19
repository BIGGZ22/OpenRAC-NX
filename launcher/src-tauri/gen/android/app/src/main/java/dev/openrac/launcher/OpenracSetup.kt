// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.content.Context
import android.os.Build
import android.system.Os
import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.util.concurrent.CountDownLatch
import java.util.concurrent.atomic.AtomicBoolean
import java.util.zip.ZipInputStream

/**
 * What the desktop launcher finds in its installer's resources, put where the Android launcher
 * finds it (docs/port/ANDROID.md): the copy of OpenRAC it carries (tools/bundle_standalone.py
 * --android, the APK's assets/openrac.zip) unpacked into files/openrac, and the environment the
 * launcher's Rust side and the programs it runs read.
 *
 * Android runs programs only from the app's native library folder, so the Python and each game's
 * windowless program are libraries there (libopenrac-python.so, libopenrac-<id>-cli.so), linked
 * from where the desktop layout has them: openrac/python/bin/python3 and
 * openrac/port/build/release/openrac-<id>. The folder moves when the app is updated, so the links
 * are made again at every start.
 *
 * The copy is unpacked once per install or update, which takes seconds: on a thread of its own,
 * while the launcher's page waits for it (`toolsReady`, OpenracPlugin) and says so. It goes into
 * openrac.new and takes the place of openrac only when whole, so a launcher stopped half-way
 * unpacks it again at its next start.
 */
object OpenracSetup {
  private const val TAG = "OpenRAC"
  private const val BUNDLE = "openrac.zip"

  private val started = AtomicBoolean(false)
  private val ready = CountDownLatch(1)
  @Volatile private var failure: String? = null
  @Volatile private var carried = false

  /** Waits until the copy is ready; null when it is, or why it is not. */
  fun awaitReady(): String? {
    ready.await()
    return failure
  }

  fun root(context: Context): File = File(context.filesDir, "openrac")

  fun discs(context: Context): File = File(context.filesDir, "discs")

  fun nativeLibraries(context: Context): File = File(context.applicationInfo.nativeLibraryDir)

  /** Sets the environment, then unpacks the carried copy when this APK's differs from the one
   *  unpacked and links its programs, on a thread of its own when there is unpacking to do. Runs
   *  before the launcher's Rust side starts, once per process. */
  fun prepare(context: Context) {
    if (!started.compareAndSet(false, true)) return
    carried = carriesCopy(context)
    discs(context).mkdirs()
    environment(context).forEach { (name, value) -> Os.setenv(name, value, true) }
    val stamp = stamp(context)
    if (!carried || isUnpacked(context, stamp)) {
      finish(context)
      return
    }
    val app = context.applicationContext
    Thread({
      try {
        unpack(app, stamp)
      } catch (e: Exception) {
        Log.e(TAG, "cannot unpack OpenRAC's tools: $e")
        failure = "OpenRAC's tools could not be unpacked: ${e.message}"
      }
      finish(app)
    }, "openrac-unpack").start()
  }

  private fun finish(context: Context) {
    val root = root(context)
    val libraries = nativeLibraries(context)
    link(File(libraries, "libopenrac-python.so"), File(root, "python/bin/python3"))
    libraries.listFiles()?.forEach { library ->
      val game = Regex("libopenrac-(.+)-cli\\.so").matchEntire(library.name)?.groupValues?.get(1)
      if (game != null) {
        link(library, File(root, "port/build/release/openrac-$game"))
      }
    }
    ready.countDown()
  }

  /** What the launcher and every program it starts run with; the game's activity gets it too.
   *  OPENRAC_BUNDLED_ROOT only when the APK carries the copy: the Rust side then works from it. */
  fun environment(context: Context): Map<String, String> {
    val root = root(context)
    val libraries = nativeLibraries(context).path
    val copy = if (carried) mapOf("OPENRAC_BUNDLED_ROOT" to root.path) else emptyMap()
    return copy + mapOf(
      "HOME" to context.filesDir.path,
      "TMPDIR" to context.cacheDir.path,
      "XDG_DATA_HOME" to File(context.filesDir, "share").path,
      "XDG_CONFIG_HOME" to File(context.filesDir, "config").path,
      "OPENRAC_DISCS_DIR" to discs(context).path,
      "OPENRAC_NATIVE_LIBRARY_DIR" to libraries,
      "PYTHONHOME" to File(root, "python").path,
      "PYTHONDONTWRITEBYTECODE" to "1",
      // libpython and the libraries its modules need are in the native library folder, as are
      // SDL and the C++ runtime the game's program needs.
      "LD_LIBRARY_PATH" to libraries,
    )
  }

  /** The APK the copy came from: a new install or update unpacks it again. */
  private fun stamp(context: Context): String {
    val info = context.packageManager.getPackageInfo(context.packageName, 0)
    return "${info.longVersionCode} ${info.lastUpdateTime}"
  }

  private fun isUnpacked(context: Context, stamp: String): Boolean {
    val file = File(root(context), ".unpacked")
    return file.isFile && file.readText() == stamp
  }

  private fun carriesCopy(context: Context): Boolean {
    if (context.assets.list("")?.contains(BUNDLE) == true) return true
    Log.w(TAG, "this APK carries no $BUNDLE: the launcher has no tools to set a game up")
    return false
  }

  private fun unpack(context: Context, stamp: String) {
    val started = System.nanoTime()
    val root = root(context)
    val fresh = File(context.filesDir, "openrac.new")
    val old = File(context.filesDir, "openrac.old")
    fresh.deleteRecursively()
    old.deleteRecursively()
    fresh.mkdirs()
    val canonical = fresh.canonicalPath + File.separator
    ZipInputStream(context.assets.open(BUNDLE).buffered()).use { zip ->
      while (true) {
        val entry = zip.nextEntry ?: break
        val dest = File(fresh, entry.name)
        if (!dest.canonicalPath.startsWith(canonical)) {
          throw SecurityException("$BUNDLE names a file outside it: ${entry.name}")
        }
        if (entry.isDirectory) {
          dest.mkdirs()
        } else {
          dest.parentFile?.mkdirs()
          FileOutputStream(dest).use { zip.copyTo(it, 1 shl 16) }
        }
      }
    }
    File(fresh, ".unpacked").writeText(stamp)
    if (root.exists() && !root.renameTo(old)) throw IllegalStateException("cannot move $root aside")
    if (!fresh.renameTo(root)) throw IllegalStateException("cannot move $fresh to $root")
    old.deleteRecursively()
    val ms = (System.nanoTime() - started) / 1_000_000
    Log.i(TAG, "unpacked OpenRAC's tools into $root in $ms ms (${Build.MODEL})")
  }

  private fun link(target: File, link: File) {
    link.parentFile?.mkdirs()
    link.delete()
    if (target.isFile) {
      try {
        Os.symlink(target.path, link.path)
      } catch (e: Exception) {
        Log.w(TAG, "cannot link $link to $target: $e")
      }
    }
  }
}
