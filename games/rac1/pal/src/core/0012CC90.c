#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x12CC90-0x12D868. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony SDK code, built with Sony's 2.9-ee (Makefile.sn, EE29_CORE). Up to
 * 0x12D068: IPU DMA helpers for libmpeg (the CHCR writes of channels 3/4
 * under a D_ENABLE hold). From 0x12D2A0: libscf -- the rom0:ROMVER record,
 * sceScfGetTimeZone (func_0012D3F0), sceScfGetSummerTime (func_0012D448),
 * the BCD<->binary sceCdCLOCK helpers and sceScfGetLocalTimefromRTC
 * (func_0012D818). Its day/hour helpers end in conditional tail calls
 * (func_0012D730, func_0012D760), which only 2.9-ee emits.
 */

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
extern void func_00118EC0(void);
extern void func_0011D4E0(void);
extern void func_00118EB0(void);
extern int D_00130420;
extern int D_00130424;
extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);
extern int func_00120F30(int);
extern void *D_00159840;
extern int D_001313E0;
extern int D_001313E8;
extern int D_001313EC;
extern int D_001313F0;
extern int D_001313E4;
extern int D_001313FC;
extern int func_0011B4C8();
extern int func_00120D28(int);
extern void func_00118C90(int);
extern char D_00132590[];
extern int D_00131440;
extern void func_0011A6C8();
extern int func_0011B6B8(void *);
extern char D_00153000[];
extern char D_00132E08[];
extern int D_001313D0;
extern int func_00121040(int);
extern int D_001325C0;
extern char D_00132E40[];
extern void func_00123650(void *);
extern char D_001534E0[];
extern int D_00132E70[];
extern int func_001238B0(int, int, int, int);
extern int D_00132EA8;
extern int *D_00159B28;
extern int *D_00159B2C;
extern int *D_00159B30;
extern char D_0015B108[];
extern int D_0015B180;
extern void func_00124B60(void *);
extern char D_00153658[];
extern int func_00124920(int);
typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;
extern Ent330 D_0015B640[];
extern void func_00119288(void *a, void *b);
extern void func_001286E8(int, int);
extern int D_00132F70[];
extern void func_0012BCC8(int);
extern void func_00128590(void *);
extern void func_00128968(void *, int);
extern int func_00128860(void *, int);
extern int func_00128A58(void *, int);
extern void func_00129180(void *);
extern char D_00153A80[];
extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");
extern void func_0012AAC8(void *, int);
extern void func_0012C468(void *, void *);
extern char D_00153B38[];
extern void func_0012C0A0(void *);
extern void func_0012BF40(void *);
extern void func_0012C278(void *);
extern void func_00129E30(void *, int, int, int);
extern char D_00153BB8[];
extern char D_00153BD8[];
extern void func_00116248(void *);
extern void func_0012C420(void *);
extern char D_00153C48[];
extern char D_00153C78[];
extern char D_00153C90[];
extern char D_00153CC8[];
extern void func_0012CE48(unsigned int *);

/*
 * Write a CHCR value to IPU DMA channel 3 (fromIPU, 0x1000B000) or 4
 * (toIPU, 0x1000B400): with interrupts off (func_0011D960/func_0011D9A8),
 * set D_ENABLEW.CPND (0x1000F590 = D_ENABLER | 0x10000) to hold the DMAC,
 * store the value, release the hold. The final EI is a tail call.
 *
 * Exact under 2.9-ee as the plain volatile MMIO sequence. It was blocked
 * under 2.95.3 only because that compiler never emits a sibling call:
 * with a frame of its own the last call stayed a `jal` plus a full
 * epilogue, 3 words over (0x70 against 0x64).
 */
void func_0012CC90(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}

void func_0012CCF8(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}

/* sceIpuStopDMA: request the IPU-in (D4/toIPU) DMA channel to stop
 * (func_0012CCF8(1)), save its MADR/TADR/QWC/CHCR into env[0..3], wait for
 * the IPU's queued-command count (IPU_CTRL bits 4-7) to drain, stop the
 * IPU-out (D3/fromIPU) channel (func_0012CC90(0)) and save its
 * MADR/QWC/CHCR plus IPU_BP and IPU_CTRL into env[4..8]. */
extern void func_0012CCF8(unsigned int);
extern void func_0012CC90(unsigned int);

