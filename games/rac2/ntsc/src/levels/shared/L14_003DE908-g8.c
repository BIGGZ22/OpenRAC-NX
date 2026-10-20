#include "include_asm.h"

/* The -G8 bodies of src/levels/shared/L14_003DE908.c live here: their small data is
   reached through $gp, which the unit's own file (built -G0) cannot produce. */

typedef unsigned int u32;

typedef float f32;

/* Both words are inside $gp's reach (0x1AEFF0 + 0x1934/0x1938), so they carry the
   small-data attribute the shared unit's -G0 file cannot use. */
extern u32 D_L14_001B0924 __attribute__((sda));

extern u32 D_L14_001B0928 __attribute__((sda));

/* The float parameter is declared third: the ABI slots are the same (a0, a1, f12,
   a2, a3, t0) but the declaration's order is what puts retail's `lui $at,0x3F80;
   mtc1 $at,$f12` first and the two `$gp` loads around the `addiu $a1,$a0,0x10`.
   Sixth argument in $t0: the call site passes the raw 0x40000000 pattern. */
extern void func_L14_003F0F38(char *a, char *b, f32 e, u32 c, u32 d, int f);

/* Binds the object and its +0x10 companion to the two small-data words. */
void func_L14_003F0E68(char *a0)
{
    func_L14_003F0F38(a0, a0 + 0x10, 1.0f, D_L14_001B0924, D_L14_001B0928, 0x40000000);
}
