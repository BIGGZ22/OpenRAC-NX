#include "common.h"
#include "structs.h"

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

extern SifResetPacket r1_D_00158540;

extern void r1_func_00118DA0(void);

extern u32 r1_func_00118E70_u(u32 reg) __asm__("r1_func_00118E70");

extern u32 r1_func_00118E60(u32 reg, u32 val);

extern void r1_func_0011AD70(void *p, s32 size);

extern u32 r1_func_00118E20(SifDmaData *dma, s32 count);

extern int r1_func_00118E70(int);

extern void r1_func_00118EC0(void);

extern char r1_D_00152A60[];

   /* "rom0:UDNL " */
extern char r1_D_00152A70[];

extern void r1_func_0011A6C8();

extern void r1_func_0011AE20(int);

extern void r1_func_0011AFC0(void);

extern int r1_func_0011D3B8(int, int);

extern void r1_func_0011D360(void *, void *, int);

extern int r1_func_0011D3A8(int);

extern void r1_func_00118D80(int);

extern int r1_D_00130138[];

extern char r1_D_0012FDB8[];

extern int r1_D_00130130;

/*
 * Exact (it was a 2/0xC4 near-miss until the int/void fix below). Boot-time
 * hardware init: three explicit (addr, value) pokes through
 * r1_func_0011D3B8, a r1_func_0011D360 block copy of r1_D_0012FDB8 (0x330 bytes)
 * to a fixed load address, an interrupt-disable/enable bracket, then a
 * loop over 5 more 8-byte table entries where r1_func_0011D3A8 reads the
 * entry's current value back before r1_func_0011D3B8 rewrites it. All
 * three callees are handwritten syscall wrappers (0x74/0x5A/0x5B).
 *
 * The loop counter has to be `unsigned int`, same tell as
 * r1_func_0011DCB8 (see that comment): as `int` this compiler reverses
 * the up-count into a down-count-from-5 with `bgezl`, giving retail's
 * `addiu s2,zero,0x1`/`sltiu ...,0x8` only with `unsigned`. What's
 * left is a single instruction's destination register ($v0 vs $v1) on
 * the loop-continuation test -- tried caching `p->a` in a local
 * instead of re-reading it for both calls, which regressed badly
 * (605 words), and reordering the locals, which did nothing; the fix was
 * r1_func_0011D3B8's return type (below).
 */
typedef struct { int a; int b; } r1_D_00130138_pair;

extern void r1_func_0011D3B8_v(int, int) __asm__("r1_func_0011D3B8");

extern int r1_func_00118EA0(void);

extern int r1_func_0011D4E0(void);

extern int r1_func_00118EB0(void);

/* rac1-decomp src/core/0011D0D0.c: func_0011D0D0 */
/* sceSifResetIop: like sceSifRebootIop (func_0011EED8) but with a caller
   -supplied argument string and reboot mode. Adapted from Lombyte (MIT)
   for PAL. */
s32 func_0011ED60(const char *arg, s32 mode) {
    SifDmaData dma;
    u32 addr;
    s32 arglen;

    r1_func_00118DA0();
    addr = r1_func_00118E70_u(0x80000000);
    r1_D_00158540.mode = mode;
    for (arglen = 0; arg[arglen] != 0; arglen++) {
        r1_D_00158540.arg[arglen] = arg[arglen];
    }
    r1_D_00158540.header.dest = 0;
    r1_D_00158540.arglen = arglen;
    r1_D_00158540.header.cid = 0x80000003;
    r1_D_00158540.header.dsize = 0;
    r1_D_00158540.header.psize = sizeof(SifResetPacket);
    dma.data = (u32)&r1_D_00158540;
    dma.addr = addr;
    dma.size = sizeof(SifResetPacket);
    dma.mode = 0x44;
    r1_func_0011AD70(&r1_D_00158540, sizeof(SifResetPacket));
    r1_func_00118E60(4, 0x40000);
    if (r1_func_00118E20(&dma, 1)) {
        r1_func_00118E60(4, 0x10000);
        r1_func_00118E60(4, 0x20000);
        r1_func_00118E60(0x80000002, 0);
        r1_func_00118E60(0x80000000, 0);
        return 1;
    }
    return 0;
}