void func_0012CD60(unsigned int *env) {
    func_0012CCF8(1);
    env[0] = *(volatile unsigned int *)0x1000B410;
    env[1] = *(volatile unsigned int *)0x1000B430;
    env[2] = *(volatile unsigned int *)0x1000B420;
    env[3] = *(volatile unsigned int *)0x1000B400;
    while ((*(volatile unsigned int *)0x10002010 & 0xF0) != 0) {
    }
    func_0012CC90(0);
    env[4] = *(volatile unsigned int *)0x1000B010;
    env[5] = *(volatile unsigned int *)0x1000B020;
    env[6] = *(volatile unsigned int *)0x1000B000;
    env[7] = *(volatile unsigned int *)0x10002020;
    env[8] = *(volatile unsigned int *)0x10002010;
}

/* sceIpuRestartDMA: given the env sceIpuStopDMA (func_0012CD60) filled,
 * work out how many quadwords the IPU's bitstream pointer (env[7], IPU_BP)
 * had already consumed (its FIFO+chain counts, bits 8-10/16-17) and fold
 * that back into the saved D4 MADR/QWC; restart D3 (fromIPU) first if it
 * still had a QWC left, reissue the BP position to the IPU command
 * register, wait for the IPU to go idle, then restart D4 (toIPU) with the
 * corrected MADR/QWC if there is anything left to transfer.
 *
 * The masked BP value needs its own local right after the load: masking
 * `bp` again at the store keeps it alive across func_0012CC90 and moves
 * qwc4 and madr4 into the wrong saved registers. */

void func_0012CE48(unsigned int *env) {
    unsigned int bp = env[7];
    unsigned int bpLow = bp & 0x7F;
    unsigned int madrSaved = env[0];
    unsigned int cnt = ((bp >> 16) & 3) + ((bp >> 8) & 0xF);
    unsigned int d3madr = env[4];
    unsigned int qwc4 = env[2] + cnt;
    unsigned int madr4 = madrSaved - (cnt << 4);

    if (d3madr != 0 && env[5] != 0) {
        *(volatile unsigned int *)0x1000B010 = d3madr;
        *(volatile unsigned int *)0x1000B020 = env[5];
        func_0012CC90(env[6] | 0x100);
    }

    while (*(volatile int *)0x10002010 < 0) {
    }
    *(volatile unsigned int *)0x10002000 = bpLow;
    while (*(volatile int *)0x10002010 < 0) {
    }

    if (madr4 == 0) {
        return;
    }
    if (qwc4 == 0) {
        return;
    }
    *(volatile unsigned int *)0x1000B410 = madr4;
    *(volatile unsigned int *)0x1000B430 = env[1];
    *(volatile unsigned int *)0x1000B420 = qwc4;
    func_0012CCF8(env[3] | 0x100);
}

int func_0012CF98(int arg0) {
    int ret = 0;

    switch (arg0) {
    case 0:
        while (*(volatile int *)0x10002010 < 0) {
        }
        ret = 0;
        break;
    case 1:
        ret = *(volatile unsigned int *)0x10002010 >> 31;
        break;
    }

    return ret;
}

/* Sony's setD4_CHCR, linked a second time as ipuinit.o's own private copy
 * (called from sceIpuInit, which immediately follows at 0x12D068): write
 * chcr to IPU DMA channel 4's CHCR (toIPU, 0x1000B400) with interrupts off
 * and D_ENABLEW.CPND (0x1000F590 = D_ENABLER | 0x10000) held around it.
 * Byte-identical to func_0012CCF8 (setD4_CHCR's other copy, already
 * matched) since it is the same source compiled into a different object.
 */
void func_0012D000(unsigned int chcr) {
    func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    func_0011D9A8();
}

typedef unsigned int u128 __attribute__((mode(TI), aligned(16)));

extern void func_0012D000(unsigned int chcr);
extern volatile u128 D_00133150[];
extern u128 D_001331A0[];

#define IPU_CMD ((volatile unsigned int *)0x10002000)
#define IPU_CTRL ((volatile int *)0x10002010)
#define IPU_IN_FIFO ((volatile u128 *)0x10007010)

