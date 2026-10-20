#include "include_asm.h"

/*
 * Disassembled at 0x002cd220
 * Signature: int FUN_002cd220(void)
 * Behavior:
 *   - Saves RA on stack
 *   - Calls func_0027C540(0)
 *   - Calls func_0029C138()
 *   - Calls func_0027C660()
 *   - Restores RA from stack
 *   - Returns 0
 */
extern void func_0027C540(long);

extern void func_0027C660(void);

void func_0027C540(long);

void func_0027C660(void);

/* 
 * Function: func_002D2EE0
 * 
 * Disassembly summary:
 * - Saves return address on stack
 * - Calls func_0027C540(0) (with zero argument)
 * - Calls func_0029C7A0()
 * - Calls func_0027C660()
 * - Restores return address
 * - Returns 0
 *
 * Notes:
 * - The function follows standard MIPS ABI prologue/epilogue.
 * - Delay slots are filled with NOP or argument setup as seen in disassembly.
 * - No local variables used.
 * - All external function addresses come from the assigned externals list.
 */

extern void func_0027C540(long arg0);

/* External functions declared with exact aliases from task.externals */
extern void func_0027C540(long param_1);

extern void func_0029CED8(void);

long long func_002D5540(void)
{
    func_0027C540(0);
    func_0029CED8();
    func_0027C660();
    return 0;
}

/* func_002D5570 is matched in 002D5540-g8.c (-G8 small data). */

int func_002D55D8(void) {
    return 0;
}

/* func_002D55E0 uses small data in 002D5540-g8.c. */

extern void func_0029CB20(void);

int func_002D5718(void) {
    func_0027C540(0);
    func_0029CB20();
    func_0027C660();
    return 0;
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void func_00341CD8(char *, int);
extern void func_00341EB0(char *, char *);
extern char D_00261540[];

int func_002D5748(void)
{
    if (D_001A8CFC[0] != 0) {
        func_00341CD8(D_001A8CFC[0] + 0x3E430, 1);
        func_00341EB0(D_001A8CFC[0] + 0x3E430, D_00261540);
    }
    return 0;
}

extern int D_00138180[];
extern char D_001F2840[];
extern int func_0029CA70(int);
extern int func_003423B0(unsigned char *);
extern void func_002D5E98(int);

int func_002D57A0(void)
{
    int result = 0;
    int *state = D_00138180;
    int flags = state[0x1C4 / 4];
    if (flags & 0x10) {
        char *root = D_001F2840;
        int value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                status = -1;
                goto finish;
            default:
                break;
            }
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x900)
        result = 1;
    else {
        func_0029CA70(flags);
        if ((state[0x1C4 / 4] & 0x40) && D_001A8CFC[0]) {
            char *entry = (char *)func_003423B0((unsigned char *)(D_001A8CFC[0] + 0x3E1D8));
            struct { short unused; unsigned short id; int value; } request;
            request.id = *(unsigned short *)(entry + 8);
            request.value = *(int *)(entry + 12);
            func_002D5E98((int)&request);
        }
    }
    return result;
}


extern void func_0029CAB0(void);

int func_002D5870(void) {
    func_0027C540(0);
    func_0029CAB0();
    func_0027C660();
    return 0;
}

extern void func_003423A8(char *, int);
extern void func_00342380(char *, char *);
extern char D_00261590[];

int func_002D58A0(void)
{
    if (D_001A8CFC[0] != 0) {
        func_003423A8(D_001A8CFC[0] + 0x3E1D8, 1);
        func_00342380(D_001A8CFC[0] + 0x3E1D8, D_00261590);
    }
    return 0;
}

extern int D_00138344;
extern void func_0029C3A0(int);

int func_002D58F8(void) {
    func_0029C3A0(D_00138344);
    return 0;
}

extern void func_0029C370(void);

int func_002D5920(void) {
    func_0027C540(0);
    func_0029C370();
    func_0027C660();
    return 0;
}

extern int D_001C51D4;

int func_002D5950(void) {
    D_001C51D4 = -1;
    return 0;
}

int func_002D5968(void) {
    return 0;
}

extern int D_001A79F0 __attribute__((sda));
extern volatile unsigned char D_001A7C01 __attribute__((sda));
extern int D_001C51D0;
extern int D_001C4FA0[];
extern void func_0027D330(int);
extern void func_002C9C60(float);
extern int func_002E5E58(int, unsigned int, void *);
extern int func_0026F7C8(void);
extern int func_0026F7D0(void);
extern void func_00288CB0(int, int);
extern void func_002B49E0(int, int, int, int, int);
extern int func_0029C410(int);

int func_002D5970(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        char *root;
        int value;
        func_0027D330(4);
        func_002C9C60(4.0f);
        root = D_001F2840;
        value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                result = -1;
                goto finish;
            default:
                break;
            }
        }
        result = 0;
    } else if (flags & 0xD00) {
        result = 1;
        D_001C51D0 = D_001A79F0;
    } else if (flags & 0x40) {
        int *base = D_001C4FA0;
        if (D_001A79F0 != base[0x230 / 4]) {
            result = 1;
            func_002E5E58(4, 0, 0);
            if (base[0x230 / 4] != 0 && D_001A7C01 == 0 && base[0x230 / 4] == 2) {
                func_002B49E0(6, 2, 1, 0x19, 0);
            } else if (func_0026F7C8() && func_0026F7D0()) {
                func_00288CB0(D_001C4FA0[0x230 / 4], 1);
            } else {
                func_002B49E0(6, 2, 1, D_001C4FA0[0x230 / 4], 0);
            }
        } else {
            result = 1;
            func_002E5E58(4, 0, 0);
            func_0027D330(4);
        }
    } else {
        func_0029C410(flags);
    }
finish:
    return result;
}

extern void func_0029C3E0(void);

int func_002D5B08(void) {
    func_0027C540(0);
    func_0029C3E0();
    func_0027C660();
    return 0;
}

extern void func_002D5B58(int, int, int *);

void func_002D5B38(int first, int second) {
    int result[4];
    result[0] = 0;
    func_002D5B58(first, second, result);
}

INCLUDE_ASM("asm/game", func_002D5B58);

void func_002D5E38(char *object, int arg, int *result)
{
    typedef struct { int first, value, third; } Func002D5E38Row;
    Func002D5E38Row *rows = *(Func002D5E38Row **)(object + 0x34);
    int index = *(int *)(object + 0x40);
    func_002D5B58(arg, rows[index].value, result);
}

void func_002D5E70(char *object, int *result) {
    func_002D5B58(*(short *)(object + 2), *(int *)(object + 4), result);
}
