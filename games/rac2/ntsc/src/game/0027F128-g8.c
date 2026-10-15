#include "include_asm.h"

/* -G8 siblings for src/game/0027F128.c: these need the small-data addressing
   mode (absolute lui forms and gp-relative loads). */

extern void func_00282A60(void *, int, int);
extern char ref_L00_002EA630_00[];
extern int ref_L00_002EA630_01;

void func_00280630(void) {
    func_00282A60(ref_L00_002EA630_00, 0, 0x600);
    ref_L00_002EA630_01 = 0;
}
