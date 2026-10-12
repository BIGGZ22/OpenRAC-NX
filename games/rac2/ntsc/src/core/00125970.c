#include "common.h"
#include "structs.h"

extern int r1_func_0011BF80(void *path, int mode);

extern int r1_func_0011C5C0(int fd, void *buf, int len);

extern void r1_func_0011C208(int fd);

extern int r1_func_00112380(void *arg0);

extern char r1_D_00153130[];

/* rac1-decomp src/core/00121D18.c: func_00121D18 */
/* Read a small text file a byte at a time into a 0x100-byte stack buffer,
 * stopping at a NUL or the buffer limit, then strtol() the tail starting
 * 9 bytes before the end and compare it against a build number. Same
 * open/read/close triplet as r1_func_0012D2A0 above. */
int func_00125970(void) {
    int fd;
    char buf[0x100];
    char *p;
    unsigned int n;

    fd = r1_func_0011BF80(r1_D_00153130, 1);
    if (fd < 0) {
        return -1;
    }
    n = 0;
    p = buf;
    while (n < 0x100) {
        r1_func_0011C5C0(fd, p, 1);
        if (*p++ == 0) {
            break;
        }
        n++;
    }
    r1_func_0011C208(fd);
    return 0x1315670 < r1_func_00112380(&buf[n - 9]);
}

INCLUDE_ASM("asm/core", func_00125A10);

INCLUDE_ASM("asm/core", func_00125A20);

INCLUDE_ASM("asm/core", func_00125D94);

INCLUDE_ASM("asm/core", func_00125E54);
