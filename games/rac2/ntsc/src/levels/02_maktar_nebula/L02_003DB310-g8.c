#include "include_asm.h"

extern int D_L02_001B04DC __attribute__((sda));

extern void func_L02_003DBA70();

extern void func_0029FCC0(char *arg0, int arg1, char *arg2);

/* Rebuild the four widget descriptions from the resident template. */
void func_L02_003DB9E0(char *arg0) {
    char buffer[16];

    func_0029FCC0(arg0, 2, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 4, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 8, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 6, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
}
