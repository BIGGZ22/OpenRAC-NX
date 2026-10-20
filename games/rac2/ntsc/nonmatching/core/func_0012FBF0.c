/* from src/core/0012FB80.c: same size (368), 91 of 92 instructions identical.
 *
 * The libmpeg decode-or-skip driver, carried from rac1-decomp
 * src/core/0012AC80.c: func_0012BDD0 with this image's +0x3E20 core shift
 * (rac1's 0x12BD60 is func_0012FB80, its 0x12BF40 is func_0012FD60).
 *
 * The one difference is at +0xB8 (0x12FCA8), the `*(int *)inner = 1;` of the
 * `case 0` arm:
 *
 *   retail  addiu $v1,$zero,0x1 ; sw $v1,0x0($s0)
 *   ours    addiu $v0,$zero,0x1 ; sw $v0,0x0($s0)
 *
 * Everything else - the prologue and the 16-byte save slots, the
 * `(inner->0xD8 & 0x3F)` refusal through func_00130250 with D_0013BD68, the
 * two nested loops, the five-entry jump table with 3 and 4 sharing an arm,
 * every func_0012FE78 call with its index/slot pair and bump - matches
 * instruction for instruction, so the source is the one above and the
 * residual is the allocator's choice of scratch register for the constant.
 * This is the class of residual AGENTS.md sends up a tier rather than a
 * fourth wording (func_00115E90 is the same shape).
 *
 * Tried: the two statement orders of the `case 0` arm and a `default:` label
 * (the latter would add a sixth jump-table arm, which retail does not have).
 * Not tried: an allocation nudge that keeps $v0 live across the call.
 *
 * Verified: tools/check.py func_0012FBF0 --diff: boot: instruction at +0xb8
 * differs (same size).
 */

extern void func_00130250(void *, void *, int);
extern int func_0012CAB0(void *);
extern int func_0012FE78(void *, int, int);
extern void func_00130020(void *);

extern char D_0013BD68[];

int func_0012FBF0(void *arg0) {
    int pictureType = 1;
    int lastResult = 0;
    char *w = (char *)arg0;
    char *inner = *(char **)(w + 0x40);

    *(int *)inner = 0;
    if ((*(int *)(inner + 0xD8) & 0x3F) != 0) {
        func_00130250(inner, D_0013BD68, *(int *)(inner + 0xD8));
        return -1;
    }
    *(int *)(inner + 0x820) = 0;

    do {
        if (lastResult != -1) {
            for (;;) {
                pictureType = func_0012CAB0(inner);
                if (pictureType == 0) break;
                if (*(int *)(inner + 0x174) == *(int *)(inner + 0xD4)) break;
                if (*(int *)(inner + 0x848) == 0) break;
            }
        }

        switch (pictureType) {
        case 0:
            func_00130020(w);
            *(int *)inner = 1;
            break;
        case 1:
            *(int *)(inner + 0xA8) = 0;
            *(int *)(inner + 0xA4) = 0;
            *(int *)(inner + 0xA0) = 0;
            lastResult = func_0012FE78(w, 0, *(int *)(inner + 0x94));
            *(int *)(inner + 0xA0) = *(int *)(inner + 0xA0) + 1;
            break;
        case 2:
            lastResult = func_0012FE78(w, *(int *)(inner + 0xA4), *(int *)(inner + 0x98));
            *(int *)(inner + 0xA4) = *(int *)(inner + 0xA4) + 1;
            break;
        case 3:
        case 4:
            lastResult = func_0012FE78(w, *(int *)(inner + 0xA8), *(int *)(inner + 0x9C));
            *(int *)(inner + 0xA8) = *(int *)(inner + 0xA8) + 1;
            break;
        }

        if (*(int *)(inner + 0x820) != 0) {
            return 1;
        }
    } while (*(int *)inner == 0);

    return 1;
}
