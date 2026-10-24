#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x11D0D0-0x11D700. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl): the IOP reset (sceSifRebootIop,
 * func_0011D248, "rom0:UDNL "), the boot-time kernel patch/poke sequence
 * (func_0011D3C8) and the TLB setup (func_0011D4A0/func_0011D4E0,
 * "# TLB spad=0 kernel=1:%d ..."). Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE).
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);
extern int func_00114060(int, void *);
extern void func_00113AE0(void *);
extern void func_00117118(void *, void *, int, int);
extern int func_00119008();
extern int D_0012FCF0 NOT_SDA;
extern void func_00118E90(int arg0, void *arg1);
extern void *D_00154A40 NOT_SDA;
extern int D_00155080[];
extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;
extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;
extern void func_0011AA90(int, int, int, int, int, int, int);
extern void func_0011AA00(void);
extern int D_0012FD08 NOT_SDA;
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;
extern char D_00157E80[];
extern int D_0012FD9C;
extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;
extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;
extern void func_001153FC(void *, int, int);
extern int D_0012FD94;
extern int D_0012FDAC;
extern char D_00158140[];
extern int D_00158180;
extern int D_001581C0;
extern char D_00158528[];
extern int D_0012FDB4;
extern int func_0011CE70(int arg0, int arg1, int arg2, void *arg3);

typedef struct {
    u64 psize : 8;
    u64 dsize : 24;
    u64 dest : 32;
    s32 cid;
    u32 opt;
} SifCmdHeader;

typedef struct {
    SifCmdHeader header;
    s32 arglen;
    s32 mode;
    char arg[80];
} SifResetPacket;

typedef struct {
    u32 data;
    u32 addr;
    s32 size;
    s32 mode;
} SifDmaData;

extern SifResetPacket D_00158540;
extern void func_00118DA0(void);
extern u32 func_00118E70_u(u32 reg) __asm__("func_00118E70");
extern u32 func_00118E60(u32 reg, u32 val);
extern void func_0011AD70(void *p, s32 size);
extern u32 func_00118E20(SifDmaData *dma, s32 count);

/* sceSifResetIop: like sceSifRebootIop (func_0011D248) but with a caller
   -supplied argument string and reboot mode. Adapted from Lombyte (GPL-3.0)
   for PAL. */
s32 func_0011D0D0(const char *arg, s32 mode) {
    SifDmaData dma;
    u32 addr;
    s32 arglen;

    func_00118DA0();
    addr = func_00118E70_u(0x80000000);
    D_00158540.mode = mode;
    for (arglen = 0; arg[arglen] != 0; arglen++) {
        D_00158540.arg[arglen] = arg[arglen];
    }
    D_00158540.header.dest = 0;
    D_00158540.arglen = arglen;
    D_00158540.header.cid = 0x80000003;
    D_00158540.header.dsize = 0;
    D_00158540.header.psize = sizeof(SifResetPacket);
    dma.data = (u32)&D_00158540;
    dma.addr = addr;
    dma.size = sizeof(SifResetPacket);
    dma.mode = 0x44;
    func_0011AD70(&D_00158540, sizeof(SifResetPacket));
    func_00118E60(4, 0x40000);
    if (func_00118E20(&dma, 1)) {
        func_00118E60(4, 0x10000);
        func_00118E60(4, 0x20000);
        func_00118E60(0x80000002, 0);
        func_00118E60(0x80000000, 0);
        return 1;
    }
    return 0;
}

extern void func_00118EC0(void);

int func_0011D210(void) {
    if (func_00118E70(0x4) & 0x40000) {
        func_00118EC0();
        return 1;
    }
    return 0;
}

extern char D_00152A60[];   /* "rom0:UDNL " */
extern char D_00152A70[];
extern void func_0011A6C8();
extern void func_0011AE20(int);
extern void func_0011AFC0(void);

/* sceSifRebootIop: reset the IOP with "rom0:UDNL " + img (at most 80
   characters). Adapted from Lombyte (GPL-3.0) for PAL. */
