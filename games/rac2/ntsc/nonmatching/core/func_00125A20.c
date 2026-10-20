/* from src/core/00125970.c (INCLUDE_ASM there): 884 bytes, not compiled.
   Measured 2026-10-15 with tools/check.py: this candidate compiles to 904 bytes
   against the retail 884, 182 of the ~221 instructions differing. The first
   ~110 (prologue through the switch dispatch at +0x1AC) align to within a few
   instructions; the four mode arms do not.

   Identity: this is not game code. rac1-decomp src/core/00121D18.c heads the
   object 0x121D18-0x122140 as Sony's libgraph graph003.o -- checkModelVersion
   (func_00125970 here), _GetGsDxDyOffset (func_00125A10, also named by
   config/datamine/ntsc-u-sdk-matches.tsv as libgraph.a graph003.o) and this
   body -- so it is the SDK's display-env builder, built with Sony's 2.9-ee like
   the rest of the prebuilt library: profile `sdk -O2 -G2 -ffunction-sections`.
   Its only caller in C is src/game/0027F128.c func_00284E40, whose prototype
   (u8 *, short x5) and DisplayState argument order fix the signature.

   Fully decoded from the retail bytes, and every fact below is measured:

   - args: env = a0, psm = (short) a1, w = (short) a2, h = (short) a3,
     dx = (short) t0, dy = (short) t1. Only o0/o1 of the four offset words
     passed to _GetGsDxDyOffset are ever read (sp+0, sp+4).
   - the mode is the halfword at D_00137E00+2 (the image holds 1, 2, 1, 3, so
     this build takes the mode-2 arm); the template pointer comes from
     func_00125960, which returns &D_00137E00.
   - offsets gate: `if ((unsigned)(t->f2 - 2) >= 2 && func_00125970() != 0)` is
     the shape that produces retail's sltiu + bnel to the zero block, the call
     block first, and the duplicate `sw $zero,0xC($sp)` in the delay slot.
   - env->f00 = 0x66, env->f08 = 2/1/3 from (short)t->f0 and (short)t->f4
     (2 when f0 == 0, else 1 when f4 == 0, else 3), env->f20 = 0.
   - env->f10 = DISPFB-shaped: (psm & 0xF) << 15 | (((w + 0x3F) >> 6) & 0x3F) << 9.
   - env->f18 = DISPLAY-shaped: DX | DY << 12 | MAG-ish << 23 | (x - 1) << 32 |
     DH. Per arm (every division is (w + 0x9FF) / w):
       mode 2, t->f0 == 1: MAG = (n - 1) << 23; DX = (dx * n + o0 + 0x27C) & 0xFFF;
         DY = ((dy + o1 + 0x32) & 0xFFF) << 12; DW-1 = h - 1 or h * 2 - 1 by t->f4.
       mode 2, t->f0 != 1: MAG = (n * w - 1) << 23; DX = (t->f0 + o0 + 0x27C) & 0xFFF;
         DY = ((dy + o1 + 0x19) & 0xFFF) << 12; DH-1 = (h - 1) << 44. This arm
         jumps into the mode-3 arm's tail at +0x640 and shares its mflo.
       mode 3, t->f0 == 1: MAG = (dx * n - 1) << 23; DY0 = (dy + o1 + 0x48) & 0xFFF;
         DX = (DY0 + o0 + 0x290) & 0xFFF; DY = DY0 << 12; DW-1 by t->f4, in the
         tail at +0x568/+0x576 that the mode-2 first arm also uses.
       mode 3, t->f0 != 1: MAG = (n * w - 1) << 23; DX = (t->f0 + o0 + 0x290) & 0xFFF;
         DY = ((dy + o1 + 0x24) & 0xFFF) << 12; DH-1 = (h - 1) << 44.
       mode 0x50: no division. DX = ((720 - w) / 2) * 2 + dx * 2 + 0xE8 + o0;
         DY = dy + o1 + 0x23; DW-1 = w * 2 - 1; DH-1 = h - 1; MAGH bit 0x800000.
       else: func_0011C820(D_0013B358, x) -- the string at 0x13B358 is
         "sceGsDefDispEnv:Not support displaymode for 0x%x!!\n".
   - each of the four computed arms carries one dead multiply next to the live
     one: mode 2/f0==1 has `mult $v1,$s0` (n * w) with the live R5900 MULT1
     being dx * n; mode 3/f0==1 is the mirror image; and the 0x9FF division's
     `div` is emitted on paths that never read its mflo (a division whose result
     is unused survives because DIV counts as a side effect). Reproducing those
     dead pairs, and the two cross-arm tail merges above, is what the shape must
     do before the arms can match.

   Attempts made (tools/check.py, profile `sdk -O2 -G2 -ffunction-sections`):
   1. offsets gate written as `(x - 2) < 2 || !checkModelVersion()` -> 900 bytes,
      the then-block laid out first (retail has the call block first).
   2. same gate as `(x - 2) >= 2 && checkModelVersion()` (this file) -> 904 bytes;
      the dispatch and everything before the switch line up, but the arms still
      differ in the order the products, masks and shifts are emitted, and in the
      `(short)t->f2` read (retail reloads `lhu` and sign-extends with
      `sll`/`sra`; this candidate emits a plain `lh`).
   Next: write each arm as one expression in retail's emission order (reloading
   t->f* per use instead of caching it in locals) and get the two DW-1/DH-1
   tails (+0x568 and +0x640) cross-jumped by writing them as the same
   statement. */

