/* from src/core/00133490-g8.c: 180 bytes against retail's 188 - same size of
 * body, same frame (0x20) in the first attempt, but retail keeps only
 * s0/s1/ra where both candidates need a third saved register.
 *
 * snd_CheckLoadInProgress (symbol_addrs, "tests/waits bank-load-in-progress
 * flag"; func_00133250-g8.c calls it -- func_00133580(0)). Retail's shape is
 * read off the disassembly: if the boot flag D_001A748C (gp-0x7B64) is
 * clear, tail into func_00124B88 and return its value; otherwise status =
 * (D_001A7100 < 1) unsigned, publish it to D_001A7498 (gp-0x7B58), return 0
 * when it is 1, return 1 when the caller passed 1, and otherwise pump
 * func_00132028 (snd_Pump) with func_0011B500(&D_001A7100, &D_001A713F)
 * until the status word reads non-zero.
 *
 * This unit is `-g8.c` (gp-relative globals): the two sda globals are
 * D_001A748C / D_001A7498, and D_001A7100 / D_001A713F are NOT_SDA - retail
 * reaches both with plain lui/addiu, i.e. gp = 0x1AEFF0.
 *
 * Where it differs (tools/check.py func_00133580 --diff, 180/188): retail
 * materialises `lui/addiu` for D_001A7100 before EACH of the two
 * func_0011B500 calls and keeps only 0x1A713F in a saved register, while this
 * cc1 hoists the D_001A7100 %hi into $s0 (and with the plain address form,
 * into $s2 as well) outside the loop. The same cc1 rematerialises constants
 * in func_00123C28, so it is reachable, but no statement layout tried here
 * produced it. Also, retail's redundant `bnez $v0` after the `arg0 == 1`
 * test survives, which is what the second `if (busy != 0) return 0;` above
 * is for: without it the candidate is 8 bytes shorter still.
 *
 * Tried: address as a local (168 bytes, s2 for the 0x713F half);
 * `(D_001A7100 < 1)` signed then unsigned (slti -> sltiu, 180 bytes).
 *
 * The candidate below is the 180-byte one, minus the D_001A7498 declaration
 * which already belongs to the unit.
 */

extern int D_001A748C __attribute__((sda));
extern int D_001A7498 __attribute__((sda));
extern unsigned int D_001A7100 NOT_SDA;
extern char D_001A713F NOT_SDA;
extern void func_0011B500(void *, void *);
extern int func_00124B88(void);
extern int func_00132028(void);

int func_00133580(int arg0) {
    char *end = &D_001A713F;
    int busy;

    if (D_001A748C == 0)
        return func_00124B88();

    func_0011B500(&D_001A7100, end);
    busy = D_001A7100 < 1;
    D_001A7498 = busy;
    if (busy == 1)
        return 0;
    if (arg0 == 1)
        return 1;
    if (busy != 0)
        return 0;
    do {
        func_00132028();
        func_0011B500(&D_001A7100, end);
        busy = D_001A7100 < 1;
        D_001A7498 = busy;
    } while (busy == 0);
    return 0;
}
