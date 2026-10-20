#include "common.h"

/* Bodies of L11_004465F8.c whose globals the compiler reaches through $gp (-G8). */

typedef int TI128 __attribute__((mode(TI)));

extern s32 D_L11_005A0000 __attribute__((sda));

extern float func_32CB48(float f12, float f13);
extern s32 func_32CAB0(s32 a0);
extern s32 func_301700(float f12);
extern s32 func_3006B8(char *a0, s32 a1);
extern void func_32ED70(char *a0);
extern float func_3015A8(float f12, float f13);
extern void func_44F978(char *a0, char *a1, s32 a2, s32 a3);
extern void func_44FA58(char *a0, char *a1, s32 a2, s32 a3, float f12);
extern void func_44FBA8(char *a0, char *a1);
extern char *func_32F000(char *a0, s32 a1, s32 a2);
extern s32 func_338F10(char *a0, char *a1, s32 a2);
extern void func_301020(char *a0, char *a1);
extern void func_337410(char *a0, char *a1, s32 a2);
extern s32 func_4461C0(s32 a0, char *a1);
extern void func_32C9E0(char *a0, s32 a1, s32 a2);
extern void func_44C268(char *a0);
extern void func_44C4F0(char *a0);

/* turret entry of the 0x4A0 rig: arms the sub-object, seeds its parameter block and the
   0.4/0.6/0.8 constant set, then runs the per-frame aim and fire path */
extern s32 func_448CE0(char *a0);
extern void func_4493F8(char *a0, s32 a1);
extern void func_449970(char *a0, s32 a1, s32 a2);
extern void func_448DF8(char *a0);
extern void func_449878(char *a0);

/* The id-list walk of the 0x7FFF family (worker 02's matched skeleton): the entry's
   two-byte ids are addresses divided by 256 into the resident object bank, and bit 15
   of the last id ends the walk.  Returns 1 only from the third live 0xD98 object. */
extern u8 *D_L11_001FC420[];
extern u8 *D_L11_001B359C __attribute__((sda));

s32 func_L11_00448D28(u8 *a0)
{
    u8 *base;
    u8 *p;
    u16 *q;
    u8 *e;
    s32 n;

    if (a0[0x21] == 0xFF)
        return 0;
    p = D_L11_001FC420[a0[0x21]];
    if (p == 0)
        return 0;
    q = (u16 *)p;
    base = D_L11_001B359C;
    n = 0;
    for (;;) {
        e = base + (*q & 0x7FFF) * 256;
        if (e != 0 && e[0x20] != 0xFE && e[0x20] != 0xFD && *(s16 *)(e + 0xAA) == 0xD98
            && e[0x95] == 6 && *(s16 *)(*(char **)(e + 0x68) + 0x366) == 1) {
            n++;
            if (n >= 3)
                return 1;
        }
        if ((s16)*q < 0)
            break;
        q++;
    }
    return 0;
}
