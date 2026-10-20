/* from src/core/001163B0.c: no C attempt - the depot does not hold this source.
 *
 * Measured: this is newlib's vfprintf.c as the SDK rebuilt it, not the copy in
 * the build tree, and the object is the same vfprintf.obj that
 * config/datamine/ntsc-u-sdk-matches.tsv shows holding strrev at 0x1184D0.  The
 * body agrees with `build/sdk-compiler-build/source/ee/newlib/libc/stdio/vfprintf.c`
 * (INTEGER_ONLY: the engine has no E G e f g case at all, and the badge string
 * "bug in vfprintf: bad base" that symbol_addrs records for it sits at 0x13A598),
 * and it keeps the source's format scan: `jal func_00115FC0` at +112 is
 * _mbtowc_r(_impure_ptr 0x133E74, &wc at sp+0x44, fmt, MB_CUR_MAX 0x133B80,
 * &state at sp+0x48), exactly the `while ((n = _mbtowc_r(...)) > 0)` loop.
 *
 * What differs from the tree's copy is the PRINT macro.  Retail has no iovec and
 * no __sprint: at +156 it tests `fp->_flags & 0x200` (a 16-bit load at fp+0xC) and
 * either memcpy(0x1153D4) into fp->_p and advances it, or calls the stream writer
 * func_00119AC0 (fp->_file at fp+0xE, base, length, 0); then `ret += len` at +240.
 * That, the object's own cvt (see func_001185E8 below), and __udivdi3/__umoddi3
 * (0x121B20 / 0x1220F0) for the digit loop are the only deviations found, but a
 * byte-exact body needs the SDK's vfprintf.c, not this one.
 *
 * Not attempted here for a second, independent reason: the conversion switch
 * compiles to a jump table, and retail keeps that table at 0x13A7B0, outside any
 * section C can place - the same refusal tools/build.py made for func_00118CC8
 * ("needs .rodata at its place, which is not C").  An exact C would still be a
 * near miss until that table has a home.
 *
 * Profile: -O2 -G2 -ffunction-sections -mastra-quad-saves (frame 0x140, `sq`
 * saves of s0..s7, fp, ra at 0xA0/0xB0/0xC0/0xD0/0xE0/0xF0/0x100/0x110/0x120/0x130).
 */
