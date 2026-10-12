# Building

The build disassembles the retail programs, links the C in `src/` in place of the matching
assembly, and compares every loaded byte with the retail programs. Everything it needs
from the game comes from your own disc; nothing game-derived is committed.

## What you need

- A disc image of **Ratchet & Clank: Going Commando, USA v1.01** (`SCUS_972.68`), as
  `baserom/SCUS_972.68.iso`. SHA-256
  `9db2e33e276133cc283647fa3279b37911955e123d6199d10065547eaa9b1ce5`.
- Python 3.12 with `pip install -r requirements.txt` (splat, spimdisasm, rabbitizer).
- **SN ProDG 2.0**'s assembler and linker as `toolchain/sn-prodg-2.0/ee/bin/Ps2EeAs.exe`
  (SHA-256 `c839dd63…`, `ps2eeas.exe` in the `eegcc_sn_v2.73a` archive) and `ld.exe`
  (`80f3724a…`, the SN ProDG 3.01 linker). Proprietary: bring your own, never commit them.
- An x86-64 Linux environment with 32-bit support (`gcc -m32`) for the compilers, and Wine
  for the SN tools.

## Steps

```sh
python3 tools/levels.py dump          # baserom/SCUS_972.68 and the 27 level programs
tools/compiler/build.sh               # the game's compiler, into build/compiler/tools/
python3 tools/compiler/sdk/build.py   # the SDK units' compiler, into build/compiler/sdk/
python3 tools/build.py all            # build and compare the boot and all 27 levels
python3 tools/progress.py             # progress/report.json
```

`tools/build.py boot`, `tools/build.py level 4` and `tools/build.py levels` build a part.
Each program prints `OK` with its C functions, or stops at the first byte that differs.
A C function that does not match where the catalogue places it is listed with the reason
and stays assembly there; the program still has to come out equal.

## On macOS, Windows or any host: the build image

`tools/docker/run.sh` runs a command in the `rac2-build` image (32-bit Debian with Wine, Python
and the build tools; `tools/docker/Dockerfile`, published to GHCR by CI), with the repository
mounted at the same path. It uses a local image, else pulls the published one, else builds it.

```sh
bash tools/docker/run.sh python3 tools/levels.py dump
bash tools/docker/run.sh tools/compiler/build.sh
bash tools/docker/run.sh python3 tools/compiler/sdk/build.py
bash tools/docker/run.sh python3 tools/build.py all
bash tools/docker/run.sh python3 tools/progress.py
```

On Apple Silicon the image runs under QEMU (Rosetta cannot run 32-bit code), so it is slow.
A faster split: run Python on the host and only the tools in containers, a linux/amd64 one for
the compilers and the linux/386 image for the SN tools, with the repository at the same path:

```sh
RAC2_LINUX="docker exec rac2-gnu" RAC2_WINE="docker exec -w {cwd} rac2-wine wine" \
  python3 tools/build.py all
```

`RAC2_LINUX` prefixes the compiler commands and `RAC2_WINE` the SN tools (`{cwd}` becomes the
directory they run in). On x86-64 Linux with Wine and `gcc -m32`, neither is needed.

## CI

`.github/workflows/checks.yml` runs the tool tests and the repository check;
`progress.yml` checks that `progress/report.json` is current (`tools/progress.py --check`),
validates it with objdiff and uploads it as `SCUS_972.68_report` for decomp.dev;
`docker-publish.yml` publishes the build image. None of them builds the game: CI has no disc and
no SN tools, so the build is run locally and its result committed.

## The compilers

- **The game's code**: GNU EE-GCC 2.9-ee-991111b rebuilt from its public source with
  Lombyte's patch stack and the transformers this project measured
  ([tools/compiler/](../tools/compiler/README.md)). The build stops unless the five patched
  sources hash as recorded; the binaries' own hashes depend on the host, so a compiler is
  checked by its output.
- **The boot's SDK units** (the `src/core/` files `config/cflags.txt` marks `sdk`): Sony EE-GCC 2.9-ee-991111-01, unmodified, from
  the public SSXModding snapshot (`tools/compiler/sdk/build.py`), with `-O2 -g2 -gstabs`.

Flags are per file, never per function: `config/cflags.txt` (default
`-O2 -G0 -ffunction-sections`, `-G8` for a file named `*-g8.c`).
