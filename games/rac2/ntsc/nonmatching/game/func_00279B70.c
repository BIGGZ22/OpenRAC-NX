#include "common.h"

/* Named-global candidate: 92 bytes, entry scheduling/register allocation differs. */
extern char D_0019B4A8[];
extern int D_001A79F0;

/* Takes a three-bit field out of the byte the level's table holds at the given index (the
   high nibble's low three bits for an odd index, the low three bits for an even one), looks
   that field up in a table, and scales the byte found there: times the scale, over 100. */
int func_00279B70(u8 *a0, int a1, int a2)
{
    int level = D_001A79F0;
    u8 *p = D_0019B4A8 + ((a2 >> 1) + (level << 10));
    int c = p[0];

    return (a0[(a2 & 1) ? ((c >> 4) & 7) : (c & 7)] * a1) / 100;
}

