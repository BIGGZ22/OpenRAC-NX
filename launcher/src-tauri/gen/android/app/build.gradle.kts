import java.util.Properties
import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("rust")
}

val tauriProperties = Properties().apply {
    val propFile = file("tauri.properties")
    if (propFile.exists()) {
        propFile.inputStream().use { load(it) }
    }
}

android {
    compileSdk = 37
    namespace = "dev.openrac.launcher"
    // The NDK the libraries were built with (tools/build_android.py, CI) also strips them below,
    // whatever NDK this Gradle plugin would pick by itself. Its version is named too: the plugin
    // strips nothing when the version it expects is not the one at ndkPath.
    (System.getenv("ANDROID_NDK_HOME") ?: System.getenv("NDK_HOME"))?.takeIf { it.isNotEmpty() }?.let { home ->
        ndkPath = home
        val source = file("$home/source.properties")
        if (source.exists()) {
            Properties().apply { source.inputStream().use { load(it) } }.getProperty("Pkg.Revision")?.let { ndkVersion = it }
        }
    }
    defaultConfig {
        manifestPlaceholders["usesCleartextTraffic"] = "false"
        // A fork's build is an app of its own (.github/workflows/launcher.yml sets these outside
        // OpenRAC/OpenRAC), installed beside the project's with its own name and data; the
        // project's own build keeps the defaults.
        applicationId = System.getenv("OPENRAC_ANDROID_APPLICATION_ID")?.takeIf { it.isNotEmpty() }
            ?: "dev.openrac.launcher"
        manifestPlaceholders["appLabel"] = System.getenv("OPENRAC_ANDROID_APP_LABEL")?.takeIf { it.isNotEmpty() }
            ?: "OpenRAC Launcher"
        // Android 9: libadrenotools' driver loading starts there, so a custom Vulkan driver (and
        // with it Enhanced graphics) works on every supported version. Newer APIs are checked at
        // run time (docs/port/ANDROID.md).
        minSdk = 28
        targetSdk = 37
        // CI numbers each build (.github/workflows/launcher.yml): Android's home screens take a
        // new icon or name only from an APK whose version code changed.
        versionCode = System.getenv("OPENRAC_ANDROID_VERSION_CODE")?.toIntOrNull()
            ?: tauriProperties.getProperty("tauri.android.versionCode", "1").toInt()
        versionName = tauriProperties.getProperty("tauri.android.versionName", "1.0")
        // The game's memory is a 4 GB reservation: 64-bit Arm only.
        ndk {
            abiFilters += "arm64-v8a"
        }
    }
    sourceSets {
        getByName("main") {
            // The game's activity (port/android/java) and SDL's own Java, which
            // tools/build_android.py copies from the SDL the port was built with.
            java.srcDirs("src/main/sdl-java", "../../../../../port/android/java")
        }
    }
    // The launcher's own key, when the build is given one (CI's secrets, .github/workflows/launcher.yml):
    // Android installs an update only when it is signed as the launcher installed is, so every
    // release is signed with this one. Without it a build is signed with the machine's debug key.
    val keystore = System.getenv("OPENRAC_ANDROID_KEYSTORE")?.takeIf { it.isNotEmpty() }
    if (keystore != null) {
        signingConfigs {
            create("openrac") {
                storeFile = file(keystore)
                storePassword = System.getenv("OPENRAC_ANDROID_KEYSTORE_PASSWORD")
                keyAlias = System.getenv("OPENRAC_ANDROID_KEY_ALIAS")
                keyPassword = System.getenv("OPENRAC_ANDROID_KEY_PASSWORD")
            }
        }
    }
    packaging {
        // Installed as files in the native library folder, not read from the APK: the launcher
        // runs Python and the game's checks from there, and libadrenotools loads its hooks from
        // there by path.
        jniLibs.useLegacyPackaging = true
    }
    buildTypes {
        getByName("debug") {
            if (keystore != null) signingConfig = signingConfigs.getByName("openrac")
            manifestPlaceholders["usesCleartextTraffic"] = "true"
            isDebuggable = true
            isJniDebuggable = true
            isMinifyEnabled = false
            // The libraries go in without their debug information, as a release's do: the APK and
            // each update a fraction of the size. tools/build_android.py --keep-symbols keeps it.
            if (System.getenv("OPENRAC_ANDROID_KEEP_SYMBOLS") == "1") {
                packaging {
                    jniLibs.keepDebugSymbols.add("*/arm64-v8a/*.so")
                }
            }
        }
        getByName("release") {
            if (keystore != null) signingConfig = signingConfigs.getByName("openrac")
            optimization {
               enable = true
            }
            proguardFiles(
                *fileTree(".") {
                  include("**/*.pro")
                  exclude("build/**")
                }.files.toTypedArray()
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    buildFeatures {
        buildConfig = true
    }
}

kotlin {
    compilerOptions {
        jvmTarget = JvmTarget.JVM_17
    }
}

rust {
    rootDirRel = "../../../"
}

dependencies {
    implementation("androidx.webkit:webkit:1.14.0")
    implementation("androidx.appcompat:appcompat:1.7.1")
    implementation("androidx.activity:activity-ktx:1.10.1")
    implementation("com.google.android.material:material:1.12.0")
    implementation("androidx.lifecycle:lifecycle-process:2.10.0")
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.4")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.0")
}

apply(from = file("tauri.build.gradle.kts"))
