#include "common.h"
#include "structs.h"

extern void r1_func_00118C90(int);

extern int r1_D_00132EAC;

extern char r1_D_00159B00[];

extern int r1_D_00159B80;

extern char r1_D_0015B0C0[];

extern int r1_func_0011B4C8();

extern int r1_func_00118CC0(int);

extern int r1_D_00132EA8;

extern char r1_D_0015A000[];

extern int r1_func_0011AD70(void *, int);

extern void func_001277F8(void *);

extern void r1_func_00118D80(int);

struct McClient {
    unsigned char reserved_00[0x24];
    int is_initialized;
};

struct McWriteReq {
    int file_descriptor;
    int port;
    int slot;
    int size;
    int offset;
    unsigned int origin;
    int buffer;
    int reserved_1c;
    unsigned char data[0x10];
};

extern struct McClient r1_D_00159B00_c __asm__("r1_D_00159B00");

extern struct McWriteReq r1_D_00159B80_r __asm__("r1_D_00159B80");

extern void r1_func_00118B20(int, void *, int);

extern int r1_func_00118BE0(void);

extern int r1_func_00118C00(void);

extern void r1_func_00123EC0(int, unsigned short, void *);

extern void func_00127B40(int);

/* rac1-decomp src/core/001236F0.c: func_00123C30 */
int func_00127888(int fd, void *buf, int nbyte) {
    char *cd = r1_D_00159B00;
    int *p;
    char *next;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    p = &r1_D_00159B80;
    next = r1_D_0015A000;
    p[0] = fd;
    p[7] = (int)next;
    r1_func_0011AD70((void *)(p[6] = (int)buf), p[3] = nbyte);
    r1_func_0011AD70(next, 0xC0);
    r = r1_func_0011B4C8(cd, 5, 1, p, 0x30, r1_D_0015B0C0, 4, func_001277F8, next);
    if (r == 0) {
        r1_D_00132EA8 = 5;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00123D48 */
/* sceMcWrite: copies the unaligned lead of the buffer into the request, sends RPC 6. Adapted from Lombyte (MIT) for PAL: src/sdk/storage/memory_card/sce_mc_write.c, sceMcWrite. */
int func_001279A0(int fd, unsigned char *buffer, int size) {
    int rpc_result;
    unsigned int i;
    int root_off;
    int base;

    if (r1_D_00159B00_c.is_initialized == 0) {
        return -0x64;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -0xC8;
    }
    r1_D_00159B80_r.file_descriptor = fd;
    if (size < 0x11) {
        r1_D_00159B80_r.origin = size;
        r1_D_00159B80_r.size = 0;
        r1_D_00159B80_r.buffer = 0;
    } else {
        base = (int)buffer - 0x10;
        root_off = ((int)(buffer - 1) & 0xFFFFFFF0) - base;
        r1_D_00159B80_r.size = size - root_off;
        r1_D_00159B80_r.origin = root_off;
        r1_D_00159B80_r.buffer = (int)(buffer + root_off);
    }
    for (i = 0; i < r1_D_00159B80_r.origin; i++) {
        r1_D_00159B80_r.data[i] = buffer[i];
    }
    r1_func_00118D80(0);
    rpc_result = r1_func_0011B4C8(&r1_D_00159B00_c, 6, 1, &r1_D_00159B80_r, 0x30, r1_D_0015B0C0, 4, 0, 0);
    if (rpc_result == 0) {
        r1_D_00132EA8 = 6;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return rpc_result;
}

INCLUDE_ASM("asm/core", func_00127B18);

/* rac1-decomp src/core/001236F0.c: func_00123EE8 */
/* mcDelayThread (libmc): sleep this thread until an alarm of `time`
   h-lines wakes it: SetAlarm (r1_func_00118B20) with mcHearAlarm
   (r1_func_00123EC0) on GetThreadId(), then SleepThread(). */
void func_00127B40(int time) {
    r1_func_00118B20((unsigned short)time, r1_func_00123EC0, r1_func_00118BE0());
    r1_func_00118C00();
}