/* sceIpuInit: reset the IPU, load its two fixed FIFO command tables, reset again. Adapted from Lombyte (MIT) for PAL: src/sdk/dma/sce_ipu_init.c, sceIpuInit. */
void func_0012D068(void) {
    func_0012D000(1);
    *IPU_CTRL = 0x40000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0;
    while (*IPU_CTRL < 0) {
    }
    *IPU_IN_FIFO = D_00133150[0];
    *IPU_IN_FIFO = D_00133150[1];
    *IPU_IN_FIFO = D_00133150[2];
    *IPU_IN_FIFO = D_00133150[3];
    *IPU_IN_FIFO = D_00133150[4];
    *IPU_IN_FIFO = D_00133150[4];
    *IPU_IN_FIFO = D_00133150[4];
    *IPU_IN_FIFO = D_00133150[4];
    *IPU_CMD = 0x50000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0x58000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_IN_FIFO = D_001331A0[0];
    *IPU_IN_FIFO = D_001331A0[1];
    *IPU_CMD = 0x60000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0x90000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CTRL = 0x40000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0;
    while (*IPU_CTRL < 0) {
    }
}

/*
 * Fill the 14-byte rom0:ROMVER record at D_001331D8 once (open
 * D_00153D00 "rom0:ROMVER" read-only, read 0xE bytes, close; failures are
 * reported through func_0011A6C8) and hand the buffer back either way.
 * (The second func_0011A6C8 gets only the format string: $a1 still holds
 * the buffer from the read call and retail never resets it, which is why
 * func_0011A6C8 stays unprototyped here.)
 *
 * Exact under 2.9-ee. It was blocked under 2.95.3 (152/160 against 156):
 * that compiler's GCSE copies the kept %hi(D_001331D8) into a saved
 * register (`move $19,$2`), where 2.9-ee loads the lui straight into $17
 * as retail does. D_001331D8 is `signed char` (the declaration below),
 * hence the cast on the return.
 */
extern int func_0011BF80(void *, int);
extern int func_0011C5C0(int, void *, int);
extern void func_0011C208(int);
extern signed char D_001331D8[];
extern char D_00153D00[], D_00153D10[], D_00153D28[];

char *func_0012D2A0(void) {
    int fd;
    if (D_001331D8[0] == 0) {
        fd = func_0011BF80(D_00153D00, 1);
        if (fd == -1) {
            func_0011A6C8(D_00153D10, D_001331D8);
        }
        if (func_0011C5C0(fd, D_001331D8, 0xE) == -1) {
            func_0011A6C8(D_00153D28);
        }
        func_0011C208(fd);
    }
    return (char *)D_001331D8;
}

extern char *func_0012D2A0(void);

int func_0012D340(void) {
    if (D_001331D8[0] == 0) {
        func_0012D2A0();
    }
    return D_001331D8[4] == 0x54;
}

/*
 * Both of these return a flag byte that retail loads with the address
 * register reused as the destination (`lui $2,%hi(X)` / `lbu
 * $2,%lo(X)($2)`), and it loads it ACROSS TWO DELAY SLOTS -- the lui in
 * the branch's, the lbu in the following b's. That rules MACRO_ADDR out
 * here, and the attempt is worth recording: a MACRO_ADDR access in a
 * delay slot is expanded $gp-relative, and these two symbols live
 * outside the +-32KB window, so the link fails outright with
 * "relocation truncated to fit: R_MIPS_GPREL16". A plain incomplete
 * array gives retail's exact split-across-delay-slots shape; the only
 * residual is the allocator putting the address in $3 rather than
 * reusing $2, plus one more register choice in func_0012D448
 * (5/96 and 6/104 differing bytes). func_0012D448 also needed its arms
 * the other way round -- the early `return 0` written last, so the
 * zero case is the one that branches over.
 */
extern unsigned char D_001331D4[];

/* One return, the `== 0` arm first, and `v` unsigned (as func_0012D3F0). */
int func_0012D380(void) {
    int buf[4];
    unsigned int v;
    int r;

    func_00118CF0(buf);
    if (func_0012D340() != 0) {
        r = D_001331D4[0];
    } else {
        func_00118CF0(buf);
        v = buf[0];
        if (((v >> 13) & 7) == 0) {
            r = (v >> 4) & 1;
        } else {
            r = (v >> 16) & 0x1F;
        }
    }
    return r;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012D3E0);

extern short D_001331D0 NOT_SDA;

/* `r` assigned in both arms and returned once: a direct `return
   D_001331D0;` loads straight into $v0 and moves the address to $v1. */
