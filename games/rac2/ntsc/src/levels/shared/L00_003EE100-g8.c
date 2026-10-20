#include "include_asm.h"

/* A body whose retail bytes address its global through $gp in ordinary flow: the assembler
   only selects that form when the back end emits the .extern size directive, which the
   pinned profile does at -G8 (see docs/COMPILER-NOTES.md). */
extern unsigned char *func_0029EFD0(int size);

extern unsigned char *D_L00_001B0A20 __attribute__((sda));

unsigned char *func_L00_003F8DC8(void)
{
    unsigned char *p = func_0029EFD0(0xD04);

    if (p != 0) {
        unsigned int v = 0xFF;

        D_L00_001B0A20 = p;
        *(unsigned short *)(p + 0x32) = v;
        p[0x20] = 1;
        p[0x30] = v;
    }
    return p;
}

/* Per-frame update of the player's gadget table: it walks the records of one table
   entry, scales each record's pair of floats and asks the slot manager to refresh
   them, then folds the two level globals into the table's offset. */

extern float D_L00_001B096C __attribute__((sda));

extern float D_L00_001B0970 __attribute__((sda));

extern char *func_L00_0032A088(int a0, int a1);

extern float func_00283B98(float a0, float a1);

extern void func_L00_003F5870(char *rec, int index);

extern int func_00282978(char *a0);

int func_L00_003F59F0(char *a0)
{
    char *state;
    char *table;
    char *base;
    char *rec;
    int index;
    int count;
    int i = 0;
    float value;

    state = *(char **)(a0 + 0x68);
    table = func_L00_0032A088(0xC04, *(short *)(state + 0x1A));
    index = *(short *)(state + 0x18);
    base = *(char **)(table + index * 4);
    count = *(short *)(state + 0x20);
    if (count > 0) {
        rec = base;
        do {
            *(float *)(rec + 0x20) = func_00283B98(*(float *)(rec + 0x20), *(float *)(rec + 0x28));
            func_L00_003F5870(base, i);
            rec += 0x30;
            i++;
        } while (i < count);
    }
    value = D_L00_001B096C;
    if (value > 7.0f) {
        D_L00_001B096C = 0.0f;
    } else if (value < -7.0f) {
        D_L00_001B096C = 0.0f;
    }
    D_L00_001B096C = D_L00_001B096C - D_L00_001B0970;
    if (D_L00_001B096C <= -4.0f) {
        D_L00_001B096C = D_L00_001B096C + 4.0f;
    }
    return func_00282978(state + 0x22) == 0;
}
