#!/usr/bin/env bash
# Generates build-sn/rac1.ld from the same addresses recorded in
# config/splat.yaml. Kept as a generator script (not a static .ld) so the
# addresses stay traceable to one source of truth.
# bss regions are real (zero-filled) loaded sections here, not NOLOAD --
# this exact ld build (v2.3.7.513-era) doesn't advance the location
# counter correctly after a NOLOAD section (verified: an explicit
# `. = X;` right after one is silently ignored, next section lands back
# at the NOLOAD section's own start address instead). See
# docs/TOOLCHAIN.md. build-sn/{core_bss_pad_1,core_bss_pad_2,bss_pad}.o must exist first --
# tools/build_sn_data.sh generates them.
cat > build-sn/rac1.ld <<'EOF'
OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)

SECTIONS
{
  /* Retail's gp base, read straight out of its own .reginfo section
     (Elf32_RegInfo.ri_gp_value). The 0x7f80-byte window below it is the
     small-data area: core.lit, .lit, .bss and the bottom of .data. All
     1011 gp-relative references in the disassembly land inside it. */
  _gp = 0x00166D00;

  /* core segment (vram 0x100080, rom 0x1000) */
  /* vutext (0x100080-0x112380) is VU0 microcode, not linked here yet */

  . = 0x112380;
  .core_text : {
EOF

# core_text objects, in link order, from config/core_text.objects -- the
# one list Makefile.sn and the tools read too.
grep -v -e '^#' -e '^$' config/core_text.objects | tr -d $'\r' | while read -r obj _start; do
  echo "    $obj(.text)" >> build-sn/rac1.ld
done

cat >> build-sn/rac1.ld <<'EOF'
  }

  /* libgcc keeps its real names; the rest of the image (and every tool)
     knows these functions by address. Map both ways. */
  func_0011DF18 = __do_global_ctors;
  func_0011DFC8 = __main;
  func_0011DFE8 = __divdi3;
  func_0011E6D8 = __fixunsdfdi;
  func_0011E7C8 = __floatdidf;
  func_0011E860 = __moddi3;
  func_0011EEC8 = __muldi3;
  func_0011EF28 = __udivdi3;
  func_0011F4F8 = __umoddi3;
  /* The compiler clears large aggregate initializers with a memset
     libcall (func_00222B98); newlib's memset lives at this address. */
  memset = func_001153FC;
  /* memcpy with a non-constant size stays a call (func_0011CE70);
     newlib's memcpy lives here. */
  memcpy = func_00115248;
  /* dp-bit.o / fp-bit.o carry Sony's GOFAST names, which is also what the
     compilers call. */
  func_0011FA38 = __pack_d;
  func_0011FB68 = __unpack_d;
  func_0011FC08 = _fpadd_parts;
  func_0011FE48 = dpadd;
  func_0011FEA0 = dpsub;
  func_0011FF08 = dpmul;
  func_001201B0 = dpdiv;
  func_00120318 = __fpcmp_parts_d;
  func_00120430 = dpcmp;
  func_00120480 = litodp;
  func_00120538 = dptoli;
  func_001205D0 = dptoul;
  func_00120670 = __make_dp;
  func_001206B0 = __unpack_f;
  func_00120778 = fptodp;
  __thenan_df = 0x001597F0;
  __CTOR_LIST__ = 0x0015ED18;

  . = 0x12f580;
  .core_data : { build-sn/core_data.data.o(.data) }

  . = 0x152300;
  /* Split around the read-only data compiled objects now bring themselves
     (__divdi3's static __clz_tab, SDK functions' literals), each in its
     retail hole: config/core_rodata.txt, tools/core_rodata.py. */
  .core_rdata : {
EOF
python tools/core_rodata.py ld >> build-sn/rac1.ld
cat >> build-sn/rac1.ld <<'EOF'
  }

  . = 0x154200;
  /* __main's static `initialized` is retail's D_001597EC, so l2__main.o's
     .bss goes exactly there, between two halves of the padding (see
     tools/build_sn_data.sh). Its .data only holds the stripped
     __do_global_dtors' static pointer, which nothing references any more;
     retail's copy is in the core_data blob, so ours is discarded. */
  .core_bss : {
    build-sn/core_bss_pad_1.o(.core_bss_pad)
    build-sn/libgcc/l2__main.o(.bss)
    build-sn/core_bss_pad_2.o(.core_bss_pad)
  }
  /DISCARD/ : { build-sn/libgcc/l2__main.o(.data) }

  /* main segment (vram 0x15ed80, rom 0x5fd00) */

  . = 0x15ed80;
  .core_lit : { build-sn/core_lit.rodata.o(.rodata) }

  . = 0x15f000;
  .lit : { build-sn/lit.rodata.o(.rodata) }

  . = 0x161380;
  .bss : { build-sn/bss_pad.o(.bss_pad) }

  . = 0x165580;
  /* The retail data, cut around the jump tables compiled functions now
     bring, with each table in its hole (tools/jump_tables.py). */
  .data : {
EOF
python tools/jump_tables.py ld >> build-sn/rac1.ld
cat >> build-sn/rac1.ld <<'EOF'
  }

  . = 0x1e8f00;
  .lvl_vtbl : { build-sn/lvl_vtbl.data.o(.data) }

  . = 0x1e8f80;
  .lvl_camvtbl : { build-sn/lvl_camvtbl.data.o(.data) }

  . = 0x1e9000;
  .lvl_sndvtbl : { build-sn/lvl_sndvtbl.data.o(.data) }

  . = 0x1e9080;
  .text : {
EOF

# text objects, in link order, from config/text.objects.
grep -v -e '^#' -e '^$' config/text.objects | tr -d $'\r' | while read -r obj _start; do
  echo "    $obj(.text)" >> build-sn/rac1.ld
done

cat >> build-sn/rac1.ld <<'EOF'
  }

  /* Sections a nonzero -G makes the compiler emit.

     .sdata/.sbss come out empty -- our C defines no data of its own,
     every global is extern and resolved from the retail data objects or
     bss_equs -- so they are parked past the end of the image.

     .lit4/.lit8 are different: FP literals are addressed via $gp too
     (R_MIPS_LITERAL), so they MUST live inside the small-data window
     0x15ED00..0x16ED00 or their references cannot reach. They go in the
     128-byte gap below .core_lit, which is the only free space in the
     window (everything from 0x15ED80 up is occupied by core_lit/.lit/
     .bss/.data). Currently 12 bytes are needed (3 float literals, all in
     text.o), so this fits -- but it is a tight spot, and if the literal
     pool grows past 128 bytes this placement has to be rethought rather
     than nudged. Note these literals land at OUR addresses, not the ones
     retail's own .lit pool uses, so any function referencing one may
     differ from retail in that operand. */
  . = 0x2400000;
  .lit4  : { *(.lit4) }
  .lit8  : { *(.lit8) }
  .sdata : { *(.sdata) }
  .sbss  : { *(.sbss) *(.scommon) }
}
EOF
echo "wrote build-sn/rac1.ld"