int func_0012D3F0(void) {
    unsigned int local;
    unsigned int bits;
    int r;

    if (func_0012D340() != 0) {
        r = D_001331D0;
    } else {
        func_00118CF0(&local);
        bits = (local >> 13) & 7;
        if (bits == 0) {
            r = 0x21C;
        } else {
            r = (int)local >> 21;
        }
    }
    return r;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012D440);

extern unsigned char D_001331D6[];
extern void func_00118DC0(void *, int, int);

extern int func_00118DC0_i(void *, int, int) __asm__("func_00118DC0");

/* One return, and func_00118DC0 returns int. */
int func_0012D448(void) {
    unsigned char buf[16];
    int r;

    if (func_0012D340() != 0) {
        r = D_001331D6[0];
    } else {
        func_00118CF0(buf);
        if (((*(unsigned int *)buf >> 13) & 7) == 0) {
            r = 0;
        } else {
            func_00118DC0_i(buf + 4, 1, 1);
            r = (buf[4] >> 4) & 1;
        }
    }
    return r;
}

int func_0012D4B0(int arg0) {
    unsigned int v = arg0 & 0xFF;
    return ((v / 10) * 6 + v) & 0xFF;
}

/*
 * REVERTED (size mismatch: ours 28 bytes, retail 32). Semantics certain
 * -- BCD byte to binary:
 *
 *   int func_0012D4E0(int arg0) {
 *       unsigned int v = arg0 & 0xFF;
 *       return (v - (v >> 4) * 6) & 0xFF;
 *   }
 *
 * (`unsigned` matters: it gives retail's `srl`, not `sra`.) Everything
 * matches except the multiply: retail uses the generic two-operand
 * `mult $0,$3,$4` followed by `mflo $3`, while this compiler uses the
 * EE three-operand `mult $v1,$v1,$a0` which writes the result directly
 * and needs no mflo -- one instruction fewer, hence 4 bytes short. That
 * is an ISA/codegen choice, not something the source can steer.
 */
/* BCD byte to binary. The product goes through an unsigned char: combine
   folds the multiply into that narrowing copy and re-recognises it as the
   generic LO-destination multiply, which is retail's `mult $0,a,b` +
   `mflo` (the plain int form picks the R5900 three-operand mult). */
/* The product goes through an unsigned char local: that gives retail's
   generic mult $0 + mflo instead of the three-operand mult. */
int func_0012D4E0(int arg0) {
    unsigned char v = arg0;
    unsigned char t = (v >> 4) * 6;
    return (unsigned char)(v - t);
}

extern int func_0012D4E0(int);

/* Convert the sceCdCLOCK fields (second..year) in place with the helper
   above. Exact under 2.9-ee, which saves $16 before $31 as retail does;
   2.95.3 saved them the other way round (4/104), the prologue-order
   residual that pointed at the SDK compiler in the first place. */
void func_0012D500(unsigned char *p) {
    p[7] = func_0012D4E0(p[7]);
    p[6] = func_0012D4E0(p[6]);
    p[5] = func_0012D4E0(p[5]);
    p[3] = func_0012D4E0(p[3]);
    p[2] = func_0012D4E0(p[2]);
    p[1] = func_0012D4E0(p[1]);
}

extern int func_0012D4B0(int);

/* The inverse conversion, same shape; exact under 2.9-ee for the same
   reason as func_0012D500 (4/104 under 2.95.3). */
void func_0012D568(unsigned char *p) {
    p[7] = func_0012D4B0(p[7]);
    p[6] = func_0012D4B0(p[6]);
    p[5] = func_0012D4B0(p[5]);
    p[3] = func_0012D4B0(p[3]);
    p[2] = func_0012D4B0(p[2]);
    p[1] = func_0012D4B0(p[1]);
}

typedef struct { char b[0xC]; } Cfg12;
extern Cfg12 D_00153D40 NOT_SDA;

/*
 * Tick the sceCdCLOCK at s (binary fields: [5] day, [6] month, [7] year)
 * forward by one day, with a working copy of the 12-byte month-length
 * table D_00153D40 (February stretched to 29 when year & 3 == 0), the
 * month rolling 12 -> 1 and the year 99 -> 0.
 *
 * The leap test reads the year into an int. Retail loads the year twice
 * (for the leap test, and again at the year roll-over); 2.9-ee's GCSE
 * would reuse a first byte-typed load for the second (SIZE 184/180, and
 * the whole reason this function once looked like 2.95.3 code). An int
 * load is a zero-extending load, a different expression to GCSE, so
 * the later byte load stays. (This C is exact under 2.95.3 as well.)
 */
