#include "common.h"
#include "structs.h"

extern void r1_func_00118C90(int);

extern int r1_func_0011B6B8(void *);

extern int r1_D_00132EAC;

extern int func_00127B88(int, int *, int *);

extern char r1_D_00159B00[];

extern char r1_D_0015B0C0[];

extern int r1_D_00132EA8;

extern void func_00127B40(int);

extern int *r1_D_00159B28;

extern int *r1_D_00159B2C;

extern int *r1_D_00159B30;

extern int r1_D_00159B80;

extern int r1_func_0011B4C8();

extern int r1_func_00118CC0(int);

extern char r1_D_0015A000[];

extern int r1_func_0011AD70(void *, int);

extern int r1_D_00159B80_a[] __asm__("r1_D_00159B80");

extern char r1_D_00159BB0[];

extern void *r1_func_00116B00(void *, const void *, int);

extern char r1_D_0015B108[];

extern int r1_D_0015B180;

extern void r1_func_0011A6C8();

extern int r1_func_0011AE20(int);

extern int r1_func_0011B2F8(void *, int, int);

extern char r1_D_0015B130[];

extern char r1_D_001535D8[];

extern char r1_D_001535F0[];

extern char r1_D_00153618[];

extern char r1_D_00153640[];

extern int r1_D_0015B600[];

extern char r1_D_0015B580[];

extern void r1_func_001138B8(int) __attribute__((noreturn));

 /* exit */
extern void func_00128898(void *, ...);

extern int func_00128440(int);

/* rac1-decomp src/core/001236F0.c: func_00123F30 */
/* sceMcSync(mode, &cmd, &result): polls the pending memory card command
   (or waits for it when mode is 0) and hands back its result. Exact
   only under 2.9-ee: 2.95.3's gcse leaves two extra %hi copies. */
