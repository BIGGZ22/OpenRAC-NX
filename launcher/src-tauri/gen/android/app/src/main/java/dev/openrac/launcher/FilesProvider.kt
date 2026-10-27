// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.launcher

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.database.Cursor
import android.database.MatrixCursor
import android.net.Uri
import android.os.CancellationSignal
import android.os.ParcelFileDescriptor
import android.os.StatFs
import android.provider.DocumentsContract
import android.provider.DocumentsContract.Document
import android.provider.DocumentsContract.Root
import android.provider.DocumentsProvider
import android.webkit.MimeTypeMap
import java.io.File
import java.io.FileNotFoundException

/**
 * The player's OpenRAC files in the system's Files app: "OpenRAC" in its list of places, with the
 * space left on the device, as emulators show theirs (Dolphin, Eden). The files stay in the app's
 * own storage, where the launcher and the game read them; the Files app copies, adds, renames and
 * deletes them through here. These folders, and not the tools the APK unpacks (OpenracSetup.kt):
 *
 *   Saves and settings  files/share/openrac  the memory cards and each game's settings
 *   Games               files/games          each game's set-up from its disc, its mods
 *   Disc images         files/discs          the images picked in the launcher
 *   GPU drivers         files/gpu_drivers    the driver packages (OpenracPlugin.kt)
 *   Screenshots         files/screenshots    the drawer's screenshots (debug_overlay.cpp)
 *   Diagnostics         files/perf           the drawer's diagnostics recordings (debug/capture.cpp)
 *
 * A document's id is "root", a folder's key, or "<key>/<path inside it>".
 */
class FilesProvider : DocumentsProvider() {
  private class Place(val key: String, val title: String, val path: String)

  companion object {
    private const val ROOT = "root"
    private val PLACES = listOf(
      Place("saves", "Saves and settings", "share/openrac"),
      Place("games", "Games", "games"),
      Place("discs", "Disc images", "discs"),
      Place("drivers", "GPU drivers", "gpu_drivers"),
      Place("screenshots", "Screenshots", "screenshots"),
      Place("perf", "Diagnostics recordings", "perf"),
    )
    private val ROOT_COLUMNS = arrayOf(
      Root.COLUMN_ROOT_ID, Root.COLUMN_FLAGS, Root.COLUMN_ICON, Root.COLUMN_TITLE,
      Root.COLUMN_DOCUMENT_ID, Root.COLUMN_AVAILABLE_BYTES,
    )
    private val DOCUMENT_COLUMNS = arrayOf(
      Document.COLUMN_DOCUMENT_ID, Document.COLUMN_MIME_TYPE, Document.COLUMN_DISPLAY_NAME,
      Document.COLUMN_LAST_MODIFIED, Document.COLUMN_FLAGS, Document.COLUMN_SIZE,
    )

    fun authority(context: Context): String = "${context.packageName}.files"

    /** The folder of the app's files a place is. */
    private fun folder(context: Context, place: Place): File = File(context.filesDir, place.path)

    /** The document a file of the app's is, or null when it is in none of its folders. */
    fun documentId(context: Context, file: File): String? {
      val path = file.absoluteFile.normalize()
      for (place in PLACES) {
        val base = folder(context, place).absoluteFile.normalize()
        if (path == base) return place.key
        if (path.startsWith(base)) return "${place.key}/${path.toRelativeString(base)}"
      }
      return null
    }

    /** Opens the Files app at `file` (one of its folders, or one inside them), else at OpenRAC's
     *  own place; false when the device has no Files app that takes it. */
    fun show(context: Context, file: File?): Boolean {
      val authority = authority(context)
      val id = file?.let { documentId(context, it) }
      val flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_GRANT_READ_URI_PERMISSION or
        Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
      val tries = mutableListOf<Intent>()
      if (id != null) {
        tries += Intent(Intent.ACTION_VIEW)
          .setDataAndType(DocumentsContract.buildDocumentUri(authority, id), Document.MIME_TYPE_DIR)
      }
      val root = DocumentsContract.buildRootUri(authority, ROOT)
      tries += Intent(Intent.ACTION_VIEW).setDataAndType(root, Root.MIME_TYPE_ITEM)
      tries += Intent(Intent.ACTION_VIEW, root)
      tries += Intent("android.provider.action.BROWSE", root)
      // Phones whose Files app answers none of these: the Files app itself.
      for (pkg in listOf("com.google.android.documentsui", "com.android.documentsui")) {
        tries += Intent(Intent.ACTION_MAIN).setClassName(pkg, "com.android.documentsui.files.FilesActivity")
      }
      for (intent in tries) {
        try {
          context.startActivity(intent.addCategory(Intent.CATEGORY_DEFAULT).addFlags(flags))
          return true
        } catch (e: ActivityNotFoundException) {
          continue
        } catch (e: SecurityException) {
          continue
        }
      }
      return false
    }
  }