void func_0012D5D0(unsigned char *s) {
    Cfg12 days;
    unsigned char m;
    int y = s[7];

    days = D_00153D40;
    s[5] = s[5] + 1;
    if ((y & 3) == 0) {
        days.b[1] = 0x1D;
    }
    if (s[5] > days.b[s[6] - 1]) {
        s[5] = 1;
        s[6] = s[6] + 1;
        m = s[6];
        if (m == 13) {
            if (s[7] == 0x63) {
                s[7] = 0;
            } else {
                s[7] = s[7] + 1;
            }
            s[6] = 1;
        }
    }
}

/*
 * Tick the sceCdCLOCK at s back by one day: the same month-length table,
 * the day counting down to 0 rolls the month back (1 -> 12) and the year
 * with it (0 -> 99), then the day is reloaded from the table.
 *
 * Levers: the leap test on an int copy of the year, as in func_0012D5D0
 * (2.9-ee's GCSE otherwise reuses the first year load: 61/164);
 * `s[5] = s[5] - 1;` before the leap block (a block boundary gives
 * retail's reload of s[5]); the month as `if (--s[6] == 0)` (stores the
 * raw decrement and tests the masked copy in another register); the
 * year as an if/else with one store per arm, which cross-jumping merges
 * into retail's single `sb`. Exact under both compilers.
 */
void func_0012D688(unsigned char *s) {
    Cfg12 days;
    int y = s[7];

    days = D_00153D40;
    s[5] = s[5] - 1;
    if ((y & 3) == 0) {
        days.b[1] = 0x1D;
    }
    if (s[5] == 0) {
        if (--s[6] == 0) {
            if (s[7] == 0) {
                s[7] = 0x63;
            } else {
                s[7] = s[7] - 1;
            }
            s[6] = 0xC;
        }
        s[5] = days.b[s[6] - 1];
    }
}

/*
 * Advance the hour ([3]); at 24 it wraps to 0 and the day ticks forward.
 * Retail's call is a bare tail `j func_0012D5D0` on the wrap path only,
 * with a plain `jr $ra` on the other: a conditional sibling call, which
 * 2.9-ee emits from this plain C. (Under 2.95.3, which has no sibling
 * calls, it was 8 bytes over.)
 */
void func_0012D730(unsigned char *s) {
    s[3] = s[3] + 1;
    if (s[3] == 0x18) {
        s[3] = 0;
        func_0012D5D0(s);
    }
}

/* The backward analog: the hour counts down, and at 0 it becomes 23 and
   the day ticks back, again through a conditional tail call. */
void func_0012D760(unsigned char *s) {
    unsigned char v = s[3];
    if (v != 0) {
        s[3] = v - 1;
        return;
    }
    s[3] = 0x17;
    func_0012D688(s);
}

/*
 * Shift the sceCdCLOCK at s by diff minutes: BCD to binary
 * (func_0012D500), add diff to the minutes, carry whole hours down or up
 * through the hour helpers, store the minutes, binary back to BCD
 * (func_0012D568, a tail call). Note the upper bound is `> 60`, not
 * `>= 60`: retail's test is `slti 0x3D`.
 *
 * The `min >= 0` arm has to come first: retail lays the carry-up loop
 * inline and the carry-down loop after it.
 */
void func_0012D788(unsigned char *s, int diff) {
    int min;

    func_0012D500(s);
    min = s[2] + diff;
    if (min >= 0) {
        while (min > 60) {
            min -= 60;
            func_0012D730(s);
        }
    } else {
        while (min < 0) {
            min += 60;
            func_0012D760(s);
        }
    }
    s[2] = min;
    func_0012D568(s);
}

/*
 * sceScfGetLocalTimefromRTC: shift the RTC clock (Japan time, UTC+9) by
 * the configured time zone (func_0012D3F0, minutes) and summer time
 * (func_0012D448, hours), i.e. by tz - 540 + st * 60 minutes. Written in
 * that order: `tz + st * 60 - 540` associates differently (19/72).
 */
void func_0012D818(unsigned char *s) {
    int tz, st;
    tz = func_0012D3F0();
    st = func_0012D448();
    func_0012D788(s, tz - 540 + st * 60);
}

/* 8 bytes of post-endlabel nop padding in retail -- see func_001F6668. */
__asm__(".section .text\n\tnop\n\tnop\n");
