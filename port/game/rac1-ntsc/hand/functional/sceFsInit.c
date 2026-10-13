/* Carried from the PAL decompilation's nonmatching/core/func_0011BCB0.c (games/rac1/pal) through the version
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
extern int func_0011D660(void);
extern void func_0011D6A8(void);
extern int func_0011AFF8(void *, int, int);
extern int func_001189B0(int);
extern int D_0012FCA0;
extern int D_0012FCA4;
extern char D_00157D80[];
extern s32 func_00118990(s32);
extern s32 func_0011B1C8(void *, s32, s32, void *, s32, void *, s32, void *, void *);

/* CD/DVD init step: install the two RPC-side handlers, bind the RPC server
 * at 0x80000001 (retrying until it has an address), clear the 32-slot table
 * D_00157D80, then send command 0xFF and copy back the reply word and the
 * flag that says whether the reply was 2. Returns 0 on success, -1 when the
 * bind failed, 0xFFFEFFFF when the RPC failed. */
extern char D_00158000[];
extern char D_00157FC0[];
extern char D_00157F80[];
extern char D_001574C0[];
extern char D_00157500[];
extern char *D_00156840[];
extern char D_00157FA8[];
extern int D_0012FC94;
extern int D_0012FC98;
extern void func_0011B568();
extern void func_0011B980();

s32 sceFsInit(void) {
    s32 r;
    s32 t;
    s32 i;
    char *p;

    func_0011AB20(0);
    *(s32 *)D_00158000 = 0;
    *(s32 *)(D_00158000 + 4) = 0;
    r = func_0011D660();
    func_0011A738(0x80000011, (int)func_0011B568, (int)D_00157FC0);
    func_0011A738(0x80000013, (int)func_0011B980, (int)D_00158000);
    if (r != 0) {
        func_0011D6A8();
    }
    for (;;) {
        r = func_0011AFF8(D_00157F80, 0x80000001, 0);
        if (r < 0) {
            return -1;
        }
        if (*(s32 *)(D_00157F80 + 0x24) != 0) {
            break;
        }
        for (i = 0x100000; i != -1; i--) {
        }
    }
    func_0011B410();
    func_001189B0(D_0012FCA0);
    for (p = D_00157D80; p < D_00157D80 + 0x200; p += 0x10) {
        *(s32 *)(p + 4) = 0;
    }
    func_00118990(D_0012FCA0);
    D_00156840[0] = D_00157500;
    D_00156840[1] = D_00157500 + 0x440;
    r = func_0011B1C8(D_00157F80, 0xFF, 0, D_00156840, 8, D_001574C0, 8, 0, 0);
    if (r < 0) {
        return (s32)0xFFFEFFFF;
    }
    *(s32 *)D_00157FA8 = *(s32 *)((u32)D_001574C0 | 0x20000000);
    t = *(s32 *)((u32)(D_001574C0 + 4) | 0x20000000);
    D_0012FC94 = 1;
    D_0012FC98 = (t == 2);
    return 0;
}
