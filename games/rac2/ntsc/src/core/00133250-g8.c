#include "common.h"

extern int D_001A748C __attribute__((sda));
extern int D_001A74C8 __attribute__((sda));

extern int func_00132028(void);
extern int func_00132C48(int, int, void *);
extern void func_00133580(int);

int func_00133250(int arg0, int arg1, int arg2, int arg3) {
    int args[4];

    if (D_001A748C == 1) {
        return 0;
    }
    if (D_001A74C8 != 0) {
        while (func_00132028() != 0) {
        }
    }
    func_00133580(0);
    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    D_001A748C = func_00132C48(0x2A, 0x10, args);
}
