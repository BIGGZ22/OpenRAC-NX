# The port on Android

How to build the APK and test it on a device. The design and the reasons for
it are in [docs/port/ANDROID.md](../../docs/port/ANDROID.md).

| Path | What it is |
|---|---|
| `compat/` | what bionic lacks below API 33 (`backtrace`), first on an Android build's include path |
| `gpu_driver.cpp` | the player's Vulkan driver package, loaded through libadrenotools on Adreno (platform/gpu_driver.h) |
| `python_main.c` | `libopenrac-python.so`, the `python3` the launcher runs its tools with |
| `java/dev/openrac/game/GameActivity.java` | the game's activity: SDL's, around `libopenrac-<id>.so` |
| `../cmake/Android.cmake` | what an Android build adds; the `android-arm64` preset is in `../CMakePresets.json` |

The launcher's side is `launcher/src-tauri/src/android.rs` and the Kotlin in
`launcher/src-tauri/gen/android` (`OpenracSetup.kt`, `OpenracPlugin.kt`,
`SetupService.kt`, `UpdateReceiver.kt`, `FilesProvider.kt`).

## Building

You need the Android SDK (`ANDROID_HOME`) with NDK r27 or newer
(`ANDROID_NDK_HOME`), JDK 17+, Rust with the `aarch64-linux-android` target,
Node with Corepack, CMake, Ninja and a host Clang (for hostgen). From the
repository's root:

```sh
rustup target add aarch64-linux-android
python3 tools/build_android.py               # rac1-pal and rac1-ntsc, a debug APK
python3 tools/build_android.py rac1-pal      # one game
python3 tools/build_android.py --no-apk      # all but Gradle: then open launcher/src-tauri/gen/android in Android Studio
python3 tools/build_android.py --release     # an unsigned release APK, to sign with your key
```

The APK lands in
`launcher/src-tauri/gen/android/app/build/outputs/apk/universal/debug/`. CI
builds the same debug APK beside the desktop installers (`launcher.yml`,
artifact `openrac-launcher-android`); a `launcher-v*` tag adds it to the draft
release.

The port alone:

```sh
cd port
cmake --preset android-arm64 && cmake --build build/android-arm64 --target openrac-rac1-pal openrac-rac1-pal-cli
```

