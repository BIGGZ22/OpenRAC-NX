# OpenRAC-NX

A native Nintendo Switch port of **Ratchet & Clank (2002)**, based on [OpenRAC](https://github.com/OpenRAC/OpenRAC).

**Early alpha. An overclock and full application mode are required for playable performance.** The app does not apply an overclock automatically. Performance varies by scene, and a complete playthrough has not been verified.

[Download releases](https://github.com/BIGGZ22/OpenRAC-NX/releases) · [Build guide](port/platform/switch/README.md) · [Credits](CREDITS.md)

## Requirements

- A homebrew-capable Nintendo Switch.
- Your own **Ratchet & Clank PAL Europe/Australia v2.00 ISO (`SCES_509.16`)**.
- At least **2 GiB of free SD space** in addition to the ISO.

Other games and disc revisions are currently unsupported. No game files are included.

## Installation

1. Download `openrac-rac1-pal.nro` from the releases page.
2. Put the NRO and your supported ISO in `SD:/switch/openrac/`. Keep only one ISO in that folder.
3. Launch in **full application mode**, then let first-launch setup prepare the game.
4. Select **Play** and press **A**.

```text
SD:/switch/openrac/
  openrac-rac1-pal.nro
  Ratchet.iso
```

**Keep the ISO in place after setup.** If setup is interrupted, use the **Setup** page to resume or repair it.

## Features

- On-console ISO setup with resumable preparation.
- Controller and touch-operated launcher.
- Saved video, audio, gameplay and controller settings.
- Button rebinding, stick deadzones and camera sensitivity options.
- Third-person lock-strafe: hold **ZL** to strafe while aiming with the right stick.
- Memory-card saves stored on the SD card.

### Upcoming: v0.2.0 alpha

The next version adds resolution presets and handheld/docked profiles, optional Status Monitor FPS/RES reporting, a lock-strafe toggle, experimental FOV controls and particle-rendering improvements. It also updates the menu and settings handoff.

These changes are being prepared for GitHub tag `v0.2.0-alpha`.

## Controls

In the launcher, use **L/R** to change pages, the **D-pad** to select, and **A** or **Left/Right** to adjust. **B** returns to Play; **+** exits. Touch navigation is supported.

| Switch input | In-game action / PS2 equivalent |
| --- | --- |
| Left / right stick | Movement / camera |
| B / A / X / Y | Cross / Circle / Triangle / Square |
| L / R / ZR | L1 / R1 / R2 |
| Hold ZL | Lock-strafe |
| + / − | Start / Select |
| Stick clicks | L3 / R3 |
| Hold L + R + Minus | Exit the game |

Customize gameplay bindings on the **Controller** page.

## Updating and saves

Back up your saves, then **replace only the NRO**. Keep your ISO, prepared data and settings.

- **Saves:** `SD:/switch/openrac/memcard/SCES_509.16/`
- **Settings:** `SD:/switch/openrac/settings/rac1-pal.ini`

Save through the game's menu and wait for it to finish before exiting.

## Troubleshooting

- **Missing files or interrupted setup:** open the Setup page and select prepare/resume.
- **Low performance:** check that your overclock is active and you launched in full application mode.
- **Reporting a bug:** include the build version, planet/scene and steps to reproduce it.
- **Game log:** `SD:/switch/openrac/data/openrac-rac1-pal.log`
- **Setup log:** `SD:/switch/openrac/data/setup.log`

## Building from source

Use the `codex/switch-port` branch. See the [Switch build guide](port/platform/switch/README.md) for devkitPro dependencies and build commands.

## Credits and license

Built on [OpenRAC](https://github.com/OpenRAC/OpenRAC) and its contributing decompilation projects. Lock-strafe is adapted from [OpenRAC/rac1-mods](https://github.com/OpenRAC/rac1-mods). Thanks to the devkitPro/libnx, SDL and Mesa contributors.

See [CREDITS.md](CREDITS.md), [LICENSE.md](LICENSE.md) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

An unofficial fan project, independently maintained and not endorsed by Sony Interactive Entertainment, Insomniac Games, Nintendo or the OpenRAC maintainers.
