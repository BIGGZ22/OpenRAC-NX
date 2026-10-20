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

extern void func_0029C680(void);

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

int func_002D2CB8(void)
{
    func_0027C540(0);
    func_0029C680();
    func_0027C660();
    return 0;
}

extern unsigned int D_00138344;
extern unsigned int D_001A7390 __attribute__((sda));
extern int D_001A79F0 __attribute__((sda));
extern char D_001F2840[];
typedef struct {
    char pad[0x1d4];
    unsigned short count;
    unsigned short unused;
    unsigned int maximum;
    unsigned int levels;
} Statistics_2D2CE8;
extern Statistics_2D2CE8 D_0018C750;
extern int func_0029C6F0(int);

int func_002D2CE8(void)
{
    int result = 0;
    unsigned int flags = D_00138344;
    if (flags & 0x10) {
        Statistics_2D2CE8 *stats = &D_0018C750;
        char *root;
        int value, status;
        if (stats->count < 0xffff) stats->count++;
        if (D_001A7390 > stats->maximum) stats->maximum = D_001A7390;
        stats->levels = (stats->levels | (1u << D_001A79F0)) | 0x80000000u;
        root = D_001F2840;
        value = *(int *)(*(char **)(root + 0x14) + 0xe0);
        if (value != 0) *(int *)(root + 0x18) = value;
        else {
            switch (*(int *)(root + 0x134)) {
            case 0: status = -1; goto finish;
            default: break;
            }
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x900) {
        Statistics_2D2CE8 *stats = &D_0018C750;
        if (stats->count < 0xffff) stats->count++;
        if (D_001A7390 > stats->maximum) stats->maximum = D_001A7390;
        stats->levels = (stats->levels | (1u << D_001A79F0)) | 0x80000000u;
        result = 1;
    } else func_0029C6F0(flags);
    return result;
}