int func_0011D248(char *img) {
    char *prefix = D_00152A60;
    char param[80];
    char *p;
    char *d;

    p = img;
    while (*p) {
        p++;
    }
    if ((unsigned int)(p + 11 - img) > 80) {
        func_0011A6C8(D_00152A70, img);
        return 0;
    }
    func_0011AE20(0);
    func_0011AFC0();
    d = param;
    while (*prefix) {
        *d++ = *prefix++;
    }
    while (*img) {
        *d++ = *img++;
    }
    *d = 0;
    return func_0011D0D0(param, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0011D358);

ASM_FUNC("asm/handwritten/core_text", func_0011D360);

/*
 * A word-at-a-time copy of nbytes (rounded down to words); returns 0.
 *
 * Exact under 2.9-ee. Retail leaves the loop branch's delay slot empty,
 * and that is 2.9-ee itself: it pads the short loop with nops in its own
 * output and leaves the bnez unfilled (as in libdma's func_001232A8).
 * Under 2.95.3 reorg fills the slot with the `addiu $4,$4,4` bump, 52
 * bytes against 56, which is what kept this a stub.
 */
int func_0011D370(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

ASM_FUNC("asm/handwritten/core_text", func_0011D3A8);

ASM_FUNC("asm/handwritten/core_text", func_0011D3B8);

extern int func_0011D3B8(int, int);
extern void func_0011D360(void *, void *, int);
extern int func_0011D3A8(int);
extern void func_00118D80(int);
extern int D_00130138[];
extern char D_0012FDB8[];
extern int D_00130130;

/*
 * Exact (it was a 2/0xC4 near-miss until the int/void fix below). Boot-time
 * hardware init: three explicit (addr, value) pokes through
 * func_0011D3B8, a func_0011D360 block copy of D_0012FDB8 (0x330 bytes)
 * to a fixed load address, an interrupt-disable/enable bracket, then a
 * loop over 5 more 8-byte table entries where func_0011D3A8 reads the
 * entry's current value back before func_0011D3B8 rewrites it. All
 * three callees are handwritten syscall wrappers (0x74/0x5A/0x5B).
 *
 * The loop counter has to be `unsigned int`, same tell as
 * func_0011DCB8 (see that comment): as `int` this compiler reverses
 * the up-count into a down-count-from-5 with `bgezl`, giving retail's
 * `addiu s2,zero,0x1`/`sltiu ...,0x8` only with `unsigned`. What's
 * left is a single instruction's destination register ($v0 vs $v1) on
 * the loop-continuation test -- tried caching `p->a` in a local
 * instead of re-reading it for both calls, which regressed badly
 * (605 words), and reordering the locals, which did nothing; the fix was
 * func_0011D3B8's return type (below).
 */
typedef struct { int a; int b; } D_00130138_pair;

extern void func_0011D3B8_v(int, int) __asm__("func_0011D3B8");

/* func_0011D3B8 returns nothing: declared int, its result register
   pushed the loop test to $v1. */
int func_0011D3C8(void) {
    int *t = D_00130138;
    D_00130138_pair *p;
    unsigned int i;
    int old;
    int r;

    func_0011D3B8_v(t[0], t[1]);
    func_0011D360((void *)0x80075000, D_0012FDB8, 0x330);
    func_00118D80(0);
    func_00118D80(2);
    func_0011D3B8_v(t[2], t[3]);
    func_0011D3B8_v(t[4], t[5]);

    p = (D_00130138_pair *)(t + 6);
    for (i = 3; i < 8; i++) {
        old = func_0011D3A8(p->a);
        func_0011D3B8_v(p->a, old);
        p++;
    }

    r = func_0011D3A8(3);
    D_00130130 = r;
    return r;
}

ASM_FUNC("asm/handwritten/core_text", func_0011D490);

extern int func_0011D4E0(void);
extern int func_00118EB0(void);

/* Set up the TLB for the machine's memory size: the 32 MB layout
   (func_0011D4E0) when func_00118EA0 reports 0x2000000, the kernel's own
   routine (func_00118EB0) otherwise. Returns its callees' values: retail
   keeps the frame and calls both, and 2.9-ee turns both arms into tail
   jumps (44 bytes against 64) when it is void. */
int func_0011D4A0(void) {
    if (func_00118EA0() == 0x2000000) {
        return func_0011D4E0();
    } else {
        return func_00118EB0();
    }
}

ASM_FUNC("asm/handwritten/core_text", func_0011D4E0);

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011D6D4);
