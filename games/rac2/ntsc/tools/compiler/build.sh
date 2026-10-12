#!/bin/bash
# The game's C compiler: GNU EE 2.9-ee-991111b (cpp, cc1, as) rebuilt from its public source.
#
#   tools/compiler/build.sh [OUT]        # default build/compiler/tools
#
# Run it on 32-bit-capable x86-64 Linux (gcc -m32), e.g. in the rac2-linux container
# (docs/BUILDING.md). It downloads the two inputs to build/downloads/ and checks them,
# applies the patches in tools/compiler/patches/ (Lombyte's sce-991111b stack, MIT, with its
# retired 0034 and its earlier 0020) and the transformers in tools/compiler/transformers/,
# refuses to build unless the five patched sources hash as listed below, and builds.
#
# The binaries' hashes depend on the build host; the sources' do not. A compiler from these
# sources compiles every matched function in src/ to the retail bytes (tools/build.py).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
HERE=$ROOT/tools/compiler
OUT=$(mkdir -p "${1:-$ROOT/build/compiler/tools}" && cd "${1:-$ROOT/build/compiler/tools}" && pwd)
DL=$ROOT/build/downloads
W=${WORK:-/tmp/rac2-compiler}
J=$(nproc)
mkdir -p "$DL" "$W"

fetch() {  # fetch FILE SHA256 URL
  [ -f "$DL/$1" ] || curl -fL --retry 3 -o "$DL/$1" "$3"
  echo "$2  $DL/$1" | sha256sum --check --quiet
}
fetch gnu-ee-binutils-gcc-1.1.tar.gz 1f518043e252d6eda726386971d52eda26541ab936ea73a9783d73712b595f92 \
  "https://web.archive.org/web/20060518234647id_/http://ps2dev.sourceforge.net:80/downloads/ee/gnu-ee-binutils-gcc-1.1.tar.gz"
fetch bison-1.28.tar.gz c5d3e4858e17cb440cee9de7837f07277bcfb03507e9d2f0c506cab5efe36c3a \
  https://ftp.gnu.org/gnu/bison/bison-1.28.tar.gz

# The source tree.
rm -rf "$W/src" && tar xzf "$DL/gnu-ee-binutils-gcc-1.1.tar.gz" -C "$W" && mv "$W/gnu-ee-binutils-gcc" "$W/src"
cd "$W/src"
export GIT_CEILING_DIRECTORIES=$W
for p in 0000 0001 0015 0016 0019 0020 0021 0022 0025 0026 0027 0028 0029 0030 0031 0032 0033 0034 \
         0036 0037 0044 0045 0046 0047 0048 0049 0050 0051 0052 0053 0054 0055 0056; do
  git apply --whitespace=nowarn "$HERE"/patches/$p-*.patch
done
patch -R -p1 -s < "$HERE"/patches/0001-r5900-quad-saves.patch   # the retail saves in 8-byte slots
T=$HERE/transformers
python3 $T/neutralise_timode_anchor.py gcc/config/mips/mips.c
python3 $T/enable_loop_padding.py gcc/config/mips/mips.h
python3 $T/count_trap_length.py gcc/config/mips/mips.c
python3 $T/ascending_save_order.py gcc/config/mips/mips.c
python3 $T/reorder_save_blocks.py gcc/config/mips/mips.c
python3 $T/u4_mtc1_helper.py gas/config/tc-mips.c
python3 $T/restrict_mtc1_exemption.py gas/config/tc-mips.c
python3 $T/fold_zero_ti_store.py gcc
python3 $T/fold_zero_ti_return_value.py gcc
python3 $T/u3_frame_order_on.py gcc/config/mips/mips.c
python3 $T/disable_frame_order_default.py gcc/config/mips/mips.c
python3 $T/pad_div_erratum_nops.py gas/config/tc-mips.c
python3 $T/neutralise_div_dslot.py gcc
python3 $T/pad_div_erratum_branch.py gas/config/tc-mips.c >/dev/null
sha256sum --check --quiet <<'SUMS'
4a6a1ae14448e504eef4f58450973ebcd6c3ba45fdf204961642f0b97138fb8a  gcc/config/mips/mips.c
87d59c06d047cf7252349cb02aebc266f8b4dcecd537c827e6ec3966a10bbfc3  gcc/config/mips/mips.h
9720897dc353fd246c6f94d5882c3beb16c81fa9e42438ee8d184c8a78a581ad  gcc/config/mips/mips.md
38d52727addc0b0b299700835788808cf23fe59c863a961cdd6f28170faaf3cc  gcc/toplev.c
b22dfff81e41d3d378b1f35a231d2c11b60700967d6faf70b820f6300caf81d6  gas/config/tc-mips.c
SUMS

# bison 1.28: the 1999 grammars need its skeleton.
if [ ! -x "$W/bison/bin/bison" ]; then
  rm -rf "$W/bison-1.28" && tar xzf "$DL/bison-1.28.tar.gz" -C "$W"
  (cd "$W/bison-1.28" && CC="gcc -std=gnu89" CFLAGS="-O2 -fcommon" ./configure --prefix="$W/bison" \
     && make -j$J && make install) > "$W/bison.log" 2>&1
fi
export BISON_SIMPLE=$W/bison/share/bison.simple BISON_HAIRY=$W/bison/share/bison.hairy

F="-O2 -fno-strict-aliasing -fcommon -std=gnu89 -D_GNU_SOURCE"
rm -rf "$W/build" && mkdir "$W/build" && cd "$W/build"
printf 'obstack.o gcc.o mkstemp.o: override CFLAGS = -g\n' > host-flags.mk
export CC="gcc -m32" CFLAGS="$F"
{
  bash "$W/src/configure" --target=mips64r5900-sf-elf --host=i686-linux-gnu --build=i686-linux-gnu \
       --disable-nls --enable-languages=c --without-headers --prefix="$W/install"
  make -j$J all-libiberty
  # The 1999 Makefile lacks flow.o -> insn-flags.h: generate the headers first.
  make -C gcc -f Makefile -f "$W/build/host-flags.mk" LANGUAGES=c "CC=gcc -m32" "CFLAGS=$F" \
       BISON="$W/bison/bin/bison" insn-flags.h insn-codes.h insn-config.h
  make -C gcc -f Makefile -f "$W/build/host-flags.mk" -j$J LANGUAGES=c "CC=gcc -m32" "CFLAGS=$F" \
       BISON="$W/bison/bin/bison" cc1 cpp xgcc
  make -j$J all-gas
} > "$W/build.log" 2>&1 || { tail -30 "$W/build.log"; exit 1; }
cp gcc/cc1 gcc/cpp "$OUT/" && cp gas/as-new "$OUT/as"
echo "compiler in $OUT"
