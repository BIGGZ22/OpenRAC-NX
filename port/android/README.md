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
`OpenracSetup.kt`, `OpenracPlugin.kt`, `SetupService.kt`, `UpdateReceiver.kt`) and `launcher/src-tauri/src/android.rs`.

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
installers (`.github/workflows/launcher.yml`, the `android` row of its
matrix, artifact `openrac-launcher-android`), and a `launcher-v*` tag puts it on the draft
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

CI numbers each APK's version code by its run (`OPENRAC_ANDROID_VERSION_CODE`,
above any version's own), so that a home screen takes a changed icon: a
local build installs over a CI build only after `adb uninstall
dev.openrac.launcher`, or with the variable set higher.

The game writes its own log as on a desktop, in its data folder
(`files/games/active/<game>/data/`); the launcher shows it on the game's
page with the crash reports. `adb shell run-as dev.openrac.launcher ls files`
lists the app's files on a debug build.

## Testing on a device

Nothing here can run in CI: there is no phone and no disc. Each step says
what to look for; report what you see with the device, its GPU (Settings,
GPU driver, shows the model and SoC) and the game's log.

1. **Install and start.** The launcher opens on the library. The first start
   unpacks the tools beside the page, which says it is preparing them;
   `adb logcat -s OpenRAC` says "unpacked OpenRAC's tools" and how long it
   took.
2. **Add the disc.** "Choose disc image" opens Android's picker; pick your
   own image of the disc (.iso, or a .chd made with `chdman createdvd`, which
   is unpacked into an .iso). Toasts count the copy in quarters; then the
   set-up runs by itself: the extractor, the levels (up to four at a time),
   the check of the levels. Leave the launcher during it (Home): a
   notification shows the step and its progress, and the set-up goes on;
   tapping it brings the launcher back. On a device with
   less free space than the image, the launcher says how much it needs.
3. **Play, on Mali.** Play opens the game in landscape: the menus over the
   title world, then a level. Check the picture against the desktop's
   (lighting, the sky, the 2D layer, text), the frame rate and the sound.
4. **Play, on Adreno.** The same, with the system's driver.
5. **Controls.** Touch the screen: the on-screen pad shows (the stick, d-pad,
   face buttons, shoulders, Select, Start; no right stick) and moves Ratchet;
   a thumb anywhere in the lower left brings the stick to it, and a thumb slid
   from one button to the next presses the next. Pushed to a diagonal, the
   stick runs, not walks. A finger on the right of the screen, away from the
   buttons, is an invisible right stick centred where it landed: pushed
   away from there it turns the camera, and keeps turning it while held
   out, as a real stick does; lifted, the camera stops. Connect a Bluetooth or USB controller
   (a DualShock 4 or DualSense too) and press a button: the pad hides and the
   controller plays; its stick pushed to a diagonal runs as well.
   In the drawer, Touch controls: Button size changes the controls at once;
   "Move the controls" leaves the game paused with a bar in the middle, a
   finger drags a group (the d-pad, the face buttons, a shoulder...), and
   near another group's line (or the middle, or its own place) it snaps into
   line with an accent line across the screen, while a little further off it
   goes where the finger takes it; Done keeps it there after the game is
   started again, "Put them back" and the bar's Reset bring every control
   home.
6. **Suspend and resume.** Switch to another app and back, and lock and
   unlock the screen, during a level: the game comes back where it was, with
   no fingers left held.
7. **Play again.** Quit the game (its menu, or Back), then Play again: it
   starts afresh. A crash leaves the launcher open with the crash report on
   the game's page.
8. **A driver package, on Adreno.** Settings, GPU driver, "Add driver
   (.zip)": pick a Mesa Turnip package (for example from
   K11MCH1/AdrenoToolsDrivers releases, made for Android emulators). It is
   listed with its name, version and vendor; choose it and play. The
   game's log says `GPU driver: the player's package, Turnip Adreno (TM) ...`
   with the driver's version. The picture still comes from OpenGL ES and the
   system's driver: the Vulkan renderer that draws on the package is the
   next change. Choose "System driver" and play again: the log says
   so. A zip without a `meta.json` is refused with a message.
9. **A driver package, on Mali.** The section says custom drivers require an
   Adreno GPU, and offers none; the game's log says why the system's
   driver is used.
10. **ANGLE.** In Developer options, "ANGLE preferences", choose ANGLE for
    OpenRAC Launcher and play: the game draws through ANGLE (its log names
    the renderer).
11. **Updating the launcher.** With a release newer than the installed
    launcher (both signed with the launcher's Android key), start the
    launcher: it offers the update; Update downloads it, the first time
    sends you to allow it to install apps, and back in the launcher
    Android's installer asks to confirm, with no second download. The launcher restarts updated, with
    the games set up as they were.
12. **Mods and Enhanced graphics.** The Mods tab shows one page for the
    game picked on Games, named at its top right: Gameplay
    mods with Enhanced graphics first, Add a mod (a link, or a .zip from the
    picker), the packs. Switch Enhanced graphics on: "Prepare the enhanced
    lighting" runs (a few minutes; the log names libopenrac-probe-bake.so's
    link). Play: the drawer's footer says Vulkan, and its Enhanced graphics
    card has its switches; the log says the probe bake was loaded. Add
    lock-strafe from its link (OpenRAC/rac1-mods): it installs and loads.
    Settings shows no Discord, Compact layout or Godot, and a switch is saved
    without a Save bar.
13. **The launcher with a controller.** With a controller connected, start
    the launcher and do not touch the screen: the d-pad and the left stick
    move an amber focus ring between every control (tabs, tiles, buttons,
    menus, settings, dialogs), A presses, B closes a menu or dialog and then
    returns to the games, LB and RB change tabs, and a held direction
    repeats. Add a disc, set a game up and play with the controller alone.
    The system's bars stay hidden, and a tap shows no blue flash.
14. **Your files.** Open the Files app: its sidebar lists OpenRAC with the
    free space, and inside it Saves and settings, Games, Disc images, GPU
    drivers and Screenshots. Settings' Open Files opens it there. Uninstall
    the app: Android asks whether to keep its data; keep it, install the
    same build again, and the games, saves and settings are back with no
    set-up. Updating from a build that kept its games under shared
    storage's OpenRAC folder moves them into the app on the first start.
15. **Touch controls and the look.** In a level, L1, L2, R1 and R2 sit below
    the health and bolt counters, and a short push on the right of the
    screen turns the camera at full speed for as long as it is held. On
    Games, picking another game changes the launcher's colour (rac2 light
    blue, rac3 yellow, rac4 red, Size Matters blue), and a page slides in
    over the last as the tabs change.