  private val files: Context get() = context ?: throw IllegalStateException("no context")

  override fun onCreate(): Boolean = true

  override fun queryRoots(projection: Array<String>?): Cursor {
    val cursor = MatrixCursor(projection ?: ROOT_COLUMNS)
    cursor.newRow().apply {
      add(Root.COLUMN_ROOT_ID, ROOT)
      add(Root.COLUMN_DOCUMENT_ID, ROOT)
      add(Root.COLUMN_TITLE, "OpenRAC")
      add(Root.COLUMN_ICON, R.mipmap.ic_launcher)
      add(Root.COLUMN_FLAGS, Root.FLAG_LOCAL_ONLY or Root.FLAG_SUPPORTS_CREATE or Root.FLAG_SUPPORTS_IS_CHILD)
      // What the Files app shows under the name: "12 GB free".
      add(Root.COLUMN_AVAILABLE_BYTES, StatFs(files.filesDir.path).availableBytes)
    }
    return cursor
  }

  override fun queryDocument(documentId: String, projection: Array<String>?): Cursor {
    val cursor = MatrixCursor(projection ?: DOCUMENT_COLUMNS)
    when (val place = place(documentId)) {
      null -> addRoot(cursor)
      else -> addFile(cursor, file(documentId), documentId, place.title.takeIf { documentId == place.key })
    }
    return cursor
  }

  override fun queryChildDocuments(parentId: String, projection: Array<String>?, sortOrder: String?): Cursor {
    val cursor = MatrixCursor(projection ?: DOCUMENT_COLUMNS)
    if (place(parentId) == null) {
      for (place in PLACES) {
        val dir = folder(files, place)
        dir.mkdirs()
        addFile(cursor, dir, place.key, place.title)
      }
    } else {
      file(parentId).listFiles()?.sortedBy { it.name.lowercase() }?.forEach { child ->
        addFile(cursor, child, "$parentId/${child.name}", null)
      }
    }
    cursor.setNotificationUri(files.contentResolver, childrenUri(parentId))
    return cursor
  }

  override fun openDocument(documentId: String, mode: String, signal: CancellationSignal?): ParcelFileDescriptor {
    val file = file(documentId)
    if (file.isDirectory) throw FileNotFoundException("$documentId is a folder")
    return ParcelFileDescriptor.open(file, ParcelFileDescriptor.parseMode(mode))
  }

  override fun createDocument(parentId: String, mimeType: String, displayName: String): String {
    if (place(parentId) == null) throw UnsupportedOperationException("OpenRAC's place holds only its folders")
    val name = plainName(displayName)
    val parent = file(parentId)
    var made = File(parent, name)
    var n = 1
    while (made.exists()) {
      val dot = name.lastIndexOf('.').takeIf { it > 0 && mimeType != Document.MIME_TYPE_DIR } ?: name.length
      made = File(parent, "${name.substring(0, dot)} ($n)${name.substring(dot)}")
      n += 1
    }
    val ok = if (mimeType == Document.MIME_TYPE_DIR) made.mkdir() else made.createNewFile()
    if (!ok) throw FileNotFoundException("cannot create $name")
    changed(parentId)
    return "$parentId/${made.name}"
  }

