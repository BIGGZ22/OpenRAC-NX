#include "include_asm.h"

char *func_002DF750(char *record)
{
    extern void func_002E0408(char *);
    extern void func_002E0558(char *);
    if (*(short *)record == 0) {
        func_002E0408(record);
        record += 0x20;
    } else if (*(short *)record == 1) {
        func_002E0558(record);
        record += 0x30;
    }
    return record;
}

