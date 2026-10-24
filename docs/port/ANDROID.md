# Android

OpenRAC runs on Android phones, tablets and handhelds. You need:

- a 64-bit ARM device (arm64-v8a),
- Android 10 (API 29) or newer,
- a Mali or Adreno GPU with OpenGL ES 3.2.

There is no emulation here either. The APK contains the same translated game
code as the desktop build, compiled for ARM with the Android NDK, and the
game data comes from your own disc, read on the device.

This page explains how the Android version is put together and why. To build
it and test it yourself, see [port/android/README.md](../../port/android/README.md).

## The short version

- One APK holds both the launcher and the game.
- The launcher is the desktop launcher (Tauri 2 + Svelte), so it looks and
  works the same.
- The game runs as a separate Android activity, in its own process.
- You pick a disc image once; the launcher copies it into the app and sets
  the game up on the device, the same way the desktop launcher does. There
  is no folder to choose: everything lives in the app's own storage, which
  the Files app shows under OpenRAC.
- A controller works everywhere. Touch controls appear when you touch the
  screen.
- The launcher updates itself from GitHub releases.
- By default the game draws with OpenGL ES on the system's driver. There is
  also a Vulkan renderer, which can use a custom Adreno driver (Turnip).

## One APK, two parts

**The launcher** is the desktop launcher's interface and logic
(`launcher/core`), built with Tauri 2's Android support. Its Android project
lives in `launcher/src-tauri/gen/android`. Tauri generates it with
`tauri android init` and expects it to be committed. Because it is the same
code, the library, game pages, settings, saves and mods all behave as on the
desktop. It always plays from the copy of OpenRAC it carries inside the APK.

The few things only Android can do (open a file with the system picker,
import a disc or a driver package, start the game) go through a small Kotlin
plugin, `OpenracPlugin`. Where a desktop command does the same job, the
plugin's command has the same name and arguments.

**The game** is `GameActivity`, which is SDL3's `SDLActivity` loading the game
as a library (`libopenrac-rac1-pal.so`, whose `main` SDL calls as
`SDL_main`). It runs:

- **in its own process** (`:game`), because the game reserves 4 GB of address
  space and quits with `exit()`, which must not close the launcher too;
- **in its own task**, so the game and the launcher are two separate windows
  in Recents. The game keeps running while you look at the launcher, and the
  game's page then offers *Return to game* and, in its menu, *Stop the game*.

### Why Tauri and not a separate Android app

One interface for every platform. The launcher's logic is plain Rust and
compiles for Android unchanged, and Tauri 2 supports Android officially. A
native Kotlin launcher would be smaller, but it would be a second launcher
to keep in sync with the first.

## How the game is built and runs

**Building.** `cmake --preset android-arm64` cross-compiles the port with the
NDK. hostgen still runs on the build machine, since it only writes C. On
Android the game is a shared library rather than a program, and SDL3 becomes
`libSDL3.so`. `tools/build_android.py` builds both and copies them into the
launcher's `jniLibs/arm64-v8a` before Gradle packages the APK.

**Graphics.** The renderer is written for desktop OpenGL 4.1. On Android it
asks for an OpenGL ES 3.2 context instead:

- Almost everything it uses exists in ES 3.2. The two functions that don't
  (`glClearDepth`, `glPolygonMode`) are replaced in `renderer/gl.cpp`.
- Shaders are written once, in GLSL 4.10, and turned into GLSL ES 3.20 when
  the context is ES (`es_source` in `renderer/shader.cpp`).
- Two optional driver extensions are used when present. Without
  `GL_NV_shader_noperspective_interpolation` some values are interpolated
  with perspective; without `GL_EXT_clip_cull_distance` the widescreen crop
  of the 2D layer is skipped.
- The same ES path runs on a desktop with `OPENRAC_GL_API=es`. That is how
  CI and the tests check it without a phone, using Mesa's ES driver.