int func_00127B88(int mode, int *cmd, int *result) {
    int r;

    if (r1_D_00132EA8 == 0) {
        return -1;
    }
    r = r1_func_0011B6B8(r1_D_00159B00);
    if (mode == 0 && r != 0) {
        while (r1_func_0011B6B8(r1_D_00159B00) != 0) {
            func_00127B40(0x3C);
        }
        r = 0;
    }
    r = (r == 0);
    if (cmd != 0) {
        *cmd = r1_D_00132EA8;
    }
    if (r != 0) {
        r1_D_00132EA8 = 0;
        if (result != 0) {
            *result = *(int *)r1_D_0015B0C0;
        }
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00124010 */
void func_00127C68(int arg0) {
    char *p = (char *)(arg0 | 0x20000000);
    if (r1_D_00159B28 != 0) *r1_D_00159B28 = *(int *)(p + 0x0);
    if (r1_D_00159B2C != 0) *r1_D_00159B2C = *(int *)(p + 0x4);
    if (r1_D_00159B30 != 0) *r1_D_00159B30 = *(int *)(p + 0x90);
}

/* rac1-decomp src/core/001236F0.c: func_00124068 */
/* sceMcGetInfo-style RPC 1 (libmc): asks for the card type, free space
   and format state (each only when its result pointer is given); the
   reply lands in r1_D_0015A000 and func_00127C68 copies it out. */
int func_00127CC0(int port, int slot, int *type, int *free, int *format) {
    int r;

    {
        char *cd = r1_D_00159B00;
        if (*(int *)(cd + 0x24) == 0) {
            return -100;
        }
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    r1_D_00159B80_a[1] = port;
    r1_D_00159B80_a[2] = slot;
    r1_D_00159B80_a[7] = (int)r1_D_0015A000;
    if (type != 0) {
        r1_D_00159B80_a[5] = 1;
    } else {
        r1_D_00159B80_a[5] = 0;
    }
    if (free != 0) {
        r1_D_00159B80_a[4] = 1;
    } else {
        r1_D_00159B80_a[4] = 0;
    }
    if (format != 0) {
        r1_D_00159B80_a[3] = 1;
    } else {
        r1_D_00159B80_a[3] = 0;
    }
    r1_D_00159B28 = type;
    r1_D_00159B2C = free;
    r1_D_00159B30 = format;
    r1_func_0011AD70(r1_D_0015A000, 0xC0);
    r = r1_func_0011B4C8(r1_D_00159B00, 1, 1, r1_D_00159B80_a, 0x30, r1_D_0015B0C0, 4, func_00127C68, r1_D_0015A000);
    if (r == 0) {
        r1_D_00132EA8 = 1;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

INCLUDE_ASM("asm/core", func_00127E40);

INCLUDE_ASM("asm/core", func_00127F90);

/* rac1-decomp src/core/001236F0.c: func_00124410 */
/* sceMcDelete(port, slot, fname): rejects a null/empty fname (SignalSema,
   return -210), else strncpy's it into the shared sifParamFname
   descriptor's name field (r1_D_00159BB0+0x14), fills in port/slot/flags
   and force-terminates at the max length, then runs RPC 0xF (delete)
   through cd_base. Nonzero (queued) result SignalSemas and returns the
   handle; 0 records the command and returns 0 -- the same idiom as
   func_00127888/sceMcRead. */
int func_00128068(int port, int slot, char *fname) {
    char *cd = r1_D_00159B00;
    char *fp;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    if (fname == 0 || *fname == 0) {
        r1_func_00118C90(r1_D_00132EAC);
        return -210;
    }
    r1_func_00116B00(r1_D_00159BB0 + 0x14, fname, 0x3FF);
    fp = r1_D_00159BB0;
    *(int *)(fp + 0x0) = port;
    *(int *)(fp + 0x4) = slot;
    fp[0x413] = 0;
    *(int *)(fp + 0x8) = 0;
    r = r1_func_0011B4C8(cd, 0xF, 1, fp, 0x414, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 0xF;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_00124528 */
/* RPC 0x11, the same shape with two request words. */
int func_00128180(int arg0, int arg1) {
    char *cd = r1_D_00159B00;
    int *buf;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    buf = &r1_D_00159B80;
    buf[1] = arg0;
    buf[2] = arg1;
    r = r1_func_0011B4C8(cd, 0x11, 1, buf, 0x30, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 0x11;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}

/* rac1-decomp src/core/001236F0.c: func_001245F8 */
int func_00128250(void) {
    r1_func_0011B4C8(r1_D_0015B108, 0x80000963, 0, &r1_D_0015B180, 0x400,
                  &r1_D_0015B180, 0x400, 0, 0);
    return r1_D_0015B180;
}

/* rac1-decomp src/core/001236F0.c: func_00124650 */
/* Bind the two IOP servers 0x80000900 and 0x8000091B (retrying with a
   busy wait; exit(-1) if binding fails), check the server version
   (func_00128250: major 2), clear the 16-word table r1_D_0015B600 and read
   the configuration into r1_D_0015B580. Returns 1, or 0 on a version
   mismatch. */
int func_001282A8(void) {
    int i;
    int v;

    r1_func_0011AE20(0);
    for (;;) {
        if (r1_func_0011B2F8(r1_D_0015B108, 0x80000900, 0) < 0) {
            func_00128898(r1_D_001535D8);
            r1_func_001138B8(-1);
        }
        if (*(int *)(r1_D_0015B108 + 0x24) != 0) {
            break;
        }
        i = 0x10000;
        while (i--) {
        }
    }
    for (;;) {
        if (r1_func_0011B2F8(r1_D_0015B130, 0x8000091B, 0) < 0) {
            func_00128898(r1_D_001535D8);
            r1_func_001138B8(-1);
        }
        if (*(int *)(r1_D_0015B130 + 0x24) != 0) {
            break;
        }
        i = 0x10000;
        while (i--) {
        }
    }
    v = func_00128250();
    if (v >> 8 != 2) {
        r1_func_0011A6C8(r1_D_001535F0);
        r1_func_0011A6C8(r1_D_00153618, 2, 0, v >> 8, v & 0xFF);
        return 0;
    }
    for (i = 15; i >= 0; i--) {
        r1_D_0015B600[i] = 0;
    }
    func_00128898(r1_D_00153640, r1_D_0015B580);
    func_00128440((int)r1_D_0015B580);
    return 1;
}
