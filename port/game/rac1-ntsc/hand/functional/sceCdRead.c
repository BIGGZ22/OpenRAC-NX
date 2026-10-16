/* Carried from the PAL decompilation's nonmatching/core/func_00121750.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
extern int func_00120B98(void);
extern int func_00120A28(int);
extern int D_001312D0;
extern int func_00120B98(void);
extern void func_0011A3C8();
extern int func_0011B1C8();
extern char D_00132490[];
extern int D_001312E8;
extern int D_001312EC;

/* sceCdRead: queue one CD/DVD read request (lba, sectors, buffer, mode)
 * in the request block at D_001313C0 and start it through the RPC
 * func_0011B1C8. Returns 1 when the RPC was accepted, 0 otherwise.
 * Sector size by mode->datapattern: 1 -> 0x918, 2 -> 0x924, else 0x800. */
extern int D_001312F4 NOT_SDA;
extern char D_001313C0[];
extern char D_001323C0[];
extern int D_00132480 NOT_SDA;
extern char D_00152FC8[];
extern char D_00152FE0[];
extern volatile int D_00131314;
extern int D_001312F0;
extern void func_0011AA70(void *, int);
extern void func_00118990(int);
extern void func_00120778();

int sceCdRead(int lba, int sectors, void *buf, unsigned char *mode) {
    char *req = D_001313C0;
    int size;
    int r;

    if ((D_001312F4 & 1) == 0) {
        if (func_00120B98() == 6) {
            return 0;
        }
    }
    if (func_00120A28(4) == 0) {
        return 0;
    }

    *(int *)(req + 0x0) = lba;
    *(int *)(req + 0x4) = sectors;
    *(void **)(req + 0x8) = buf;
    req[0xC] = mode[0];
    req[0xD] = mode[1];
    req[0xE] = mode[2];
    *(void **)(req + 0x10) = D_001323C0;
    *(void **)(req + 0x14) = &D_00132480;

    if (mode[2] == 1) {
        size = sectors * 0x918;
    } else if (mode[2] == 2) {
        size = sectors * 0x924;
    } else {
        size = sectors << 11;
    }

    D_00132480 = 0;
    if ((D_001312F4 & 2) == 0) {
        func_0011AA70(buf, size);
    }
    func_0011AA70(D_001323C0, 0x90);
    func_0011AA70(req, 0x18);
    func_0011AA70(&D_00132480, 4);
    if (D_001312D0 > 0) {
        func_0011A3C8(D_00152FC8);
    }

    D_00131314 = 1;
    D_001312F0 = 1;
    r = func_0011B1C8(D_00132490, 1, 1, req, 0x18, 0, 0,
                      (void *)((char *)func_00120778 + 0x10), D_001323C0);
    if (r >= 0) {
        if (D_001312D0 > 0) {
            func_0011A3C8(D_00152FE0);
        }
        return 1;
    }
    D_00131314 = 0;
    D_001312F0 = 0;
    func_00118990(D_001312E8);
    return 0;
}
