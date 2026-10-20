/* from src/core/00115200.c: not a C candidate - the retail body is the SDK's
 * hand-written strcmp, and symbol_addrs/usa-v2/symbol_addrs.txt already names
 * it: `strcmp = 0x00115544; // CONFIRMED - SDK strcmp(a,b), hand-written
 * r5900 strcmp.S ... SOURCE is newlib's hand-written
 * libc/machine/r5900/strcmp.S, vendored verbatim in going-decompiled/libc and
 * LINKED from it in the r5900_memcmp_strcmp group ... Assembled by the SN
 * Ps2EeAs it reproduces the ROM byte-exact in USA and EU (FACT 9816)`.
 *
 * The vendored source is on this machine too - not only in going-decompiled's
 * checkout but in this repo's own build tree:
 *   build/sdk-compiler-build/source/ee/newlib/libc/machine/r5900/strcmp.S
 * alongside memchr.S, memcmp.S, memcpy.S, memmove.S, memset.S, strcat.S,
 * strchr.S, strcpy.S, strlen.S, strncat.S, strncmp.S, strncpy.S - the whole
 * hand-written r5900 string library, which is the source of this row and of
 * func_00115250/00115340/001153D4/00115690/001157AC/001158F4 next door.
 *
 * Evidence from the bytes that no C reaches (tools/check.py cannot be tried
 * because no C source shape produces them):
 *   - +84  0x78830000 is lq $v1,0($a0), +88 is lq $v0,0($a1): 128-bit quad
 *     loads the EE backend only emits from TImode objects;
 *   - +88/+104 are MMI (opcode 0x1C, funct PSUBB/PAND/PCPYUD class) byte
 *     compares: a 16-byte equality test with the 0x8080...8080 mask built by
 *     the +20..+44 ori/dsll chain, printed by rabbitizer as `.word # INVALID`;
 *   - +204..+236 the 8-byte path: `ld`, `nor`, `dsubu`, two `and`s and the
 *     mask, i.e. the zero-byte test on a doubleword;
 *   - +280..+328 the scalar tail, `subu $v0,$v1,$v0` in the jr delay slot,
 *     returning (unsigned char)a[i] - (unsigned char)b[i] as the comment says.
 * The body also has no prologue at all (no $sp adjustment, no $ra save) and
 * jumps between the 16-, 8- and 1-byte paths by `bnel`/`beql` - the shape of
 * the .S file's `__OPTIMIZE_SIZE__`-off macros, not of compiled C.
 *
 * Recommendation (maintainer decision): ASM_FUNC, like the core syscall stubs
 * and the other members of this group. If the .S is to be linked instead, it
 * has to be assembled by the SN Ps2EeAs (EE gas gives a few words shorter:
 * the two assemblers expand the dli mask constants differently).
 */
