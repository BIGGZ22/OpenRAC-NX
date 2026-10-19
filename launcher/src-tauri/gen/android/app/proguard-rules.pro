# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile
# SDL's Java and the game's activity are called from native code by name (JNI).
-keep class org.libsdl.app.** { *; }
-keep class dev.openrac.game.** { *; }
# OpenRAC's Tauri plugin is found by name (src-tauri/src/android.rs).
-keep class dev.openrac.launcher.OpenracPlugin { *; }
