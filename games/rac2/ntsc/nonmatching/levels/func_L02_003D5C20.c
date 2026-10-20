/* func_L02_003D5C20 (208 bytes, 1 place) - best attempt, NOT MATCHED.
 *
 * The body is byte-exact from +0x20 (0x003D5C40) to the end: the pending sub-object
 * release, the four armed slots, the two sound flags and the two clearing calls all
 * land in the retail's order, with the retail's `beqz`/`bgez` pair, its delay-slot
 * `addiu $s2,$s2,-1` and its `addiu $s0,$s0,0x4` in the branch slots.
 *
 * What still differs is the prologue's callee-saved save order, and only that:
 *   retail : sd $s3,0x18 ; sd $s0,0x0 ; daddu $s3,$a0,$zero ; sd $s2,0x10 ; sd $s4,0x20
 *   here   : sd $s3,0x18 ; sd $s4,0x20 ; daddu $s3,$a0,$zero ; sd $s0,0x0 ; sd $s2,0x10
 * The registers themselves agree ($s3 = arg0, $s0 = the slot pointer, $s2 = the
 * counter, $s4 = state+0x220, $s1 = the state); retail saves the slot pointer and the
 * counter before the $s4 slot, this cc1 saves $s4 second.  The order follows the
 * allocator's priority order, so what is needed is a spelling in which the
 * state+0x220 value is allocated after the two loop variables.
 *
 * Measured, no effect: the `state+0x220` pointer as a variable assigned before the
 * release block (this file), the same expression written out at each of its three
 * uses, the pointer assigned after the loop, the slot and the counter assigned
 * before the release block, and the counter initialised in the declaration instead
 * of after the block.  They move the $s4 save between position 2 and position 4 and
 * the $s0/$s2 saves with it, but none lands on retail's order.  The `qcopy`/`qzero`
 * family is not involved here; the body is plain C.
 */

extern void func_L02_003D5DF8(char *);
extern void func_0029F338(char *);
extern void func_0029F9F0(char *, char *);
extern void func_00282A60(char *, int, int);

void func_L02_003D5C20(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *q = state + 0x220;
    char *slot;
    char *p;
    int n;

    if (*(char **)(state + 0x300) != 0) {
        func_L02_003D5DF8(*(char **)(state + 0x300));
        *(char **)(state + 0x300) = 0;
    }
    slot = state + 0x2A0;
    n = 4;
    do {
        p = *(char **)(slot);
        if (p != 0) {
            func_0029F338(p);
            *(char **)(slot) = 0;
        }
        n--;
        slot += 4;
    } while (n >= 0);
    if (*(unsigned char *)(state + 0x261) != 0)
        func_0029F9F0(arg0, state + 0x260);
    if (*(unsigned char *)(state + 0x221) != 0)
        func_0029F9F0(arg0, q);
    func_00282A60(q, 0, 0x40);
    func_00282A60(q, 0, 0x40);
}
