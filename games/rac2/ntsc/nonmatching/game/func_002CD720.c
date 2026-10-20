/* from src/game/002CD620.c: 340 bytes, retail's size; 69 of 85 words equal (tools/check.py: 0 of 28).
 * Everything matches except the order of the two arms of the flag update: this spelling lays out
 * the `|= 4` arm first and the `&= ~4` arm after it, retail the other way round.
 *
 * The cause is where GCSE (the block LCM, -fno-edge-lcm) puts the high half of D_0025E2B8, which
 * lives in $s2 across the third call. Retail computes it once before the compare that follows the
 * second call (it lands in that jal's delay slot). With the arms in retail's order (`if (result1 ==
 * result2 && ...) &= ~4; else |= 4;`, the previous draft: 344 bytes), the `|= 4` arm is reached
 * only by jumps, so compute_preds_succs (flow.c, Cygnus edge splitting) splits the critical edge
 * from the call's block into it, LCM inserts the high half on that edge and at the end of the
 * D_001A63AC test instead, and the arm keeps a lui retail moves into every branch's delay slot.
 * With the `|= 4` arm as the fall-through (this file) the edge is not split and the placement is
 * retail's. Tried without a match: the arms in retail's order with &&, with !(...||...), with the
 * inner test negated, with gotos, with the early return turned into an if block, with a flag
 * variable, with a local pointer to D_0025E2B8 (it keeps the full address in $s0); duplicated
 * `|= 4` arms (356 bytes); -fedge-lcm (348) and the usual single extra flags. */
typedef struct {
    char reserved00[0x14];
    char *source;
    int value;
    char reserved1c[0x118];
    int fallback;
} CallbackState_002CD720;

typedef struct {
    char pad0[0x44];
    int f44;
    char pad48[8];
    int f50;
    int f54;
    int f58;
} State_0025E2B8_002CD720;

extern CallbackState_002CD720 D_001F2840;
extern int D_00138344;
extern State_0025E2B8_002CD720 D_0025E2B8;
extern char *D_0025E284;
extern int D_0025E464;
extern short D_001A63AC;
extern char *D_001A8CFC[] __attribute__((sda));
extern int func_00336EF0(void);
extern int func_0029C480(int);

int func_002CD720(void)
{
    int result = 0;
    int result1;
    int result2;
    int flags;
    int value;

    result1 = func_00336EF0();
    flags = D_00138344;
    if (flags & 0x10) {
        value = *(int *)(D_001F2840.source + 0xE0);
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            result = -1;
            goto after_first;
        }
        result = 0;
    } else if (flags & 0x900) {
        result = 1;
    } else {
        func_0029C480(flags);
    }
after_first:
    if (D_001A8CFC[0] == 0)
        return result;
    result2 = func_00336EF0();
    if (result1 != result2 || D_001A63AC != 0 ||
        ((D_0025E2B8.f50 != result1 || D_0025E2B8.f44 != 2) &&
         (D_0025E2B8.f54 != result1 || D_0025E2B8.f44 != 4)))
        *(int *)(D_0025E284 + 0x10) |= 4;
    else
        *(int *)(D_0025E284 + 0x10) &= ~4;
    if (result2 != result1)
        D_0025E464 = -0x22C;
    D_0025E2B8.f58 = func_00336EF0();
    return result;
}
