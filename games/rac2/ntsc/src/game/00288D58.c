#include "include_asm.h"

/* Frozen Batch A A16-sda-byte-16; acknowledged claim 3b1980407c8765dd4baf122f505692e49c411cbff6089e896e8b3921ce208b41. */
typedef unsigned char BatchA_A16_u8;

extern BatchA_A16_u8 BatchA_A16_BatchA_D001A7A0A[] __attribute__((sda));

int func_00288D58(void) { return BatchA_A16_BatchA_D001A7A0A[0] != 0; }

extern int D_001A7B8C __attribute__((sda));

void func_00288D68(void) {
    D_001A7B8C = 0;
}
