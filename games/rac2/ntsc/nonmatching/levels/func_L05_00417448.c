/* func_L05_00417448, 264 bytes, 3 placements.  ABANDON - allocation only.
   The body is recovered: the flag word at +0x34 of the object, the byte at
   +0x20, the call to func_002ED688 and the word it returns at +0x148, the
   spawner call, the level's record index (D_L05_00221D10 scaled by 128) into
   D_L05_001B2614, the three words at +0x1C0/+0x1C4/+0x1C8 and the float at
   +0x180 of the state, the two func_002A7940 calls and the func_002A7878 call
   whose float argument is read from the record, the func_002AF000 call with
   the three floats, the func_00282CC0 call and the last two stores (0x02f).
   The prologue, the first call and the whole tail from the func_002A7878 call
   on are byte-identical; the residue is the register allocation of the
   middle block: retail puts the -1 in $a0, the 0x78 in $v1 and the record
   index in $v0, and it loads D_L05_001B2614 through the register that holds
   the value ($s1), while this cc1 spreads them over $v0/$v1/$a0/$a1/$s0 and
   reorders the three stores.  Four orderings of the stores measured. */

#include "include_asm.h"
#include "common.h"

extern char *func_002ED688(char *object);
extern void func_L05_0037AEC0(char *object, int value);
extern float func_002A7878(float a, float b);
extern float func_002A7940(void);
extern void func_002AF000(char *a0, float a, float b, float c);
extern void func_00282CC0(char *a0, char *a1, char *a2);
extern unsigned int D_L05_00221D10;
extern unsigned int D_L05_001B2614;

void func_L05_00417448(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *frame = object + 0x10;
    char *p;
    unsigned int v0;
    float f;
    float g;
    float h;

    *(unsigned char *)(object + 0x20) = 1;
    *(unsigned short *)(object + 0x34) |= 0x100;
    p = func_002ED688(object);
    *(unsigned int *)(p + 0x148) = 1;
    func_L05_0037AEC0(object, 1);
    *(int *)(state + 0x1C4) = -1;
    *(int *)(state + 0x1C8) = 0x78;
    *(int *)(state + 0x1C0) = -1;
    v0 = D_L05_00221D10;
    p = (char *)(D_L05_001B2614 + (v0 << 7));
    *(float *)(state + 0x180) = 1.0f;
    f = func_002A7878(0.0f, *(float *)p);
    g = func_002A7940();
    h = func_002A7940();
    func_002AF000(frame, f, g, h);
    func_00282CC0(frame, p + 0x30, frame);
    *(unsigned int *)(state + 0x1B8) = 0;
    *(float *)(object + 0x2C) = 0.02f;
}
