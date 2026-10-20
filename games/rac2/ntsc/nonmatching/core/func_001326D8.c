/* from src/core/00131780-g8.c: 328 bytes against retail's 316. The -g8 unit
 * route works - every `$gp` access below lands on the right symbol - and the
 * body's control flow, calls, argument order and epilogue are retail's. What
 * is left is the scheduling/register allocation around the busy-flag block
 * and the two wait paths.
 *
 * Recipe measured for this unit, worth reusing: the file must be a `*-g8.c`
 * sibling (build.py gives it -O2 -G8 -ffunction-sections) and every small
 * global needs `__attribute__((sda))`; with the game compiler that produces
 * retail's `lw`/`sw X($gp)`. GP is 0x001AEFF0, which pins the addresses:
 * -0x7B28 is D_001A74C8, -0x7B68 is D_001A7488, -0x7AF8 is D_001A74F8 and
 * -0x7E70 is D_001A7180.
 *
 * Shapes the retail body fixes:
 *  - the sentinel is unsigned: retail materialises -1 as `lui 0xFFFF` +
 *    `ori 0xFFFF` (load_register's unsigned path), so the store and both
 *    comparisons are -1u;
 *  - the same object at 0x001A7180 is read through $gp in the value
 *    positions and materialised absolutely where its *address* is passed to
 *    func_0011D620 (`lui $t1,0x1A` + `addiu`, not `addiu $t1,$gp`), and the
 *    wait loop reads it with a literal address (`lui $v0,0x1A` +
 *    `lw $v0,0x7180($v0)`) - hence the second, non-sda declaration and the
 *    literal below;
 *  - the refusal path is written as two returns (`if (D_001A74F8 != 0)
 *    return 0;` then the report and `return 0;`): retail sets $v0 twice, in
 *    the branch's delay slot and in the jump's. With a single shared `return
 *    0` (the `if (D_001A74F8 == 0) report;` spelling) the candidate measures
 *    300 bytes, so the split return is the closer shape.
 *
 * Verified: tools/check.py func_001326D8 --diff: boot: 328 bytes, expected
 * 316. No instruction reads a register nothing writes; the row itself is
 * sound.
 */

/* $gp-relative unit: the small-data globals below are reached through $gp
   (the game compiler's -G8 with __attribute__((sda)); the SDK compiler emits
   absolute addresses even under -G8). GP is 0x001AEFF0, which is what pins
   the addresses named here: -0x7B28 is D_001A74C8, -0x7B68 is D_001A7488,
   -0x7AF8 is D_001A74F8 and -0x7E70 is D_001A7180. The three strings and the
   two driver words go through $at absolutely, as they do in retail.

   snd_BankLoadFromIOP (symbol_addrs's own name): the synchronous half of the
   pair whose asynchronous half is func_00132498. It refuses when the busy
   flag is set (clearing D_001A7488 and reporting 0x001A7730 unless the quiet
   flag is set), latches the bank pointer in D_001A71C0 and the -1 sentinel in
   D_001A7180, then polls func_0011D810(0x001A7140), reporting 0x001A7578 and
   running the sound pump func_00132028() with a func_0011AEA0(0) yield each
   pass, and finally issues RPC 0x59 through func_0011D620 with the nine
   arguments retail sets up ($a0-$a3, $t0-$t3 and a zero word on the stack).
   The failure path reports 0x001A7760 and latches 0x106; otherwise it waits
   for the sentinel to move. */

extern int D_001A74C8 __attribute__((sda));

extern int D_001A7488 __attribute__((sda));

extern int D_001A74F8 __attribute__((sda));

extern unsigned int D_001A7180 __attribute__((sda));

/* The same object again, without the sda attribute: retail passes its
   address absolutely (lui/addiu through the argument register) while
   reading and writing its value through $gp. */
extern unsigned int a_001A7180 __asm__("D_001A7180");

extern int D_001A71C0;

extern int D_001A7140;

extern char D_001A7730[];

extern char D_001A7578[];

extern char D_001A7760[];

extern void func_00115D48(const void *, ...);

extern void func_0011AEA0(int);

extern int func_0011D810(const void *);

extern int func_0011D620(const void *, int, int, int *, int, int *, int, int,
                         int);

int func_001326D8(int arg0) {
    if (D_001A74C8 != 0) {
        D_001A7488 = 0;
        if (D_001A74F8 != 0)
            return 0;
        func_00115D48(D_001A7730);
        return 0;
    }
    D_001A71C0 = arg0;
    D_001A7180 = -1u;
    while (func_0011D810(&D_001A7140) != 0) {
        if (D_001A74F8 == 0)
            func_00115D48(D_001A7578);
        func_00132028();
        func_0011AEA0(0);
    }
    if (func_0011D620(&D_001A7140, 0x59, 1, &D_001A71C0, 4, &a_001A7180, 4, 0,
                      0) < 0) {
        if (D_001A74F8 == 0)
            func_00115D48(D_001A7760);
        D_001A7488 = 0x106;
        return 0;
    }
    while (*(unsigned int *)0x001A7180 == -1u)
        func_0011AEA0(0);
    return *(unsigned int *)0x001A7180;
}
