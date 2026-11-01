# Status Monitor and ReverseNX-RT

OpenRAC-NX publishes NX-FPS telemetry directly from its homebrew process when SaltyNX's shared-memory service is available. No SaltyNX injection into the NRO is required. Without SaltyNX the game and its physical handheld/docked profiles still work.

## What is measured

- FPS comes from elapsed system ticks between completed buffer swaps, including interpolated frames. It is presentation throughput, not PS2 simulation ticks or a promised physical scanout rate.
- The integer FPS is measured over an elapsed window of at least one second. The ten-frame tick ring supplies Status Monitor's moving average.
- Render resolution comes from the completed game's actual render target, not the saved preset.
- The viewport list reports the window drawable dimensions. These may differ from the internal render size; the picture can also be pillarboxed.
- The launcher reports its own 1280x720 canvas and actual window drawable when it presents. Its mostly static menu redraws on demand, so its FPS is not a gameplay benchmark.
- CPU/GPU utilization, clocks, temperature and memory remain the overlay/sysmodules' measurements. The game does not invent these values.

The writer uses OpenGL/EGL API ID 2, the common NX-FPS fields, and a 310-byte current-format allocation. It can reuse an existing older block without writing past its 174-byte common prefix. Writes preserve control fields used by other overlays. No GPU counters unsupported by this OpenGL renderer are advertised.

## Profiles and frame caps

Display has common Render resolution, Handheld resolution and Docked resolution rows. Each profile initially inherits the common resolution. Fixed profiles offer the same 480p-and-up presets, including 4:3 options. Auto is also available. The chosen profile supplies both image aspect and target dimensions, with existing framebuffer recreation handling changes.

By default profile selection follows the physical operation mode. With SaltyNX and ReverseNX-RT available, disabling system control in ReverseNX-RT and selecting handheld/docked chooses the corresponding OpenRAC-NX profile. Selecting system control restores the physical mode. This does not alter clocks, HDMI mode, display refresh or the panel. ReverseNX's DDR overrides are not advertised; choose actual profile resolutions inside the launcher. Overlay profile forcing starts in system control on attachment to avoid inheriting another game's forced state.

Frame rate and Frame cap already exist in the in-game Video options. The launcher's Frame cap now also activates capped mode immediately when adjusted. Limits remain in the game's own settings; FPSLocker controls are not implemented.

## Console test

1. Use the usual full-application homebrew launch, with SaltyNX and Status Monitor installed and running. The NRO does not bundle/install those tools.
2. Start gameplay and open Status Monitor. Check that FPS varies with actual load rather than sticking to the cap. Allow a second for the integer measurement.
3. Change common/profile resolution in the launcher and start gameplay. The overlay render resolution should match the actual game target; drawable output may remain 1280x720.
4. Set distinguishable handheld/docked resolutions and test real docking. With ReverseNX-RT, test forced handheld, forced docked, then system control. Check image aspect and HUD placement too.
5. Compare 30/50/60 caps in launcher/in-game options. A cap is a ceiling and cannot guarantee the game reaches that rate.
6. Test without SaltyNX, repeated launches, sleep/resume, and reopening the overlay.

Compatibility is unverified on console. Startup now checks named-port session capacity before SDL/audio threads start. If permitted it raises an exhausted session limit; otherwise it temporarily releases and restores the startup service-manager session around the connection. Later retries do not release that session. The service is retried every five seconds if absent or busy. Connection results are appended to `/switch/openrac/overlay-status.log`. A build passing cannot prove overlay/loader compatibility.

Protocol references: [SaltyNX](https://github.com/masagrator/SaltyNX/blob/master/saltysd_core/source/NX-FPS-Common.hpp), [SaltyNX service](https://github.com/masagrator/SaltyNX/blob/master/saltysd_proc/source/service.c), [ReverseNX wire format](https://github.com/masagrator/SaltyNX/blob/master/saltysd_core/source/ReverseNX.cpp), [Status Monitor reader](https://github.com/masagrator/Status-Monitor-Overlay/blob/master/source/Utils.hpp).
