#include "include_asm.h"

/* The two compared keys live in small data, so this file is built with -G8.
   Their addresses differ from program to program, so they carry no level. */
extern char *D_001A8CFC;

extern int ref_L00_002D5570_00 __attribute__((sda));
extern int ref_L00_002D5570_01 __attribute__((sda));

extern void func_00339798(char *);
extern void func_00339818(char *);

int func_002D5570(char *a0)
{
    if (D_001A8CFC != 0) {
        if (a0 == (char *)&ref_L00_002D5570_00)
            func_00339798(D_001A8CFC + 0x3CAA8);
        else if (a0 == (char *)&ref_L00_002D5570_01)
            func_00339818(D_001A8CFC + 0x3CAA8);
    }
    return 0;
}
