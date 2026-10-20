#include "include_asm.h"

/* -G8 siblings for src/game/0027F128.c: these need the small-data addressing
   mode (absolute lui forms and gp-relative loads). */

typedef float f32;
typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

extern void func_00282A60(void *, int, int);
extern char ref_L00_002EA630_00[];
extern int ref_L00_002EA630_01;
extern f32 func_00283CE0(s32);
extern s32 func_00283CF0(f32);

void func_00280630(void) {
    func_00282A60(ref_L00_002EA630_00, 0, 0x600);
    ref_L00_002EA630_01 = 0;
}

/* Sum the scaled per-character widths. */
int func_0027F1D8(u8 *text, int length, u8 *table, float scale) {
    int i = 0;
    f32 total = 0.0f;
    u8 *p;
    u8 c;

    if (length != 0 && text[0] != 0) {
        p = text;
        do {
            c = *p;
            i++;
            p++;
            total += func_00283CE0(*(s8 *)(table + c * 4 + 3)) * scale;
            if (i == length) break;
        } while (*p != 0);
    }
    return func_00283CF0(total);
}
