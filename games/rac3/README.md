# Ratchet & Clank: Up Your Arsenal (2004)

Released in Europe as *Ratchet & Clank 3*.

| Version | Directory | Came from | Builds on |
|---|---|---|---|
| NTSC-U, `SCUS_973.53` (disc v1.00) | [ntsc/](ntsc/README.md) | [rac3-uya-decomp](https://github.com/OpenRAC/rac3-uya-decomp) | Windows; Linux and macOS with wibo |

The project (formerly ratchet-uya-decomp) decompiles three programs and
rebuilds each byte for byte ([ntsc/docs/targets.md](ntsc/docs/targets.md)):
`frontbin.elf`, the frontend and menu program; `boot_elf.elf`, the main
executable (the engine core and a second copy of the front end, seeded from
frontbin's C, [ntsc/docs/boot_elf.md](ntsc/docs/boot_elf.md)); and
`i5bootn.elf`, the launcher the disc starts first
([ntsc/docs/i5bootn.md](ntsc/docs/i5bootn.md)). They live inside the game's
data, and [Wrench](https://github.com/chaoticgd/wrench) unpacks them. At the
commit synced on 2026-10-07, `tools/pr_check.py --target all` counts frontbin
1,800 of 1,867 entries final (1,419 functions in C), boot_elf 2,265 of 2,809
(1,580 in C) and i5bootn 191 of 199 (35 in C). The level programs are
counted for progress, and a first set of common level functions is matched in
C ([ntsc/docs/common_level_c.md](ntsc/docs/common_level_c.md)).

Its compiler research, 15 compilers against 8 flag sets, is in
[ntsc/docs/compiler_matrix_findings.md](ntsc/docs/compiler_matrix_findings.md),
and the launcher's in [ntsc/docs/compiler_matrix_i5bootn.md](ntsc/docs/compiler_matrix_i5bootn.md)
(library code from Sony's 2.9-ee, which emits sibling calls; SN 2.95.3 never
does). It is the most systematic compiler study among the projects
([toolchains/README.md](../../toolchains/README.md)).

Disc checksums are in [baserom/README.md](../../baserom/README.md); progress
in [progress/](../../progress/README.md). The project is licensed under the GNU
GPL v3 ([ntsc/LICENSE](ntsc/LICENSE)), as rac1/pal is since 2026-10-07; the
other games are MIT, which matters when code moves between them
([LICENSE.md](../../LICENSE.md), [open questions](../../docs/policy/OPEN_QUESTIONS.md#1-licensing)).

## Getting started

On Windows, follow [ntsc/docs/wiki/Setup.md](ntsc/docs/wiki/Setup.md). On
macOS and Linux, from the top of OpenRAC (tested on an Apple Silicon Mac on
2026-10-07: all three programs matched retail byte for byte):

```sh
# The compiler: SN ee-gcc 2.95.3 v1.36 is the ee/gcc tree of the SN ProDG 3.01
# mirror (toolchains/README.md); setup links toolchains/eegcc_2.95.3_sn_v1.36 to it.
python3 tools/openrac.py setup rac3/ntsc

# wibo runs the Windows compiler: wibo-macos natively (needs Rosetta 2;
# xattr -d com.apple.quarantine wibo-macos if a browser downloaded it), or
# wibo-i686 on Linux. Wrench unpacks the disc; it has Linux and Windows builds.
mkdir -p build/rac3 && cd build/rac3
curl -sSLO https://github.com/decompals/wibo/releases/download/1.2.0/wibo-macos && chmod +x wibo-macos
curl -sSLO https://github.com/chaoticgd/wrench/releases/download/v0.5/wrench_v0.5_linux-glibc2.39-0ubuntu8.5.zip
unzip -q wrench_v0.5_linux-glibc2.39-0ubuntu8.5.zip -d wrench && cd ../..

# The three programs live inside the game's data. On a Mac, run Wrench's Linux
# build in an amd64 container (Rosetta runs it); the unpack writes about 4.8 GB.
docker run --rm --platform linux/amd64 -v "$PWD:$PWD" -w "$PWD" ubuntu:24.04 \
  build/rac3/wrench/wrench_v0.5_linux-glibc2.39-0ubuntu8.5/wrenchbuild \
  unpack baserom/SCUS_973.53.iso -o build/rac3/wrench_out -s
W=build/rac3/wrench_out/uya_scus_973_53
cp $W/globals/misc/frontbin.elf $W/boot_elf.elf $W/files/i5bootn.elf games/rac3/ntsc/
shasum games/rac3/ntsc/*.elf
# 487975305f8a263c750dfede50391b575ed07835  boot_elf.elf
# 3bc94ee895e4b4af9b5602a229af599c1103b542  frontbin.elf
# 71f3ecfc54c3d24d1475ef9efe8228fbfe59d65f  i5bootn.elf

# Python packages, the assembly, the build, the pre-PR check. The library
# ranges marked @ee29 need Sony's 2.9-ee-991111: the SDK 2.4 mirror's ee/gcc
# folder (toolchains/README.md) has it.
python3 -m venv build/rac3/venv && build/rac3/venv/bin/pip install -r games/rac3/ntsc/tools/requirements.txt
cd games/rac3/ntsc
export UYA_TOOLCHAIN=$PWD/../../../toolchains/eegcc_2.95.3_sn_v1.36
export UYA_RUNNER=$PWD/../../../build/rac3/wibo-macos
export UYA_EE29=$PWD/../../../toolchains/sn-prodg-24/local/sce/ee/gcc
../../../build/rac3/venv/bin/python tools/setup_asm.py --target all
../../../build/rac3/venv/bin/python tools/build.py --target all \
  --toolchain $UYA_TOOLCHAIN --runner $UYA_RUNNER
../../../build/rac3/venv/bin/python tools/pr_check.py --target all
```

The build ends with one `MATCH` line per program, for example `MATCH:
build/frontbin.bin sha1 3bc94ee... (0x218924 bytes)`. To try one function,
`tools/try_func.py` takes the same three variables (and `UYA_TARGET` for
boot_elf or i5bootn). `localdecomp/server.py` assumes
Windows. `pr_check.py --obj build/src/text.c.o` still rejects the `.rodata`
that the switch jump tables put in `text.c.o`, which the linker script places
on purpose; the check itself is out of date.

The wiki's other pages ([ntsc/docs/wiki/](ntsc/docs/wiki/Home.md)) cover the
workflow, matching patterns and pull requests.