  override fun deleteDocument(documentId: String) {
    if (!documentId.contains('/')) throw UnsupportedOperationException("its folders stay")
    if (!file(documentId).deleteRecursively()) throw FileNotFoundException("cannot delete $documentId")
    changed(documentId.substringBeforeLast('/'))
  }

  override fun renameDocument(documentId: String, displayName: String): String {
    if (!documentId.contains('/')) throw UnsupportedOperationException("its folders keep their names")
    val file = file(documentId)
    val renamed = File(file.parentFile, plainName(displayName))
    if (renamed.exists() || !file.renameTo(renamed)) throw FileNotFoundException("cannot rename $documentId")
    val parentId = documentId.substringBeforeLast('/')
    changed(parentId)
    return "$parentId/${renamed.name}"
  }

  override fun isChildDocument(parentId: String, documentId: String): Boolean =
    parentId == ROOT || documentId.startsWith("$parentId/")

  /** The place a document is in: null for the root itself. */
  private fun place(documentId: String): Place? {
    if (documentId == ROOT) return null
    val key = documentId.substringBefore('/')
    return PLACES.firstOrNull { it.key == key } ?: throw FileNotFoundException("no document $documentId")
  }

  /** The file a document is: its place's folder and the path inside it, with no step out of it. */
  private fun file(documentId: String): File {
    val place = place(documentId) ?: throw FileNotFoundException("the root is no file")
    val inside = documentId.substringAfter('/', "")
    val parts = if (inside.isEmpty()) emptyList() else inside.split('/')
    if (parts.any { it.isEmpty() || it == "." || it == ".." }) throw FileNotFoundException("no document $documentId")
    val file = parts.fold(folder(files, place)) { dir, part -> File(dir, part) }
    if (!file.exists()) throw FileNotFoundException("no document $documentId")
    return file
  }

  private fun plainName(name: String): String {
    val plain = name.replace('/', '_').trim()
    if (plain.isEmpty() || plain == "." || plain == "..") throw IllegalArgumentException("no name")
    return plain
  }

  private fun addRoot(cursor: MatrixCursor) {
    cursor.newRow().apply {
      add(Document.COLUMN_DOCUMENT_ID, ROOT)
      add(Document.COLUMN_MIME_TYPE, Document.MIME_TYPE_DIR)
      add(Document.COLUMN_DISPLAY_NAME, "OpenRAC")
      add(Document.COLUMN_FLAGS, 0)
      add(Document.COLUMN_SIZE, null)
    }
  }

  private fun addFile(cursor: MatrixCursor, file: File, id: String, title: String?) {
    val dir = file.isDirectory
    var flags = 0
    if (file.canWrite()) {
      flags = if (dir) Document.FLAG_DIR_SUPPORTS_CREATE else Document.FLAG_SUPPORTS_WRITE
      if (title == null) flags = flags or Document.FLAG_SUPPORTS_DELETE or Document.FLAG_SUPPORTS_RENAME
    }
    val mime = if (dir) {
      Document.MIME_TYPE_DIR
    } else {
      MimeTypeMap.getSingleton().getMimeTypeFromExtension(file.extension.lowercase()) ?: "application/octet-stream"
    }
    cursor.newRow().apply {
      add(Document.COLUMN_DOCUMENT_ID, id)
      add(Document.COLUMN_MIME_TYPE, mime)
      add(Document.COLUMN_DISPLAY_NAME, title ?: file.name)
      add(Document.COLUMN_LAST_MODIFIED, file.lastModified())
      add(Document.COLUMN_FLAGS, flags)
      add(Document.COLUMN_SIZE, if (dir) null else file.length())
    }
  }

  private fun childrenUri(parentId: String): Uri = DocumentsContract.buildChildDocumentsUri(authority(files), parentId)

  /** Tells the Files app to list PARENTID again. */
  private fun changed(parentId: String) = files.contentResolver.notifyChange(childrenUri(parentId), null)
}
