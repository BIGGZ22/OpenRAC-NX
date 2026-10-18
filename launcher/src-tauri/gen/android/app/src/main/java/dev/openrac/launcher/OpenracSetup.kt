// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.content.Context
import android.os.Build
import android.system.Os
import android.util.Log
import java.io.File
import java.io.FileOutputStream
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
 */
object OpenracSetup {
  private const val TAG = "OpenRAC"
  private const val BUNDLE = "openrac.zip"

  fun root(context: Context): File = File(context.filesDir, "openrac")

  fun discs(context: Context): File = File(context.filesDir, "discs")

  fun nativeLibraries(context: Context): File = File(context.applicationInfo.nativeLibraryDir)

  /** Unpacks the carried copy when this APK's differs from the one unpacked, links its programs
   *  and sets the environment. Runs before the launcher's Rust side starts. */
  fun prepare(context: Context) {
    val root = root(context)
    try {
      unpack(context, root)
    } catch (e: Exception) {
      Log.e(TAG, "cannot unpack OpenRAC's tools: $e")
    }
    val libraries = nativeLibraries(context)
    link(File(libraries, "libopenrac-python.so"), File(root, "python/bin/python3"))
    libraries.listFiles()?.forEach { library ->
      val game = Regex("libopenrac-(.+)-cli\\.so").matchEntire(library.name)?.groupValues?.get(1)
      if (game != null) {
        link(library, File(root, "port/build/release/openrac-$game"))
      }
    }
    discs(context).mkdirs()
    environment(context).forEach { (name, value) -> Os.setenv(name, value, true) }
  }

  /** What the launcher and every program it starts run with; the game's activity gets it too. */
  fun environment(context: Context): Map<String, String> {
    val root = root(context)
    val libraries = nativeLibraries(context).path
    return mapOf(
      "HOME" to context.filesDir.path,
      "TMPDIR" to context.cacheDir.path,
      "XDG_DATA_HOME" to File(context.filesDir, "share").path,
      "XDG_CONFIG_HOME" to File(context.filesDir, "config").path,
      "OPENRAC_BUNDLED_ROOT" to root.path,
      "OPENRAC_DISCS_DIR" to discs(context).path,
      "OPENRAC_NATIVE_LIBRARY_DIR" to libraries,
      "PYTHONHOME" to File(root, "python").path,
      "PYTHONDONTWRITEBYTECODE" to "1",
      // libpython and the libraries its modules need are in the native library folder, as are
      // SDL and the C++ runtime the game's program needs.
      "LD_LIBRARY_PATH" to libraries,
    )
  }

  private fun unpack(context: Context, root: File) {
    val info = context.packageManager.getPackageInfo(context.packageName, 0)
    val stamp = "${info.longVersionCode} ${info.lastUpdateTime}"
    val stampFile = File(root, ".unpacked")
    if (stampFile.isFile && stampFile.readText() == stamp) {
      return
    }
    val names = context.assets.list("")?.toList() ?: emptyList()
    if (BUNDLE !in names) {
      Log.w(TAG, "this APK carries no $BUNDLE: the launcher has no tools to set a game up")
      return
    }
    root.deleteRecursively()
    root.mkdirs()
    val canonical = root.canonicalPath + File.separator
    ZipInputStream(context.assets.open(BUNDLE).buffered()).use { zip ->
      while (true) {
        val entry = zip.nextEntry ?: break
        val dest = File(root, entry.name)
        if (!dest.canonicalPath.startsWith(canonical)) {
          throw SecurityException("$BUNDLE names a file outside it: ${entry.name}")
        }
        if (entry.isDirectory) {
          dest.mkdirs()
        } else {
          dest.parentFile?.mkdirs()
          FileOutputStream(dest).use { zip.copyTo(it) }
        }
      }
    }
    stampFile.writeText(stamp)
    Log.i(TAG, "unpacked OpenRAC's tools into $root (${Build.MODEL})")
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
