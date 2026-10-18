# The port on Android

How to build the APK and what to test on a device. The design, and why it
is built this way, is [docs/port/ANDROID.md](../../docs/port/ANDROID.md).

| Path | What it is |
|---|---|
| `compat/` | what bionic lacks below API 33 (`backtrace`), first on the include path of an Android build |
| `gpu_driver.cpp` | the player's Vulkan driver package, loaded through libadrenotools on Adreno (platform/gpu_driver.h) |
| `python_main.c` | `libopenrac-python.so`, the `python3` program the launcher runs its tools with |
| `java/dev/openrac/game/GameActivity.java` | the game's activity: SDL's, around `libopenrac-<id>.so` |
| `../cmake/Android.cmake` | what an Android build adds; the `android-arm64` preset in `../CMakePresets.json` |

The launcher's side is in `launcher/src-tauri/gen/android` (Kotlin:
`OpenracSetup.kt`, `OpenracPlugin.kt`) and `launcher/src-tauri/src/android.rs`.

## Building

You need the Android SDK (`ANDROID_HOME`) with an NDK, r27 or newer
(`ANDROID_NDK_HOME`), JDK 17 or newer, Rust with the `aarch64-linux-android`
target, Node with Corepack, CMake, Ninja and a host Clang (hostgen runs on
the build machine). Then, from the repository's root:

```sh
rustup target add aarch64-linux-android
python3 tools/build_android.py               # rac1-pal and rac1-ntsc, a debug APK
python3 tools/build_android.py rac1-pal      # one game
python3 tools/build_android.py --no-apk      # everything but Gradle: open launcher/src-tauri/gen/android in Android Studio
python3 tools/build_android.py --release     # an unsigned release APK, to sign with your key
```

The APK is in
`launcher/src-tauri/gen/android/app/build/outputs/apk/universal/debug/`
(Tauri's flavour folder). CI builds the same debug APK beside the desktop
installers (`.github/workflows/launcher.yml`, the `android` rows of its
matrices, artifact `openrac-launcher-android`), and a `launcher-v*` tag puts it on the draft
release with them.

Only the port, without the launcher:

```sh
cd port
cmake --preset android-arm64 && cmake --build build/android-arm64 --target openrac-rac1-pal openrac-rac1-pal-cli
```

`-DOPENRAC_ANDROID_ADRENOTOOLS=OFF` builds without libadrenotools (the
system's driver always).

## Installing and reading the logs

```sh
adb install -r app-universal-debug.apk
adb logcat -s OpenRAC SDL SDL/APP openrac
```

The game writes its own log as on a desktop, in its data folder
(`files/OpenRAC/active/<game>/data/`); the launcher shows it on the game's
page with the crash reports. `adb shell run-as dev.openrac.launcher ls files`
lists the app's files on a debug build.

## Testing on a device

Nothing here can run in CI: there is no phone and no disc. Each step says
what to look for; report what you see with the device, its GPU (Settings,
GPU driver, shows the model and SoC) and the game's log.

1. **Install and start.** The launcher opens on the library. The first start
   unpacks the tools (a few seconds); `adb logcat -s OpenRAC` says
   "unpacked OpenRAC's tools".
2. **Add the disc.** "Choose disc image" opens Android's picker; pick your
   own image of the disc (.iso, or a .chd made with `chdman createdvd`, which
   is unpacked into an .iso). Toasts count the copy in quarters; then the
   set-up runs by itself: the extractor, the levels (a few minutes on a
   phone: one level at a time), the check of the levels. On a device with
   less free space than the image, the launcher says how much it needs.
3. **Play, on Mali.** Play opens the game in landscape: the menus over the
   title world, then a level. Check the picture against the desktop's
   (lighting, the sky, the 2D layer, text), the frame rate and the sound.
4. **Play, on Adreno.** The same, with the system's driver.
5. **Controls.** Touch the screen: the on-screen pad shows (sticks, d-pad,
   face buttons, shoulders, Select, Start) and moves Ratchet. Connect a
   Bluetooth or USB controller and press a button: the pad hides and the
   controller plays.
6. **Suspend and resume.** Switch to another app and back, and lock and
   unlock the screen, during a level: the game comes back where it was, with
   no fingers left held.
7. **Play again.** Quit the game (its menu, or Back), then Play again: it
   starts afresh. A crash leaves the launcher open with the crash report on
   the game's page.
8. **A driver package, on Adreno.** Settings, GPU driver, "Add a driver
   package (.zip)": pick a Mesa Turnip package (for example from
   K11MCH1/AdrenoToolsDrivers releases, made for Android emulators). It is
   listed with its name, version and library; choose it and play. The
   game's log says `GPU driver: the player's package, Turnip Adreno (TM) ...`
   with the driver's version. The picture still comes from OpenGL ES and the
   system's driver: the Vulkan renderer that draws on the package is the
   next change. Choose "The system's driver" and play again: the log says
   so. A zip without a `meta.json` is refused with a message.
9. **A driver package, on Mali.** The section says packages are for Adreno
   and the choice is greyed out; the game's log says why the system's
   driver is used.
10. **ANGLE.** In Developer options, "ANGLE preferences", choose ANGLE for
    OpenRAC Launcher and play: the game draws through ANGLE (its log names
    the renderer).
