#include "include_asm.h"

typedef struct Rac2Native_428d1e2cc4aa1f21_Keyed {
    unsigned char unknown_00[28];
    char *owner_1c;
} Rac2Native_428d1e2cc4aa1f21_Keyed;

extern char *D_L00_001B8E10;

extern Rac2Native_428d1e2cc4aa1f21_Keyed *D_L00_001B1844;

int func_L00_00424978(char *a0)
{
    char *p = D_L00_001B8E10;
    char *q;
    char *v;

    if (*(short *)(p + 134) != 0) {
        return -1;
    }
    q = *(char **)(p + 112);
    v = *(char **)(q + 576);
    if (v == 0) {
        *(char **)(q + 576) = a0;
        return 1;
    }
    if (v == a0) {
        return 1;
    }
    if (*(unsigned char *)(D_L00_001B1844[*(short *)(v + 132)].owner_1c + 28) <
        *(unsigned char *)(D_L00_001B1844[*(short *)(a0 + 132)].owner_1c + 28)) {
        *(char **)(q + 576) = a0;
        return 1;
    }
    return 0;
}
