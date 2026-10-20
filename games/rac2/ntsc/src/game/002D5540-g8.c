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

extern int D_001A7424;
extern int D_001A7420;
extern int D_00138180[];
extern char D_001F2840[];
extern char *ref_L00_002D55E0_00 __attribute__((sda));
extern int func_0029CAE0(int);
extern int func_00341E58(char *);
extern void func_002D5E98(int);

int func_002D55E0(void)
{
    int result = 0;
    int *state = D_00138180;
    int flags;
    D_001A7424 = (D_001A7424 | 2) & ~4;
    flags = state[0x1c4 / 4];
    if (flags & 0x10) {
        char *root = D_001F2840;
        int value = *(int *)(*(char **)(root + 0x14) + 0xe0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0: status = -1; goto finish;
            default: break;
            }
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x900) result = 1;
    else {
        func_0029CAE0(flags);
        if ((state[0x1c4 / 4] & 0x40) && ref_L00_002D55E0_00) {
            char *entry = (char *)func_00341E58(ref_L00_002D55E0_00 + 0x3e430);
            struct { short unused; short id; int value; } request;
            request.id = *(unsigned short *)(entry + 8);
            request.value = *(int *)(entry + 12);
            if ((D_001A7420 == 6 || D_001A7420 == 13) && request.id == 5) {
                D_001A7420 = 12;
                D_001A7424 &= ~2;
            }
            func_002D5E98((int)&request);
        }
    }
    return result;
}
