/* func_002900B8 (1388 bytes, boot + 27 levels) - near miss: this C compiles to 1372 bytes.

   Belongs in src/game/0028F948-g8.c (gp-relative loads of D_001A9004..D_001A9010 and
   D_001A8D40 in ordinary flow need -G8; D_001A8CFC and D_001B23A4 are declared as
   incomplete `sda` arrays to stay absolute there). It returns D_001A8D40, so the
   `extern void func_002900B8(void);` used by func_00290010 in that file must become
   `extern int func_002900B8(void);` with a `(void (*)(void))` cast at the call (no code
   change for func_00290010: it still matched 28 of 28 with that edit).

   Blocking difference (toolchain, not C): retail has seven `mfc1 $r,$fN` immediately
   followed by an instruction that reads $r (`mfc1 $v1,$f1; mtc1 $v1,$f13`,
   `mfc1 $v0,$f1; subu $v1,$v1,$v0`) with no nop. This gas has cop_interlocks false for
   the r5900, so it inserts a nop after every such mfc1 (cc1 already marks it `#nop`), and
   cc1's scheduler, which models the mfc1 delay, interleaves the float chains that retail
   keeps in source order. The retail boot has 75 such adjacent mfc1 uses in 34 functions
   (func_0028D1E8 alone has 13) and none of them is matched yet; the matched corpus has no
   instance of the pattern. An assembler/scheduler change is needed before any of them can
   match.

   Also still different here, to revisit once that is fixed: the first jump pass moves the
   case 1/2 prologue in front of case 0 (retail keeps case 0 after the dispatch and ends the
   dispatch with `b case12`); the constant 484.0f is shared in a callee-saved FPR here while
   retail reloads it into $f12 for each func_00300A00 call (1.0f/0.0f/0.5f/206.0f are shared
   in both); callee-saved register numbering (retail keeps hi(D_001B9040) in $s5 and 0x8710
   in $s3). */

#include "include_asm.h"

extern int D_001A9000 __attribute__((sda));
extern int D_001A9004 __attribute__((sda));
extern float D_001A9008 __attribute__((sda));
extern int D_001A900C __attribute__((sda));
extern int D_001A9010 __attribute__((sda));

typedef struct {
    int active;
    int timer;
    char pad08[0x18];
    int entryIndex;
    int unk24;
} Subtitle_002900B8;

typedef struct {
    char pad[0xB0];
    float opacity;
} Hud_002900B8;

extern char *D_001A8CFC[] __attribute__((sda));
extern Subtitle_002900B8 D_00254E78;
extern Hud_002900B8 D_001B9040;
extern float D_001B23A4[] __attribute__((sda));
extern int D_001A8D40 __attribute__((sda));
extern void func_0027AB40(void);
extern float func_00283CE0(int);
extern int func_00283D38(int, int, float);
extern char *func_00336D50(char *, int);
extern void func_00300A00(char *, int, float, float, float, float, float);
extern void func_0027DE50(int, int, int, int, int);

int func_002900B8(void)
{
    float saved;
    float pulse;
    float step;
    float lower;
    float upper;
    float limit;
    int first;
    int second;
    int top;
    int i;

    if (D_001A8CFC[0] != 0 && D_00254E78.active == 0 && D_00254E78.unk24 == -1) {
        saved = D_001B9040.opacity;
        D_001B9040.opacity = 0.62f;
        func_0027AB40();
        pulse = func_00283CE0(D_001A9000) * 0.1f;
        first = func_00283D38(0x442D00, 0x60442D00, pulse);
        second = func_00283D38(0xF0C070, 0x55F0C070, pulse);
        func_00300A00(func_00336D50(D_001A8CFC[0] + 0x8710, 0xA6), first, 484.0f,
                      (float)(int)(D_001B23A4[0] * 206.0f + 0.5f), 1.0f, 1.0f, 0.0f);
        func_00300A00(func_00336D50(D_001A8CFC[0] + 0x8710, 0xA7), second, 484.0f,
                      (float)(int)(D_001B23A4[0] * 206.0f + 0.5f), 1.0f, 1.0f, 0.0f);
        switch (D_001A9004) {
        case 0:
            if (0.0f <= D_001A9008 && D_001A9008 <= 1.0f) {
                first = func_00283D38(D_001A900C & 0xFFFFFF, D_001A900C, pulse);
                second = func_00283D38(D_001A9010 & 0xFFFFFF, D_001A9010, pulse);
                top = (int)((float)(285 - (int)((1.0f - D_001A9008) * 157.0f)) * D_001B23A4[0] + 0.5f);
                step = (float)(int)(D_001B23A4[0] * 128.0f + 0.5f) * 0.25f;
                lower = (float)(int)(D_001B23A4[0] * 285.0f + 0.5f);
                upper = lower - step + 1.0f;
                for (i = 0; i < 5; i++) {
                    limit = (float)top;
                    if (upper <= limit)
                        upper = limit;
                    func_0027DE50((int)upper, (int)lower, 477, 489, first);
                    lower -= step;
                    upper -= step;
                    if (lower <= limit)
                        break;
                }
                upper = (float)(int)(D_001B23A4[0] * 128.0f + 0.5f);
                lower = upper + step - 1.0f;
                for (i = 0; i < 5; i++) {
                    limit = (float)top;
                    if (limit <= lower)
                        lower = limit;
                    func_0027DE50((int)upper, (int)lower, 476, 488, second);
                    upper += step;
                    lower += step;
                    if (limit <= upper)
                        break;
                }
            }
            break;
        case 1:
        case 2:
            first = func_00283D38(D_001A900C & 0xFFFFFF, D_001A900C, pulse);
            top = (int)((float)(285 - (int)((1.0f - D_001A9008) * 157.0f)) * D_001B23A4[0] + 0.5f);
            step = (float)(int)(D_001B23A4[0] * 128.0f + 0.5f) * 0.25f;
            lower = (float)(int)(D_001B23A4[0] * 285.0f + 0.5f);
            upper = lower - step + 1.0f;
            for (i = 0; i < 5; i++) {
                limit = (float)top;
                if (upper <= limit)
                    upper = limit;
                func_0027DE50((int)upper, (int)lower, 476, 488, first);
                lower -= step;
                upper -= step;
                if (lower <= limit)
                    break;
            }
            break;
        }
        D_001B9040.opacity = saved;
        func_0027AB40();
    }
    return D_001A8D40;
}
