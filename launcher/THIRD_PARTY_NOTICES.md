# Third-party notices: the launcher

What the launcher takes from others, and under which license. Libraries it
depends on (Tauri, Svelte, serde and the rest) keep their own licenses in
`node_modules/` and Cargo's registry; they are listed in `package-lock.json`
and `Cargo.lock`.

## The OpenRAC website

The colours, type, header bar, game cards and progress bar follow
[OpenRAC/openrac-site](https://github.com/OpenRAC/openrac-site) (`src/app/globals.css`,
`src/components/Header.tsx`, `src/components/Progress.tsx`, `src/lib/projects.ts`).
`public/wrench.webp` is the site's `public/wrench.webp`; `public/icon.png` is its
`src/app/icon.png`; the app icons in `src-tauri/icons/` are made from its
`src/app/apple-icon.png` with `tauri icon`. The Android icons
(`src-tauri/gen/android/app/src/main/res/mipmap-*`) are `yarn tauri icon
src-tauri/icons/android.json -o <dir>`'s `android/` folder: the same icon,
and for Android's adaptive icon `android-foreground.png`, that icon at 64% on
its own orange so that the system's mask never cuts the wrench.

```
MIT License

Copyright (c) 2026 OpenRAC contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## The T3SDK launcher

The launcher's design (a Rust side that owns every file and process, a UI
that names jobs from a fixed list, a browser preview backed by a mock, one
job at a time with live output) follows the launcher of
[T3SDK](https://github.com/Veradictus/Thief3-Decomp) for Thief: Deadly Shadows.
Two small helpers are adapted from it: the bounded process probe
(`run` in `core/src/detect.rs`, from its `src-tauri/src/proc.rs`) and the
removal of terminal escape sequences from tool output (`plain` in
`core/src/jobs.rs`, from `strip_escapes` in its `src-tauri/src/tasks.rs`).

```
MIT License

Copyright (c) 2026 The T3SDK contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## ReRAC

The save editor (`core/src/save_edit.rs`) decodes the Ratchet & Clank save from
the game's own code in OpenRAC's decompilations, and checked what each chunk
means against ReRAC's notes on the save (`docs/plan/game_state.md`) and its
game state (`crates/rc-game/src/game_state.rs`), ISC license, which in turn
checked its chunk names against Wrench's `savegame.wtf`. Only facts about the
format were used (chunk ids, sizes and meanings); no code was taken.

## Fonts

Bundled into the app from npm (`@fontsource/audiowide`,
`@fontsource-variable/orbitron`, `@fontsource-variable/inter`), each under the
[SIL Open Font License 1.1](https://openfontlicense.org), whose full text ships in
each package's `LICENSE`:

| Font      | Copyright                                                                                              |
| --------- | ------------------------------------------------------------------------------------------------------ |
| Audiowide | Copyright (c) 2012 by Brian J. Bonislawsky DBA Astigmatic (AOETI), with Reserved Font Name "Audiowide" |
| Orbitron  | Copyright 2018 The Orbitron Project Authors (https://github.com/theleagueof/orbitron)                  |
| Inter     | Copyright 2016 The Inter Project Authors (https://github.com/rsms/inter)                               |

## Embree, in the carried probe baker

The game copy an installer carries includes `openrac-probe-bake`, the baker of
the Mods tab's Enhanced graphics, which runs on the player's computer over their
own extracted levels. It has [Embree](https://github.com/RenderKit/embree) 4.4.1
built in (Apache License 2.0, Copyright Intel Corporation; on ARM with SSE2NEON,
MIT License). Embree's `LICENSE.txt` and `third-party-programs.txt` are carried
beside it, in `port/build/release/licenses/embree/` of the copy; the details are
in OpenRAC's [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md#embree).
