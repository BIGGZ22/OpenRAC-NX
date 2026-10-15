#include "include_asm.h"

void func_00297730(char *destination, unsigned char *source, int argument)
{
    extern void func_00298170(void);
    extern void func_002977A8(char *, unsigned char *, int);
    extern void func_002979D8(char *, unsigned char *);
    extern int D_L00_001C93C8;
    func_00298170();
    if (D_L00_001C93C8) {
        if (source[0] & 1)
            func_002977A8(destination, source, argument);
        else
            func_002979D8(destination, source);
    }
}
