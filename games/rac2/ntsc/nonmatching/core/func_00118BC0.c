/* from src/core/001163B0.c: C that compiles to 264 bytes where retail has 264
 * - the same size, but 55 words differ.
 *
 * Staging buffer: each byte of src[i] goes through the write pointer
 * D_0013466C, the count D_00134668 is incremented, and when it reaches 0x80
 * the buffer at D_0013C100 is handed to func_0011B1E8 (object, buffer base,
 * count), the pointer and the count are reset, and a zero return ends the
 * function with 0. A fourth argument of 1 flushes what is held and resets
 * without writing, and returns what the flush returned. func_00119AC0 is the
 * same routine over 0x4670/0x4674 and the buffer at 0x0013C180.
 *
 * Two corrections to the earlier draft, both measured:
 *   - the count test is `>= 0x80`, not `== 0x80`: retail compares with
 *     `slti $v0,$a2,0x80; bnez $v0` (skip the flush while the count is
 *     below 0x80), and with `== 0x80` the candidate is twelve bytes longer
 *     (276) - with `>= 0x80` it is exactly retail's 264;
 *   - the flush path returns what func_0011B1E8 returned (retail's `b` to
 *     the epilogue keeps the call's result in $v0), not a constant.
 *
 * What differs now is register pressure and the order of the two accesses to
 * the write pointer: this cc1 materialises the full address of both globals
 * into callee-saved registers and keeps two copies of the staging buffer
 * (one in $s7, one copied to $s1 for the call argument), so it needs nine
 * saved slots and a 0x90 frame where retail uses eight and 0x80; retail keeps
 * only the page base in the register and folds the offset into the access
 * (`lw $a0,0x466C($s1)`, one `lui $s1,0x13` outside the loop). Count and
 * pointer are therefore $s2/$s3 here against $s2/$s1 in retail. Retail also
 * loads the write pointer twice in the loop (once for the store of the byte,
 * once for the increment) and keeps the increment in the branch's delay
 * slot; the `*D_0013466C[0]++` spelling stores the incremented pointer
 * before the count test instead and is four bytes longer.
 *
 * Profile: the frame slots are `sq`/`lq` (the r5900's quad saves), so this
 * unit needs the game compiler with `-mastra-quad-saves`; the unit's own
 * `sdk` line gives `sd`/`ld` slots and 260 bytes.
 */
extern char D_0013C100[];

extern int D_00134668[1];

extern char *D_0013466C[1];

extern int func_0011B1E8(int arg0, char *buf, int count);

int func_00118BC0(int arg0, char *src, unsigned int len, int flush) {
    unsigned int i;
    int r;

    if (flush == 1) {
        r = func_0011B1E8(arg0, D_0013C100, D_00134668[0]);
        D_00134668[0] = 0;
        D_0013466C[0] = D_0013C100;
        return r;
    }
    for (i = 0; i < len; i++) {
        *D_0013466C[0]++ = src[i];
        if (++D_00134668[0] >= 0x80) {
            if (func_0011B1E8(arg0, D_0013C100, D_00134668[0]) == 0)
                return 0;
            D_0013466C[0] = D_0013C100;
            D_00134668[0] = 0;
        }
    }
    return i;
}