**Input.** Controllers work as on the desktop: Bluetooth, USB, and the
built-in pads of handhelds. For touch there is an on-screen pad
(`platform/touch_pad.h`). It is laid out from the screen edges, stays inside
the safe area, and its left stick moves to wherever your thumb lands. Its
right stick is not drawn: a finger that lands on the right of the screen,
away from the buttons, holds an invisible stick centred where it landed,
and the camera turns for as long as it is pushed away from there, as with
a real stick (`kLookReach`); pushed further, the centre follows the finger.
The shoulder buttons sit a little below the top edge, clear of the game's
health and bolt counters; a test checks that no two controls overlap at the
usual screen shapes and sizes. Its size and where each part sits are the
player's (the drawer's Touch controls). A round stick, a gamepad's or the
touch pad's, is stretched to the DualShock 2's square (`platform/pad.h`), so
a stick pushed to a diagonal runs. The layout is plain data with desktop
tests, and it is drawn by a single shader (`game/common/touch_overlay.cpp`).

**Files.** The launcher tells the game where everything is, on the command
line and in the environment (`--data`, `--levels`, `HOME`, `XDG_DATA_HOME`,
all inside the app's own storage). So `main.cpp` needs no Android-specific
paths.

**Where your files are.** The app keeps everything in its own storage
(`files/`): the games (`files/games`, the launcher's install folder),
saves and settings (`files/share/openrac`), disc images, GPU drivers and
screenshots. A documents provider (`FilesProvider.kt`, authority
`<package>.files`) shows them in the Files app's sidebar as OpenRAC, with
the free space, the way Dolphin and Eden do, and Settings has an Open
Files button. The app sets `hasFragileUserData`, so uninstalling asks
whether to keep this data; kept, it is all back when the same app (signed
with the same key) is installed again. An install from before this layout,
under shared storage's `OpenRAC` or the app's data folder, is moved into
`files/games` on the first start (`settle_install_dir`), and the first-run
folder screen is skipped.

**Older Android versions.** Android's C library (bionic) only has
`backtrace()` from Android 13 (API 33). For Android 10 to 12 a small
replacement built on `_Unwind_Backtrace` is compiled in (`port/android/compat`).
`memfd_create` is called through `syscall` below API 30.

## Setting up a game on the device

**Choosing the disc.** You pick the `.iso` with Android's file picker. The
picker hands over an open file rather than a path, and the game needs to read
the disc by sector for as long as it runs, so the image is copied once into
the app's storage (`files/discs/`). Free space is checked first, and the
copy shows its progress. A `.chd` image (from `chdman createdvd`) is
unpacked into a plain `.iso` instead of copied (`launcher/core/src/chd.rs`).

**Running the setup.** After that, the launcher runs exactly what the
desktop launcher runs: `tools/extractor.py`, then `editor/extract.py port`
for the levels, then the game's `--check-levels`. Their output streams into
the launcher.

These steps need Python and some programs, and Android only lets an app run
programs from its native library folder. So the APK carries:

- **Python.** CPython for Android as packaged by
  [Chaquopy](https://github.com/chaquo/chaquopy) (MIT; `libpython3.13.so`
  with its modules and standard library, a pinned version from Maven
  Central), plus `libopenrac-python.so`, a small `python3` program around it
  (`port/android/python_main.c`).
- **The game without a window** (`libopenrac-<id>-cli.so`), the game built a
  second time as a program, for `--check-levels`.
- **Enhanced graphics' probe baker** (`libopenrac-probe-bake.so`), with
  Embree built in by the NDK (the release and SHA-256 `tools/build_port.py`
  pins), which bakes the bounced light from the player's own levels on the
  device, as the desktops do (`port/enhanced/tools/probes/bake_all.py`).
  Playing with Enhanced graphics on starts the game with the Vulkan renderer
  and those bakes (`--probes`), the environment of the launcher's Play action
  passed on to the game's activity.
- **A copy of OpenRAC** (`tools/bundle_standalone.py --android`), stored as
  `assets/openrac.zip`. When the APK changes, `OpenracSetup.kt` unpacks it
  into `files/openrac` on a background thread (into `files/openrac.new`
  first, which then replaces the old copy). It then links `python3` and the
  game's programs to the libraries above, so the launcher finds them where a
  desktop copy keeps them. The launcher waits for this and explains the
  wait if it takes more than a second.

**Made for a phone.** The setup tries to be light on storage and CPU:

- The image is copied by the kernel (`copy_file_range` or `sendfile`).
- The extractor writes out only `SYSTEM.CNF` and the boot executable
  (`--boot-only`). It still reads and checks every file; the port reads
  nothing else from the disc.
- Android's Python has no process pools (no `sem_open`), so
  `editor/extract.py` runs up to four levels at once in separate Python
  processes (`SubprocessPool`).
- Preparing the enhanced lighting bakes several levels at once, and on
  Android each baker may use every CPU thread, the largest levels first,
  so that the last level to finish does not bake alone on one core
  (`bake_all.py`). The probes are the same as with any thread count.

**Leaving the app during setup.** You can switch away while it works. A
foreground service (`SetupService.kt`) keeps it running and shows the
current step and its progress in a notification
(`backgroundStatus` in `launcher/src/lib/progress.ts`).

Playing is the one action that doesn't run a program: the launcher starts
the game's activity with the same command line a desktop would use. Nothing
leaves the device.

## The launcher's screen and controls

The launcher runs full screen. The system bars are hidden (swipe to show
them for a moment) and the page stays clear of a camera cut-out
(`MainActivity.kt`).

You don't need a touch screen (`launcher/src/lib/pad.ts`; the desktop does
the same through the Gamepad API):

| Button | Does |
|---|---|
| D-pad or left stick | moves the focus to the nearest control in that direction |
| A | presses it |
| B | closes a menu or dialog, then goes back to the games |
| LB / RB | switch tabs |
| Android's Back | same as B; from the games list it leaves the launcher (a running setup keeps going) |

On Android the launcher has a navigation rail (Games, Mods, Settings) and
its own arrangement of the pages:

- Games shows a compact card per game. The game picked last is ringed,
  and the whole launcher wears its colour: Ratchet & Clank orange, Going
  Commando light blue, Up Your Arsenal yellow, Deadlocked red, Size
  Matters blue (`launcher/src/lib/themes.ts`).
- Mods edits the game picked on Games, named at the top right.
- A game's page keeps Play and the saves at hand; Differences from the
  original and Help (logs and crash reports) are folded, Help at the foot.
- A page slides in as the last slides away and fades, from below for a tab
  lower on the rail and from above for one higher (a view transition); its
  sections come with it. With the system's reduced motion they simply
  change.

The game is full screen too (`port/platform/window.cpp`). Its on-screen pad
appears only after you touch the screen, and hides again once you use a
controller or leave the screen alone for ten seconds.

## The in-game drawer

On a desktop, the game has menus along the top of the window (View,
Graphics, Sound, Developer). On Android they live in a drawer, like in an
emulator. It slides in from the left over the paused game when you press:

- the pad's options button (between Select and Start),
- Back, or
- a controller's Guide button.

Its rows are grouped on cards like the launcher's settings, big enough for a
thumb, and a controller can move through them (left and right change a
value, B closes). It wears the launcher's theme and the game's accent, which
the launcher hands it at Play (`OPENRAC_UI_COLORS`), and moves as the
launcher does: it slides out with the launcher's easing, presses, switches
and the focus ease in and out, and a flicked list glides on. It contains:

- **Graphics:** picture shape (4:3, 16:10, 16:9, 21:9, 32:9 or the whole
  screen), HUD position, render scale, anti-aliasing, texture filtering,
  frame rate, VSync and the FPS counter.
- **Enhanced graphics**, when it is on in the launcher (Mods): its switch,
  modern shadows, bounced light and its strength, ambient occlusion and its
  quality. When this run cannot have it (it is off, the game fell back to
  OpenGL ES, or the level's lighting is not prepared yet), the card says so.
- **Sound and game:** volume and skipping cutscenes.
- **Touch controls:** the buttons' size, the camera's speed, *Move the
  controls* (drag them about over the paused game, with a bar to resize them,
  reset them and finish; a group dragged near another's line, the middle, the
  mirror of another across it or its own place snaps onto it, the line shown)
  and *Put them back*.
- **Developer tools**, folded away: the debug overlay, pause, a single
  frame step, game speed, a screenshot (saved to the app's
  `files/screenshots`), and the level select in developer mode.
- **Quit.**

A change is saved and applied right away. Back, Resume, B or a touch outside
the drawer closes it (`port/game/common/debug_overlay.cpp`; set
`OPENRAC_DRAWER=1` to see it on a desktop). The debug overlay shows no
keyboard shortcuts on Android: you scroll it with a finger, and a
controller can move through it while the game is paused.

The game asks Android for the display's fastest refresh rate while it is in
front (`GameActivity.java`). Otherwise a 120 Hz panel can drop to 60 Hz after
the game has been capped at 60 for a while, and then stay there.

## Updating the launcher

On the desktop, Tauri's updater replaces the program in place. Android
doesn't allow that, so the Android launcher has its own updater, using the
same release manifest (`latest.json`, whose `android-aarch64` entry is the
APK) and the same signing key:

1. It downloads the APK into its cache.
2. It checks the APK's signature against the public key in `tauri.conf.json`
   (`launcher/core/src/update.rs`).
3. It hands the APK to Android's installer (`PackageInstaller`). Android
   asks you to confirm, and checks that the new APK is signed with the same
   Android key as the installed one.

The first time, Android asks you to allow the launcher to install apps. If
you have left the launcher by the time the download is done, the
confirmation waits in a notification and appears when you come back
(`UpdateReceiver.kt`).

An update is only offered when the build carries the launcher's own Android
key, so you are never offered an update that Android would refuse. Releases
are signed with that key ([RELEASING.md](../../launcher/docs/RELEASING.md)).
A build made elsewhere has a different key: it is not updated, and it is not
offered as an update.

## GPU drivers, including your own

Emulators on Android (Skyline, Yuzu and its successors, Eden, Citron,
Winlator Bionic) use these kinds of drivers. Here is what each one means for
OpenRAC:

| Driver | GPUs | How you get it | In OpenRAC |
|---|---|---|---|
| The system's OpenGL ES driver | Mali, Adreno | every app gets it | works; the default |
| An updated system driver | the vendor's | Android's "updatable graphics drivers", installed by the vendor or phone maker | works with no extra code |
| ANGLE (OpenGL ES on top of Vulkan) | both | Developer options, "ANGLE preferences", per app | works with no extra code; useful on a phone whose GLES driver is broken |
| A custom Vulkan driver package (Mesa Turnip, newer Qualcomm drivers) | Adreno | [libadrenotools](https://github.com/bylaws/libadrenotools) (BSD-2-Clause) loads the driver from the app's storage; packages are zips with a `meta.json` (`name`, `author`, `vendor`, `driverVersion`, `minApi`, `libraryName`), the format Eden and the driver builders use | Vulkan only: needs the Vulkan renderer |

The last row is what players usually mean by "custom drivers", and it only
works with Vulkan. libadrenotools hooks the Vulkan loader; there is nothing
like it for OpenGL ES, and Android's ANGLE can't be pointed at another
Vulkan driver without patching it. Mali has no replacement drivers worth
loading (Eden only offers them on Qualcomm devices, where `/dev/kgsl-3d0`
exists), so on Mali the choices are the system driver, an updated one, or
ANGLE.

So the custom-driver path is built in three parts:

1. **Driver packages** (`OpenracPlugin.kt`, "GPU driver" in Settings). You
   import a zip. The launcher checks its `meta.json` (the `libraryName` must
   be in the zip, and `minApi` must not be above the device's), unpacks it
   into `files/gpu_drivers/<id>/`, and lets you list, choose and remove
   packages. The chosen one is passed to the game in
   `OPENRAC_GPU_DRIVER_DIR` and `OPENRAC_GPU_DRIVER_LIBRARY`.
2. **Loading** (`port/android/gpu_driver.cpp`). On Adreno, the game loads the
   chosen package with `adrenotools_open_libvulkan` and uses its
   `vkGetInstanceProcAddr`. What happened (the driver's name and version, or
   why it was refused) is logged and shown in the launcher. Anywhere else, or
   if loading fails, the system driver is used.
3. **The Vulkan renderer** (`port/renderer/vulkan/`). The renderers
   only use a small set of OpenGL calls (`renderer/gl.h`,
   `renderer/world/gl_world.h`), and this layer carries out those calls on
   Vulkan 1.3. OpenGL objects become Vulkan objects, shaders are compiled
   with glslang, pipelines are cached by the state they bake in, and buffers
   stream through a per-frame ring until they stop changing. The renderers,
   their shaders, the touch pad and the drawer run on it unchanged. You
   choose it with "Renderer" in Settings (the launcher passes
   `OPENRAC_GL_API=vulkan`). If Vulkan 1.3 can't start, the game falls back
   to OpenGL ES by itself.

**Decided:** the Vulkan renderer stays a translation layer for now, and gets
measured before any part of it is rewritten as native Vulkan. `--debug-dump`
writes its per-frame numbers (`vk_*`: draws, pipelines created, ring
kilobytes, flushes, and CPU time spent drawing, presenting and waiting for
the GPU). The most expensive paths are the ones to rewrite first. Shipping a
patched ANGLE was the alternative and was not chosen. The drawing tests also
run on Vulkan (`*_vulkan`, with the validation layer, on Mesa's lavapipe in
CI).

**Why Vulkan and a custom driver gain less here than in an emulator.** A
Switch or PS2 emulator is held back by the GPU: it draws the console's
picture at several times its size through large shaders, and a faster
driver shows at once. The port draws a PS2 game's scenes natively, which a
current Adreno draws with time to spare, so its frame waits on the
processor: the game's own code, and the renderer's calls, which the Vulkan
layer translates from OpenGL's shape. A better driver changes neither, and
when both renderers reach the screen's rate there is nothing left to gain
in frames per second. What helps there is processor time: the Android
build is linked with ThinLTO (`OPENRAC_PORT_LTO`, the `android-arm64`
preset), which both renderers gain from. Which side a device is on is in
`OPENRAC_FRAME_COST`'s lines (`scene`, `submit`, `gpu`, `show`) and
`--debug-dump`'s `vk_*` numbers; those are what decide what of the layer
is rewritten next.

## Building and CI

`tools/build_android.py` builds everything: Python, the port with the NDK,
the libraries and SDL's Java code into the launcher's Android project, the
carried copy of OpenRAC, and finally the APK with
`yarn tauri android build`.

In CI (`.github/workflows/launcher.yml`), Android is a row in both build
matrices, next to the desktop systems. It runs on `ubuntu-latest`, which
comes with the Android SDK and NDK:

- `game (android)` runs `build_android.py --no-apk` and keeps what it put into
  the Android project;
- `android` builds the APK from that and keeps it with the other installers
  (and, on a `launcher-v*` tag, adds it to the draft release).

The desktop port tests (`checks.yml`) include the OpenGL ES variants on Mesa.

The APK is signed with the launcher's Android key when the repository has
the `ANDROID_KEYSTORE` secrets. Without them, CI signs it with a throwaway
debug key, so each build has to be installed fresh rather than over the
previous one. `python3 tools/android_keystore.py` makes the key once and
prints the secrets to add ([RELEASING.md](../../launcher/docs/RELEASING.md)).

A fork's CI builds an app of its own, so that it installs beside the
project's: outside `OpenRAC/OpenRAC` the APK's package is
`dev.openrac.launcher.fork_<owner>`, its name on the home screen "OpenRAC
Fork" and its file `OpenRAC.Launcher.Fork_<version>_android-arm64-debug.apk`
(`OPENRAC_ANDROID_APPLICATION_ID` and `OPENRAC_ANDROID_APP_LABEL`, read by
`app/build.gradle.kts`). Its data is its own too. The project's own builds,
and pull requests to it, keep `dev.openrac.launcher` and "OpenRAC Launcher".

## What still needs a real device

CI has no phone and no disc, so these are tested by hand (step by step in
[port/android/README.md](../../port/android/README.md)):

- the launcher's screens;
- importing a disc;
- the game's first frames on Mali and on Adreno;
- a controller and the touch pad;
- the in-game drawer;
- suspending and resuming;
- the Vulkan renderer, and that it falls back when it can't start;
- on Adreno, importing a Turnip package and seeing in the log that it
  loaded and draws.
