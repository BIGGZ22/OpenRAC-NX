#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;

extern u8 *D_L26_001B1C1C __attribute__((sda));

extern unsigned char D_L26_001FA6E0[];

/* Walks the record's live list and flags every entry whose id matches. */
INCLUDE_ASM("asm/levels", func_L26_003DC128);