#include "common.h"
#include "structs.h"

typedef struct {
    unsigned short f0;
    unsigned short f2;
    unsigned short f4;
    unsigned short f6;
} DispEnvMode_125A20;

typedef struct {
    u64 f00;
    u64 f08;
    u64 f10;
    u64 f18;
    u64 f20;
} GsDispEnv_125A20;

extern DispEnvMode_125A20 *func_00125960(void);
extern int func_00125970(void);
extern void func_00125A10(int, int *, int *, int *, int *);
extern int func_0011C820(const char *, ...);
extern char D_0013B358[];

void func_00125A20(u8 *env_p, short psm, short w, short h, short dx, short dy) {
    GsDispEnv_125A20 *env = (GsDispEnv_125A20 *)env_p;
    DispEnvMode_125A20 *t = func_00125960();
    int o0, o1, o2, o3;
    unsigned long long mag;
    unsigned long long dxy;
    int n;

    if ((unsigned)(t->f2 - 2) >= 2 && func_00125970() != 0) {
        func_00125A10((short)t->f2, &o0, &o1, &o2, &o3);
    } else {
        o0 = o1 = o2 = o3 = 0;
    }

    env->f00 = 0x66;

    if ((short)t->f0 == 0) {
        env->f08 = 2;
    } else if ((short)t->f4 == 0) {
        env->f08 = 1;
    } else {
        env->f08 = 3;
    }

    env->f10 = (((unsigned long long)(psm & 0xF) << 15) |
                ((unsigned long long)(((w + 0x3F) >> 6) & 0x3F) << 9));

    if ((short)t->f2 == 2) {
        if ((short)t->f0 == 1) {
            n = (w + 0x9FF) / w;
            mag = (unsigned long long)(n - 1) << 23;
            mag |= (unsigned long long)((short)t->f2 - 1) << 32;
            dxy = (unsigned long long)((dx * n + o0 + 0x27C) & 0xFFF);
            if ((short)t->f4 == 0) {
                dxy |= (unsigned long long)(h - 1) << 32;
            } else {
                dxy |= (unsigned long long)(h * 2 - 1) << 32;
            }
            env->f18 = mag | dxy | (unsigned long long)(((dy + o1 + 0x32) & 0xFFF) << 12);
        } else {
            n = (w + 0x9FF) / w;
            mag = (unsigned long long)(n * w - 1) << 23;
            mag |= (unsigned long long)((short)t->f2 - 1) << 32;
            dxy = (unsigned long long)((t->f0 + o0 + 0x27C) & 0xFFF);
            dxy |= (unsigned long long)(h - 1) << 44;
            env->f18 = mag | dxy | (unsigned long long)(((dy + o1 + 0x19) & 0xFFF) << 12);
        }
    } else if ((short)t->f2 == 3) {
        if ((short)t->f0 == 1) {
            n = (w + 0x9FF) / w;
            mag = (unsigned long long)(dx * n - 1) << 23;
            mag |= (unsigned long long)((short)t->f2 - 1) << 32;
            dxy = (unsigned long long)(((dy + o1 + 0x48) & 0xFFF) + o0 + 0x290);
            dxy &= 0xFFF;
            if ((short)t->f4 == 0) {
                dxy |= (unsigned long long)(h - 1) << 32;
            } else {
                dxy |= (unsigned long long)(h * 2 - 1) << 32;
            }
            env->f18 = mag | dxy | (unsigned long long)(((dy + o1 + 0x48) & 0xFFF) << 12);
        } else {
            n = (w + 0x9FF) / w;
            mag = (unsigned long long)(n * w - 1) << 23;
            mag |= (unsigned long long)((short)t->f2 - 1) << 32;
            dxy = (unsigned long long)((t->f0 + o0 + 0x290) & 0xFFF);
            dxy |= (unsigned long long)(h - 1) << 44;
            env->f18 = mag | dxy | (unsigned long long)(((dy + o1 + 0x24) & 0xFFF) << 12);
        }
    } else if ((short)t->f2 == 0x50) {
        env->f18 = (((unsigned long long)(h - 1) << 44) |
                    ((unsigned long long)(w * 2 - 1) << 32) | 0x800000 |
                    (unsigned long long)(((720 - w) / 2 * 2 + dx * 2 + 0xE8 + o0) & 0xFFF) |
                    ((unsigned long long)((dy + o1 + 0x23) & 0xFFF) << 12));
    } else {
        func_0011C820(D_0013B358, (short)t->f2);
    }

    env->f20 = 0;
}
