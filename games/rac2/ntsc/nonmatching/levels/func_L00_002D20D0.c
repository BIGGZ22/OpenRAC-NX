/* func_L00_002D20D0, 152 bytes, 27 placements.  ABANDON - register-allocation and reload wall.
   Copies resident clocks and the owner's key bytes onto the object the state word names, then
   follows two of those bytes through the object's own table (object->+0x24 + 0x48, indexed by
   the object's bytes +0x42 and +0x43) into the two records the object points at (+0x58/+0x5C),
   each reached through a pointer plus the owner's byte +0x41 scaled by four.
   Candidate is 160 bytes (two long); every load, store and offset is the same, but retail keeps
   the resident block in `$a0` and the object in `$a3` where this cc1 uses `$t1`/`$t0`, and
   retail reads the owner's +0x41 byte twice through ONE owner load where this cc1 reloads the
   owner once more (the `sb` to the object is treated as possibly aliasing it here).
   Unblocking: a shape that reuses the owner register across the two byte reads, e.g. naming the
   byte once and letting the two stores read it. */

#include "include_asm.h"

extern char D_00189E20_2[];

void func_L00_002D20D0(void)
{
    char *base = D_00189E20_2;
    char *object = *(char **)(base + 0x2494);
    char *table;
    char *first;
    char *second;

    if (object == 0)
        return;
    table = *(char **)(object + 0x24);
    *(float *)(object + 0x4C) = *(float *)(base + 0xC24);
    table += 0x48;
    *(float *)(object + 0x44) = *(float *)(*(char **)(base + 0x2290) + 0x44);
    first = table + *(unsigned char *)(object + 0x42) * 4;
    second = table + *(unsigned char *)(object + 0x43) * 4;
    *(unsigned char *)(object + 0x40) = *(unsigned char *)(*(char **)(base + 0x2290) + 0x41);
    *(unsigned char *)(object + 0x41) = *(unsigned char *)(*(char **)(base + 0x2290) + 0x41);
    *(int *)(object + 0x58) = *(int *)(*(char **)first
        + *(unsigned char *)(*(char **)(base + 0x2290) + 0x41) * 4 + 0x1C);
    *(int *)(object + 0x5C) = *(int *)(*(char **)second
        + *(unsigned char *)(*(char **)(base + 0x2290) + 0x41) * 4 + 0x1C);
}