/* rac1-decomp src/core/0011D0D0.c: func_0011D210 */
int func_0011EEA0(void) {
    if (r1_func_00118E70(0x4) & 0x40000) {
        r1_func_00118EC0();
        return 1;
    }
    return 0;
}

/* rac1-decomp src/core/0011D0D0.c: func_0011D248 */
/* sceSifRebootIop: reset the IOP with "rom0:UDNL " + img (at most 80
   characters). Adapted from Lombyte (MIT) for PAL. */
int func_0011EED8(char *img) {
    char *prefix = r1_D_00152A60;
    char param[80];
    char *p;
    char *d;

    p = img;
    while (*p) {
        p++;
    }
    if ((unsigned int)(p + 11 - img) > 80) {
        r1_func_0011A6C8(r1_D_00152A70, img);
        return 0;
    }
    r1_func_0011AE20(0);
    r1_func_0011AFC0();
    d = param;
    while (*prefix) {
        *d++ = *prefix++;
    }
    while (*img) {
        *d++ = *img++;
    }
    *d = 0;
    return func_0011ED60(param, 0);
}

INCLUDE_ASM("asm/core", func_0011EFE8);

ASM_FUNC("asm/core", func_0011EFF0);

/* rac1-decomp src/core/0011D0D0.c: func_0011D370 */
/*
 * A word-at-a-time copy of nbytes (rounded down to words); returns 0.
 *
 * Exact under 2.9-ee. Retail leaves the loop branch's delay slot empty,
 * and that is 2.9-ee itself: it pads the short loop with nops in its own
 * output and leaves the bnez unfilled (as in libdma's r1_func_001232A8).
 * Under 2.95.3 reorg fills the slot with the `addiu $4,$4,4` bump, 52
 * bytes against 56, which is what kept this a stub.
 */
int func_0011F000(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

ASM_FUNC("asm/core", func_0011F038);

ASM_FUNC("asm/core", func_0011F048);

/* rac1-decomp src/core/0011D0D0.c: func_0011D3C8 */
/* r1_func_0011D3B8 returns nothing: declared int, its result register
   pushed the loop test to $v1. */
int func_0011F058(void) {
    int *t = r1_D_00130138;
    r1_D_00130138_pair *p;
    unsigned int i;
    int old;
    int r;

    r1_func_0011D3B8_v(t[0], t[1]);
    r1_func_0011D360((void *)0x80075000, r1_D_0012FDB8, 0x330);
    r1_func_00118D80(0);
    r1_func_00118D80(2);
    r1_func_0011D3B8_v(t[2], t[3]);
    r1_func_0011D3B8_v(t[4], t[5]);

    p = (r1_D_00130138_pair *)(t + 6);
    for (i = 3; i < 8; i++) {
        old = r1_func_0011D3A8(p->a);
        r1_func_0011D3B8_v(p->a, old);
        p++;
    }

    r = r1_func_0011D3A8(3);
    r1_D_00130130 = r;
    return r;
}

ASM_FUNC("asm/core", func_0011F120);

/* rac1-decomp src/core/0011D0D0.c: func_0011D4A0 */
/* Set up the TLB for the machine's memory size: the 32 MB layout
   (r1_func_0011D4E0) when r1_func_00118EA0 reports 0x2000000, the kernel's own
   routine (r1_func_00118EB0) otherwise. Returns its callees' values: retail
   keeps the frame and calls both, and 2.9-ee turns both arms into tail
   jumps (44 bytes against 64) when it is void. */
int func_0011F130(void) {
    if (r1_func_00118EA0() == 0x2000000) {
        return r1_func_0011D4E0();
    } else {
        return r1_func_00118EB0();
    }
}

ASM_FUNC("asm/core", func_0011F170);

INCLUDE_ASM("asm/core", func_0011F364);
