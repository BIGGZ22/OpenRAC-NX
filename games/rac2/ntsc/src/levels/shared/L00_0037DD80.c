#include "include_asm.h"

extern int D_L05_00231E40_1[][4];

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

typedef struct {
    u32 field00,field04;
    s32 field08;
    u8 gap0c[4];
} OrbitalPairTableView16;

extern u8 D_L00_00231340[];

extern u8 D_L00_001B26B4[];

extern int func_00282948(void *p);

extern void func_L00_0037FE08(void);

extern void func_L00_00387F98(void);

extern void func_L00_0038A9E0(void);

/* shared native body of family 954231c412 */
int func_L00_0037DD80(int a0, int a1)
{
    int i;
    int r = 1;

    for (i = 0; i < 32; i++)
        if (D_L05_00231E40_1[i][1] == a0 && D_L05_00231E40_1[i][0] == a1) {
            r = 0;
            break;
        }
    return r;
}

/* searches 32 sixteen-byte records for matching key and owner words and returns the value word or minus one */
s32 func_L00_0037DDE8(u32 key,u32 owner) {
    OrbitalPairTableView16 *entry=(OrbitalPairTableView16 *)D_L00_00231340;
    s32 checked=0;
    do {
        ++checked;
        if(entry->field04==key && entry->field00==owner) return entry->field08;
        ++entry;
    } while(checked<32);
    return -1;
}

INCLUDE_ASM("asm/levels", func_L00_0037DE30);

INCLUDE_ASM("asm/levels", func_L00_0037DEF8);

/* Near miss: nonmatching/levels/func_L00_0037E000.c - the condition chain is right but this
   cc1 runs the loop downwards (i = 31, i--) where retail counts up, and the `e != 0` test
   comes out as `beql` with the store in its annulled slot against retail's `beqz`. */
INCLUDE_ASM("asm/levels", func_L00_0037E000);

/* clears the second word of every sixteen-byte record whose second word equals a0 */
void func_L00_0037E0C0(int a0)
{
    OrbitalPairTableView16 *entry = (OrbitalPairTableView16 *)D_L00_00231340;
    u32 *field = &entry->field04;
    int i;

    for (i = 31; i >= 0; i--) {
        if (*field == (u32)a0)
            *field = 0;
        field += 4;
    }
}

INCLUDE_ASM("asm/levels", func_L00_0037E418);

/* runs the three per-frame passes in the order the level expects */
void func_L00_0037E5C0(void)
{
    func_L00_00387F98();
    func_L00_0038A9E0();
    func_L00_0037FE08();
}

/* func_L00_0037E5F0 is built with -G8: see L00_0037DD80-g8.c. */

/* func_L00_0037E640 is matched in L00_0037E640.c (built with -mno-split-addresses). */
