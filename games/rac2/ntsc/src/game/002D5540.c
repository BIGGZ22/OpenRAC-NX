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

INCLUDE_ASM("asm/game", func_002D55E0);

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

INCLUDE_ASM("asm/game", func_002D57A0);

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

INCLUDE_ASM("asm/game", func_002D5970);

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
