/* from src/core/001163B0.c: no C attempt - the depot does not hold this source.
 *
 * The float-capable twin of func_00119BF8, from the same vfprintf.obj and the
 * same newlib-derived vfprintf.c: same frame shape (`sq` saves, 0x280 here),
 * same _mbtowc_r scan, same conversion switch, and its own copy of the
 * "bug in vfprintf: bad base" string at 0x13A788 (the integer engine's is
 * 0x13A598).  It differs by the E G L e f g cases and by one conversion call.
 *
 * Measured on that call (0x119180): cvt is func_001185E8 and its four arguments
 * are (double value in $s5, precision in $fp, the format char in $s6, and
 * `dot` = !(flags & ALT) built from `andi $s3,0x1` + `movn $fp,1,fp`).
 * It returns a *string*: the caller stores $v0 and takes `size` from strlen
 * (0x1157AC) of it at +1856.  That is not newlib's eight-argument cvt, which
 * returns a digit pointer and fills *decpt / *ndig / *softsign - the SDK
 * replaced it (see nonmatching/core/func_001185E8.c), so the tree's vfprintf.c
 * cannot be transcribed here either.
 *
 * Same second blocker as the integer engine: the conversion switch's jump table
 * lives at 0x13A6C0, so tools/build.py would refuse an exact C ("needs .rodata at
 * its place, which is not C"), as it already does for func_00118CC8.
 *
 * Profile: -O2 -G2 -ffunction-sections -mastra-quad-saves (frame 0x280, `sq`
 * saves of s0..s7, fp, ra at 0x1E0..0x270).
 */