`-DOPENRAC_ANDROID_ADRENOTOOLS=OFF` leaves libadrenotools out (always the
system's driver).

## Installing and logs

```sh
adb install -r app-universal-debug.apk
adb logcat -s OpenRAC SDL SDL/APP openrac
adb shell run-as dev.openrac.launcher ls files   # the app's files, on a debug build
```

CI sets each APK's version code from its run (`OPENRAC_ANDROID_VERSION_CODE`),
so that a home screen picks up a changed icon. A local build therefore installs
over a CI one only after `adb uninstall dev.openrac.launcher`, or with the
variable set higher.

The game writes its log in its data folder
(`files/games/active/<game>/data/`), and the launcher shows it with the crash
reports on the game's page.

## Testing on a device

CI has no phone and no disc, so these steps are done by hand. When you report,
give the device, its GPU (Settings, GPU driver shows the model and SoC) and the
game's log.

1. **Install and start.** The launcher opens on the library and says it is
   preparing its tools; logcat says "unpacked OpenRAC's tools" and how long it
   took.
2. **Add the disc.** "Choose disc image" opens the picker. Pick your image
   (.iso, or a .chd from `chdman createdvd`).
   - Toasts count the copy in quarters, then the set-up runs: the extractor,
     the levels (four at a time), the level check.
   - Press Home meanwhile: a notification shows the step and goes on, and
     tapping it brings the launcher back.
   - With less free space than the image, the launcher says how much it needs.
3. **Play on Mali, then on Adreno** (the system's driver). The game opens in
   landscape on the title world. Compare the picture with the desktop's
   (lighting, sky, 2D layer, text), and check the frame rate and sound.
4. **Touch controls.**
   - Touching the screen shows the pad: stick, d-pad, face buttons, shoulders,
     Select and Start, but no right stick. L1, L2, R1 and R2 sit below the
     health and bolt counters.
   - A thumb anywhere in the lower left brings the stick to it, and sliding
     from one button to the next presses the next. A diagonal runs.
   - A finger on the right, off the buttons, turns the camera: a short push
     turns it at full speed for as long as it is held, and lifting stops it.
   - In the drawer's Touch controls, Button size changes the pad at once.
     *Move the controls* pauses the game with a bar in the middle. Drag a
     group (d-pad, face buttons, a shoulder...): near another group's line,
     the middle or its own place it snaps, with an accent line across the
     screen, and a little further off it follows the finger. Done keeps it
     after a restart, and the bar's Reset brings it all home.
5. **A controller.** Connect one (Bluetooth or USB, a DualShock 4 or DualSense
   too) and press a button: the pad hides and the controller plays. A diagonal
   runs.
6. **Suspend and resume.** Switch apps, and lock and unlock the screen, during
   a level: the game comes back where it was, with no fingers left held.
7. **Play again.** Quit (the game's menu, or Back) and Play again: it starts
   afresh. A crash leaves the launcher open with the report on the game's page.
8. **A driver package, on Adreno.** Settings, GPU driver, "Add driver (.zip)":
   pick a Mesa Turnip package (for example from K11MCH1/AdrenoToolsDrivers).
   It is listed with name, version and vendor. Choose it and play: the log
   says `GPU driver: the player's package, Turnip Adreno (TM) ...` with its
   version. Choose "System driver" and the log says
   so. A zip without `meta.json` is refused with a message.
9. **A driver package, on Mali.** The section says custom drivers need an
   Adreno GPU and offers none; the log says why the system's driver is used.
10. **ANGLE.** Developer options, "ANGLE preferences": choose ANGLE for the
    launcher and play; the log names ANGLE as the renderer.
11. **Updating.** With a newer release signed with the launcher's Android key,
    start the launcher: it offers the update by itself. Update downloads it,
    sends you once to allow installing apps, and Android's installer asks to
    confirm with no second download. The launcher restarts updated, with the
    games as they were.
12. **Mods and Enhanced graphics.** Mods shows one page for the game picked on
    Games: Gameplay mods with Enhanced graphics first, Add a mod (a link or a
    .zip), then the packs.
    - Switch Enhanced graphics on: "Prepare the enhanced lighting" runs for a
      few minutes (the log names `libopenrac-probe-bake.so`).
    - Play: the drawer's footer says Vulkan, its Enhanced graphics card has
      its switches, and the log says the probe bake loaded.
    - Add lock-strafe from its link (OpenRAC/rac1-mods): it installs and loads.
    - Settings has no Discord, Compact layout or Godot, and saves a switch
      without a Save bar.
13. **The launcher with a controller.** Without touching the screen: the d-pad
    and left stick move an amber focus ring over every control, A presses, B
    closes a menu or dialog and then returns to the games, LB and RB change
    tabs, and a held direction repeats. Add a disc, set it up and play with the
    controller alone. The system bars stay hidden and a tap shows no blue
    flash.
14. **The launcher's look.** Picking another game on Games changes the
    launcher's colour (rac2 light blue, rac3 yellow, rac4 red, Size Matters
    blue), pages slide in as the tabs change, and the drawer in the game wears
    the theme picked in Settings.
15. **Your files.** The Files app's sidebar lists OpenRAC with the free space,
    holding Saves and settings, Games, Disc images, GPU drivers and
    Screenshots; Settings' Open Files opens it there. Uninstall and keep the
    data, reinstall the same build: games, saves and settings are back with no
    set-up. A build that kept its games in shared storage's OpenRAC folder has
    them moved into the app at the first start.
