<h1 align="center">
  <a href="https://openrac.dev/">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="docs/assets/openrac-logo-dark.svg">
      <img src="docs/assets/openrac-logo-light.svg" alt="OpenRAC" width="440">
    </picture>
  </a>
</h1>

<p align="center">
  <strong>Ratchet &amp; Clank on PC, natively.</strong><br>
  The PlayStation 2 games, decompiled back into C and rebuilt for Windows, macOS, Linux and Android.
</p>

<p align="center">
  <a href="https://github.com/OpenRAC/OpenRAC/releases/latest"><img src="https://img.shields.io/github/v/release/OpenRAC/OpenRAC?filter=launcher-v*&label=download&color=e8a317" alt="Download the latest launcher"></a>
  <a href="https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml"><img src="https://github.com/OpenRAC/OpenRAC/actions/workflows/checks.yml/badge.svg" alt="Checks"></a>
  <a href="https://openrac.dev/"><img src="https://img.shields.io/badge/website-openrac.dev-e8a317" alt="openrac.dev"></a>
  <a href="https://discord.gg/Sfd2B54PDG"><img src="https://img.shields.io/badge/Discord-join%20us-5865F2?logo=discord&logoColor=white" alt="Join us on Discord"></a>
</p>

<p align="center">
  <a href="#play">Play</a> ·
  <a href="https://openrac.dev/#progress">Progress</a> ·
  <a href="#make-mods">Make mods</a> ·
  <a href="#help-out">Help out</a> ·
  <a href="docs/README.md">Docs</a>
</p>

<p align="center">
  <img src="docs/assets/launcher.jpg" alt="The OpenRAC launcher: the Ratchet &amp; Clank games beside the page of the one picked, with its saves and the way to play" width="860">
</p>

OpenRAC is a fan project that brings the PlayStation 2 Ratchet & Clank games to
PC: Ratchet & Clank, Going Commando, Up Your Arsenal and Deadlocked, with Size
Matters and Secret Agent Clank on the way. Each game is decompiled function by
function until the C builds into an exact copy of the original program. That C
then runs on your computer as a real PC game. There is no emulator.

OpenRAC does not include any game data. The models, textures, sound and levels
come from your own disc.

## Play

1. [Download the launcher](https://github.com/OpenRAC/OpenRAC/releases/latest)
   for your system and install it.
2. Open it and pick the folder with your PlayStation 2 disc images (`.iso`).
   The launcher finds your games and sets them up.
3. Press **Play**.

There is nothing else to install. You only need an image of your own disc:
OpenRAC never downloads a game. The release page says which file to pick and
what does not work yet.

Ratchet & Clank (PAL, Europe) is the first game you can play. The others can be
set up now and become playable as their decompilation gets further: follow
how far each game is on [openrac.dev](https://openrac.dev/#progress).
[games/](games/README.md) has a page for each game, and
[baserom/README.md](baserom/README.md) lists the supported discs with their
checksums.

## Make mods

OpenRAC comes with a level editor built on [Godot 4](https://godotengine.org/).
It turns the levels from your disc into Godot projects, so you can move, add and
remove enemies, crates and scenery, then play your changes in the PC port. The
launcher's **Mods** tab does all of this in a few clicks. To change how things
behave, use the mod SDK.

- [editor/README.md](editor/README.md): the level editor
- [docs/port/MODS.md](docs/port/MODS.md): the mod SDK

## Help out

Anyone can help: test the PC port and report bugs, decompile functions, or work
on the launcher, the port or the editor.

- [docs/DEVELOPING.md](docs/DEVELOPING.md): set up a checkout and build everything
- [CONTRIBUTING.md](CONTRIBUTING.md): the ground rules and how commits look
- [docs/policy/SOURCING.md](docs/policy/SOURCING.md): what may and may not be used
- [docs/README.md](docs/README.md): a map of every document

## Credits and license

OpenRAC brings several decompilation projects together. The people behind them
are in [CREDITS.md](CREDITS.md), and where each project came from is in
[docs/SOURCES.md](docs/SOURCES.md). Each part keeps its own license
([LICENSE.md](LICENSE.md)), and third-party work is listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

OpenRAC is an unofficial fan project. It is not affiliated with or endorsed by
the owners of Ratchet & Clank ([trademarks](LICENSE.md#trademarks)).
