#include "include_asm.h"

/* func_00297F98 needs the small-data profile: it materialises the address of
   the pair of 16-byte records through $gp.  Its other globals stay absolute. */

extern int D_001A79F0;

extern int D_001C4F40 __attribute__((nosda));

extern int D_001A9428 __attribute__((sda));

typedef struct {
    int field00;
    int field04;
    int field08;
    int field0c;
} Quad16;

extern Quad16 D_00255ED0[];

int func_00297F98(int a0)
{
    if (D_001A79F0 == 20) {
        int value = D_001C4F40;

        return (int)((char *)&D_001A9428 + (value % 2) * 16);
    }
    return (int)&D_00255ED0[a0];
}
