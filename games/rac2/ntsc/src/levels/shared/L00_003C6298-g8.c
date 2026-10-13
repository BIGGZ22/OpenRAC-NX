#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */

typedef struct Rac2Native_9cdc323a4d0c2fbd_Vector16 {
    unsigned int word_00;
    unsigned int word_04;
    unsigned int word_08;
    unsigned int word_0C;
} Rac2Native_9cdc323a4d0c2fbd_Vector16;

extern Rac2Native_9cdc323a4d0c2fbd_Vector16 D_L00_002472C0[];

extern unsigned int D_L00_001B0138 __attribute__((sda));

/* True when the shared mode word equals four. */
int func_L00_003CBBD0(void)
{
    return (D_L00_001B0138 ^ 4) == 0;
}

/* Clears the first word of five consecutive 16-byte records, from the last one down, then the mode word. */
void func_L00_003CBBE0(void)
{
    int i;

    for (i = 4; i >= 0; i--)
        D_L00_002472C0[i].word_00 = 0;

    D_L00_001B0138 = 0;
}
