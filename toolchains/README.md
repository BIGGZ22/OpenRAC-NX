# toolchains: compilers you supply

The PS2 compilers, assemblers and linkers the games were built with are
proprietary. They are never committed (this directory is ignored by git,
apart from this file); each game's docs say where its builders get them.

## Ratchet & Clank, PAL (`games/rac1`)

The build uses two public community mirrors, cloned here:

```sh
git clone https://github.com/AngheloAlf/SN-Systems-ProDG_for_PS2_3.01 toolchains/sn-prodg-3.01
git clone https://github.com/AngheloAlf/sce_ps2_sdk_24 toolchains/sn-prodg-24
python3 tools/openrac.py setup rac1/pal   # links games/rac1/toolchain to this directory
```

- `sn-prodg-3.01`: `make`, the assembler and the linker.
- `sn-prodg-24`: GCC 2.95.3 (SN BUILD v1.14) for the game code and 989snd,
  and Sony's `2.9-ee-991111` for Sony's SDK code and libgcc.
- `ee-gcc-2.9-991111-01/bin/ee-gcc` (optional): a Linux build of Sony's
  2.9-ee-991111-01. Sony built its libgcc on Linux, and `__moddi3`,
  `__udivdi3` and `__umoddi3` match only with this compiler; when the file is
  there, `Makefile.sn` compiles libgcc with it, and without it those three
  differ in unused stack slots. The one these matches were verified with was
  built from the public
  [SSXModding/ps2-ee-toolchain](https://github.com/SSXModding/ps2-ee-toolchain)
  source (b595ded) with only the `c-parse.in` fix a modern bison needs.
  decomp.me's compilers release also has a prebuilt
  `ee-gcc2.9-991111-01.tar.xz` (the one rac1/ntsc's `setup.sh` downloads),
  not yet checked on these three. It runs inside the build container, which
  is 32-bit Linux.

The build runs in a Linux container with Wine on macOS and Linux; it mounts
all of OpenRAC, so the link resolves inside it. Details:
[games/rac1/README.md](../games/rac1/README.md) and
[docs/TOOLCHAIN.md](../games/rac1/docs/TOOLCHAIN.md).

## Ratchet & Clank, NTSC-U (`games/rac1/ntsc`)

Nothing to put here: `./setup.sh` downloads its pinned toolchains (every
download checked by SHA-256) into `games/rac1/ntsc/tools/`, and builds the
game compiler from the public GNU EE source archive with the project's
patches. See [docs/building.md](../games/rac1/ntsc/docs/building.md).

## Ratchet & Clank: Going Commando (`games/rac2/ntsc`)

You supply an SN ProDG 2.0 EE toolchain (`Ps2EeAs.exe`, `ld.exe`) for the
assembly rebuild, passed with `--toolchain` (`--c-toolchain` supplies only the
linker for the C step). The C compiler is a patched GNU EE-GCC 2.9 built under
WSL, which `scripts/wsl_chain.py` runs; see
[docs/COMPILER-NOTES.md](../games/rac2/ntsc/docs/COMPILER-NOTES.md). The
project's README still names SN ProDG 3.01 for the C.

## Ratchet & Clank: Up Your Arsenal (`games/rac3/ntsc`)

SN Systems ee-gcc 2.95.3 v1.36 is the `usr/local/sce/ee/gcc` tree of the
`sn-prodg-3.01` mirror above: its `cc1` reports "2.95.3 SN BUILD 1.36", and the
tree already has the layout the project's
[Setup page](../games/rac3/ntsc/docs/wiki/Setup.md) shows (`bin/ee-gcc2953.exe`,
`bin/ee-as.exe`, `ee/bin/as.exe`, `ee/bin/Ps2EeAs.exe`, ...). `python3
tools/openrac.py setup rac3/ntsc` links `toolchains/eegcc_2.95.3_sn_v1.36` to
it; point `UYA_TOOLCHAIN` (or `--toolchain`) there. (`sn-prodg-24`'s 2.95.3 is
SN BUILD 1.14, the one rac1/pal uses.) The library ranges of `boot_elf` and
`i5bootn` (`@ee29`) also need Sony's 2.9-ee-991111: point `UYA_EE29` at
`sn-prodg-24`'s `local/sce/ee/gcc`, whose `lib/gcc-lib/ee/2.9-ee-991111/` the
project picks. The full macOS and Linux recipe is in
[games/rac3/README.md](../games/rac3/README.md#getting-started).

## Ratchet: Deadlocked (`games/rac4/ntsc`)

The same two mirrors as rac1/pal, cloned above: SN GCC 2.95.3 v1.36 from
`sn-prodg-3.01` for the game code, and Sony's `2.9-ee-991111` from
`sn-prodg-24` for libgcc and libm. `python3 tools/openrac.py setup rac4/ntsc`
links `games/rac4/ntsc/toolchain` to this directory, and the build runs in
rac1/pal's container image. `tools/get_newlib.sh` clones the newlib snapshot
libm is built from into the project's ignored `private/`.
