#include "common.h"
#include "structs.h"

extern void r1_func_0011A6C8();

/*
 * Fill the 14-byte rom0:ROMVER record at r1_D_001331D8 once (open
 * r1_D_00153D00 "rom0:ROMVER" read-only, read 0xE bytes, close; failures are
 * reported through r1_func_0011A6C8) and hand the buffer back either way.
 * (The second r1_func_0011A6C8 gets only the format string: $a1 still holds
 * the buffer from the read call and retail never resets it, which is why
 * r1_func_0011A6C8 stays unprototyped here.)
 *
 * Exact under 2.9-ee. It was blocked under 2.95.3 (152/160 against 156):
 * that compiler's GCSE copies the kept %hi(r1_D_001331D8) into a saved
 * register (`move $19,$2`), where 2.9-ee loads the lui straight into $17
 * as retail does. r1_D_001331D8 is `signed char` (the declaration below),
 * hence the cast on the return.
 */
extern int r1_func_0011BF80(void *, int);

extern int r1_func_0011C5C0(int, void *, int);

extern void r1_func_0011C208(int);

extern signed char r1_D_001331D8[];

extern char r1_D_00153D00[], r1_D_00153D10[], r1_D_00153D28[];

extern char *func_00131540(void);

/* rac1-decomp src/core/0012CC90.c: func_0012D2A0 */
char *func_00131540(void) {
    int fd;
    if (r1_D_001331D8[0] == 0) {
        fd = r1_func_0011BF80(r1_D_00153D00, 1);
        if (fd == -1) {
            r1_func_0011A6C8(r1_D_00153D10, r1_D_001331D8);
        }
        if (r1_func_0011C5C0(fd, r1_D_001331D8, 0xE) == -1) {
            r1_func_0011A6C8(r1_D_00153D28);
        }
        r1_func_0011C208(fd);
    }
    return (char *)r1_D_001331D8;
}
