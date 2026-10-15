#!/usr/bin/env python3
"""
Generates build-sn/rac1.ld from config/core_text.objects and config/text.objects.
Cross-platform Python replacement for rac1.ld.sh (works on Windows, Linux, macOS).
"""
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

def generate_ld():
    core_objs_path = REPO_ROOT / "config" / "core_text.objects"
    text_objs_path = REPO_ROOT / "config" / "text.objects"
    out_path = REPO_ROOT / "build-sn" / "rac1.ld"
    out_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append('OUTPUT_FORMAT("elf32-littlemips")')
    lines.append('OUTPUT_ARCH(mips)')
    lines.append('')
    lines.append('SECTIONS')
    lines.append('{')
    lines.append('  /* Retail\'s gp base, read straight out of its own .reginfo section */')
    lines.append('  _gp = 0x00166D00;')
    lines.append('')
    lines.append('  . = 0x112380;')
    lines.append('  .core_text : {')

    # Core text objects
    with open(core_objs_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            obj = parts[0]
            if obj == "build-sn/libgcc/fp_addsub_df.o":
                lines.append('    func_0011FC08 = .;')
            lines.append(f'    {obj}(.text)')

    lines.append('  }')
    lines.append('')
    lines.append('  /* libgcc keeps its real names; the rest of the image (and every tool)')
    lines.append('     knows these functions by address. Map both ways. */')
    lines.append('  func_0011DFE8 = __divdi3;')
    lines.append('  func_0011E6D8 = __fixunsdfdi;')
    lines.append('  func_0011E7C8 = __floatdidf;')
    lines.append('  func_0011EEC8 = __muldi3;')
    lines.append('  func_0011FE48 = __adddf3;')
    lines.append('  func_0011FEA0 = __subdf3;')
    lines.append('  func_0011FF08 = __muldf3;')
    lines.append('  func_001201B0 = __divdf3;')
    lines.append('  func_00120318 = __fpcmp_parts_d;')
    lines.append('  func_00120430 = __cmpdf2;')
    lines.append('  func_00120480 = __floatsidf;')
    lines.append('  func_00120538 = __fixdfsi;')
    lines.append('  func_001205D0 = dptoul;')
    lines.append('  func_00120670 = __make_dp;')
    lines.append('  __pack_d   = func_0011FA38;')
    lines.append('  __unpack_d = func_0011FB68;')
    lines.append('  __thenan_df = 0x001597F0;')
    lines.append('  dpadd  = __adddf3;')
    lines.append('  dpsub  = __subdf3;')
    lines.append('  dpmul  = __muldf3;')
    lines.append('  dpcmp  = __cmpdf2;')
    lines.append('  litodp = __floatsidf;')
    lines.append('')
    lines.append('  . = 0x12f580;')
    lines.append('  .core_data : { build-sn/core_data.data.o(.data) }')
    lines.append('')
    lines.append('  . = 0x152300;')
    lines.append('  .core_rdata : {')
    lines.append('    build-sn/core_rdata_1.o(.rodata)')
    lines.append('    build-sn/libgcc/l2_divdi3.o(.rodata)')
    lines.append('    build-sn/core_rdata_2.o(.rodata)')
    lines.append('  }')
    lines.append('')
    lines.append('  . = 0x154200;')
    lines.append('  .core_bss : { build-sn/core_bss_pad.o(.core_bss_pad) }')
    lines.append('')
    lines.append('  . = 0x15ed80;')
    lines.append('  .core_lit : { build-sn/core_lit.rodata.o(.rodata) }')
    lines.append('')
    lines.append('  . = 0x15f000;')
    lines.append('  .lit : { build-sn/lit.rodata.o(.rodata) }')
    lines.append('')
    lines.append('  . = 0x161380;')
    lines.append('  .bss : { build-sn/bss_pad.o(.bss_pad) }')
    lines.append('')
    lines.append('  . = 0x165580;')
    lines.append('  .data : { build-sn/data.data.o(.data) }')
    lines.append('')
    lines.append('  . = 0x1e8f00;')
    lines.append('  .lvl_vtbl : { build-sn/lvl_vtbl.data.o(.data) }')
    lines.append('')
    lines.append('  . = 0x1e8f80;')
    lines.append('  .lvl_camvtbl : { build-sn/lvl_camvtbl.data.o(.data) }')
    lines.append('')
    lines.append('  . = 0x1e9000;')
    lines.append('  .lvl_sndvtbl : { build-sn/lvl_sndvtbl.data.o(.data) }')
    lines.append('')
    lines.append('  . = 0x1e9080;')
    lines.append('  .text : {')

    # Text objects
    with open(text_objs_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            obj = parts[0]
            lines.append(f'    {obj}(.text)')

    lines.append('  }')
    lines.append('')
    lines.append('  . = 0x2400000;')
    lines.append('  .lit4  : { *(.lit4) }')
    lines.append('  .lit8  : { *(.lit8) }')
    lines.append('  .sdata : { *(.sdata) }')
    lines.append('  .sbss  : { *(.sbss) *(.scommon) }')
    lines.append('}')
    lines.append('')

    content = "\n".join(lines)
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(content)
    print("wrote build-sn/rac1.ld")

if __name__ == "__main__":
    generate_ld()
