/*
 * func_0033B910 (src/game/0033AE20.c, 28 places): NEAR MISS.
 *
 * Compiles to 1060 bytes against retail's 1060 (game profile, -O2 -G0). Measured with
 * tools/check.py and RTL dumps (-dL -dg); every instruction shape below is retail's, what
 * still differs is loop-invariant motion and global register allocation:
 *
 * 1. D_001A7BC8's address stays half inside the 21-entry loop (lui hoisted by GCSE, the
 *    `addiu %lo` left in the loop), where retail hoists both array addresses whole into
 *    $s7/$fp. loop.c's threshold for this loop is 61 and drops by 3 per moved register:
 *    the life-1 invariants table+4, table+8 and the two array lo_sums can only all move in
 *    the second loop pass if that pass sees at most 52 insns; it sees 54, the two extra
 *    being the dead increments of the x/scale DEST_REG givs that pass 1 reduces (a
 *    DEST_REG giv used once cannot absorb others). Retail's two table pointers (x/scale
 *    on one, y/text on the other, 8 apart) require exactly this pass-1/pass-2 split, so
 *    a row pointer (`&D_00265140[i]`) fixes the hoisting but merges the fields wrongly.
 *    Measured levers: the colours only leave the loop when passed through the inline
 *    helper (its parameter lives 5 insns); `lit = D_001A7BE8[i]` read into an int is 1 insn
 *    shorter than the direct test (that version hoists D_001A7BE8 but not D_001A7BC8).
 * 2. `obj` gets $s3 where retail has $s0: global-alloc priorities are obj 15 refs / 82 =
 *    0.549 against the widget pointer (obj + 0x2EC) 13 / 70 = 0.557, a near tie that
 *    cascades into most of the differing words (obj+N arguments, the post-loop pointers).
 * 3. The 0x180000FF argument's lui/ori pair is scheduled one slot earlier in each draw.
 *
 * Retail-shaped pieces that are measured to matter: `base = D_001C4FA0` (lui/addiu then
 * lw 0x230), the walker copy `r = first` (retail's `daddu $s0,$s3,$zero`), the colour
 * if/else polarity (`== 0` first) and `if (D_001A7BC8[i] != 0)`.
 */
#include "include_asm.h"

#ifndef RAC2_T_ENTRY_F624A0727
#define RAC2_T_ENTRY_F624A0727
typedef struct { char pad[8]; short n; void (*fn)(char *); } ENTRY_F624a0727;
#endif

typedef struct {
    int x;
    int y;
    int text;
    float scale;
} Row_0033B910;

extern void func_00336320(char *);
extern void func_00336A78(char *);
extern void func_00336828(char *);
extern void func_00335E38(void *, float, float, float, float);
extern void func_00336920(char *, int, int);
extern void func_00335F88(void *, float, float, float, float);
extern int *func_00335E20(char *);
extern float *func_00335E10(char *);
extern void func_0028FAB8(int, int, int, int, int, int);
extern Row_0033B910 D_00265140[];
extern unsigned char D_001A7BC8[];
extern unsigned char D_001A7BE8[];
extern char D_001C4FA0[];

static __inline__ void SetColor_0033B910(char *w, int c)
{
    *func_00335E20(w) = c;
}

void func_0033B910(char *obj)
{
    int i;
    char *base;

    func_00336320(obj);
    func_00336320(obj + 0x4C);
    func_00336320(obj + 0x98);
    func_00336A78(obj + 0x1D8);
    func_00336A78(obj + 0x220);
    func_00336A78(obj + 0x268);
    func_00336828(obj + 0x2B0);
    for (i = 0; i < 21; i++) {
        func_00335E38(obj + 0x2EC, (float)D_00265140[i].x + (*(float **)(obj + 0x1C8))[0],
                      (float)D_00265140[i].y + (*(float **)(obj + 0x1C8))[1], 0.0f, 0.0f);
        func_00336920(obj + 0x2EC, 0xE99E, D_00265140[i].text);
        func_00335F88(obj + 0x2EC, D_00265140[i].scale, D_00265140[i].scale, 0.0f, 0.0f);
        if (D_001A7BE8[i] == 0)
            SetColor_0033B910(obj + 0x2EC, 0x70A0C0C0);
        else
            SetColor_0033B910(obj + 0x2EC, 0x7029A1FF);
        if (D_001A7BC8[i] != 0)
            func_00336828(obj + 0x2EC);
    }
    base = D_001C4FA0;
    if (*(int *)(base + 0x230) < 21) {
        char *first = obj + 0xE4;
        char *r = first;
        char *q = obj + 0x114;
        int k;

        for (k = 2; k >= 0; k--) {
            ENTRY_F624a0727 *t = *(ENTRY_F624a0727 **)q;
            t->fn(r + t->n);
            r += 76;
            q += 76;
        }
        func_0028FAB8((int)(func_00335E10(first)[0] + 1.0f),
                      (int)(func_00335E10(obj + 0x130)[1] + 6.0f),
                      (int)(func_00335E10(first)[0] + 1.0f),
                      (int)(func_00335E10(first)[1] - 4.0f), 0x180000FF, 0);
        func_0028FAB8((int)(func_00335E10(obj + 0x17C)[0] + 8.0f),
                      (int)(func_00335E10(obj + 0x17C)[1] + 2.0f),
                      (int)(func_00335E10(obj + 0x17C)[0] + 304.0f),
                      (int)(func_00335E10(obj + 0x17C)[1] + 2.0f), 0x180000FF, 0);
        func_0028FAB8((int)(func_00335E10(first)[0] + 1.0f),
                      (int)(func_00335E10(obj + 0x130)[1] + 6.0f),
                      (int)(func_00335E10(first)[0] + 1.0f),
                      (int)(func_00335E10(first)[1] - 4.0f), 0x180000FF, 0);
        func_0028FAB8((int)(func_00335E10(obj + 0x17C)[0] + 8.0f),
                      (int)(func_00335E10(obj + 0x17C)[1] + 2.0f),
                      (int)(func_00335E10(obj + 0x17C)[0] + 304.0f),
                      (int)(func_00335E10(obj + 0x17C)[1] + 2.0f), 0x180000FF, 0);
    }
}
