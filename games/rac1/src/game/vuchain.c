#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * vuchain.cpp in the original source; text 0x2347F0-0x2352C8.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);
extern void func_002023E0(int);
extern void func_002027C0(int);
extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;
extern int D_001A0468[];
extern void func_00205830(int a, int b);
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");
extern int D_001A01F0[];
extern int *D_001602E0;
extern unsigned char D_0013D49C NOT_SDA;
extern unsigned char D_0013D49D NOT_SDA;
extern unsigned char D_0013D4A5 NOT_SDA;
extern short D_0015FE24;
extern unsigned char D_0013D4AC NOT_SDA;
extern unsigned char D_0013D4AD NOT_SDA;
extern unsigned char D_0013D4AE NOT_SDA;
extern unsigned char D_0013D4AF NOT_SDA;
extern unsigned char D_0013D4B5 NOT_SDA;
extern int D_001A04B4 NOT_SDA;
extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;
extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;
extern unsigned char D_0013D4E0;
extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;
extern unsigned char D_0013D4E9 NOT_SDA;
extern unsigned char D_0013D502 NOT_SDA;
extern unsigned char D_0013D503 NOT_SDA;
extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);
extern short D_0015EE84;
extern int D_0015EE84_far __asm__("D_0015EE84") NOT_SDA;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;
extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);
extern int func_001236F0(void);
extern int func_001E9730();
extern char D_001E8690[];
extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;
extern int D_0013D9B4 NOT_SDA;
extern unsigned char D_0013D490[];
extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;
extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;
extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;
extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;
extern int D_0013D5C8 NOT_SDA;
extern unsigned char D_0013D4B0 NOT_SDA;
extern unsigned char D_0013DE55 NOT_SDA;
extern unsigned char D_0013D5DD NOT_SDA;
extern unsigned char D_0013D5E7 NOT_SDA;
extern int D_001B2F40[];
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_00118D80(int);
extern char D_00165600[];
extern char D_001B3200[];
extern int func_001160D8(void);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9DC0(void *, void *, float);
extern void func_001FA460(void *);
extern void func_002150B0(void *, void *);
extern void func_001FA480(void *, void *);
extern float func_0020D830(void);
extern float func_00215A98(int, float);
extern unsigned char D_0014BFC0[];
extern unsigned char D_0013E620[];
extern unsigned char D_0013D510[];
extern void func_0012F068(void *);
extern void func_002177F0(int);
extern short D_001517D0[];
extern void func_0012EDE0(void *);
extern void func_0012EFE8(void);
extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);
extern int func_0012F030(void);
extern void func_002167C0(short, short, short);
extern void func_002169B8(short, short, short);
extern short D_001517F0 NOT_SDA;
extern char D_0013CA40[];
extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);
extern void func_00217F68(void *);
extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];
extern char D_001D5F70[] NOT_SDA;
extern char D_001D603B[];
extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);
extern int D_00141FA0[];
extern char D_001D0A50[];
extern char D_001D0A88[];
extern int D_001A0418 NOT_SDA;
extern float func_001FA748(float, float);
extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");
extern int func_002267C0(int);
extern void func_00234C98(int, long);
extern void func_00205E70(void);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern short D_001602B0;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;
extern int func_00226EA8(int);
extern int func_00226F68(int);
extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);
typedef struct {
    int key;
    int flags;
} PadBind;
extern PadBind D_001D6448_t[] __asm__("D_001D6448");
extern int func_00227018(int handle);
extern int D_001D6448[];
extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;
extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];
extern char D_00187180_a[] __asm__("D_00187180");
extern char D_00194220[];
extern int D_0013E6BC;
extern void func_002141A8(void *, float, float);
extern void func_001F9BD8_a(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_a(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BF0_a(void *, void *, void *) __asm__("func_001F9BF0");
extern int func_001EFE10_a(void *, void *, int, int, int) __asm__("func_001EFE10");
extern char D_00187180[];
extern void func_001F9BF0(void *, void *, void *);
extern void func_001EFE10(void *, void *, int, int, int);
extern float func_001F9D10(int, void *);
extern void func_0022DA10(void *, float, float, float);
extern void func_001F9EE8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_001FA058_a(float, float) __asm__("func_001FA058");
extern void func_001FA898(float);
extern void func_00120F30(int);
extern int func_0012E060(void *, int);
extern void func_0012EE70(int);
extern void func_0012EF48(int);
extern void func_0012E2E8(void);
extern void func_0011AE20(int arg0);
typedef struct {
    int unk_00;
    int unk_04;
    int unk_08;
    int unk_0C;
} Rec10;
extern Rec10 D_001DD568[];
typedef struct {
    short a;
    short b;
} TexRemap;
typedef struct {
    char *items;
    int count;
} TexChunk;
extern TexChunk D_001E1200[];
extern TexRemap D_001E0F00[];


#define VIF0_STAT ((volatile unsigned int *)0x10008000)
#define VIF0_FBRST ((volatile unsigned int *)0x10008020)
#define VIF0_BASE ((volatile unsigned int *)0x10008030)

/* VU0_loadMicroProgram(long *): waits for VIF0 to go idle, resets it,
 * points it at the (uncached-masked) micro program address, kicks off
 * MSCALL 0x45 by writing the code word directly, then waits for VIF0 to
 * go idle again. */
void func_002347F0(long *prog) {
    while (*VIF0_STAT & 0x100) {
        func_001F9988(0x10);
    }
    *VIF0_FBRST = 0;
    *VIF0_BASE = (unsigned int)prog & 0x0FFFFFFF;
    *VIF0_STAT = 0x145;
    while (*VIF0_STAT & 0x100) {
        func_001F9988(0x10);
    }
}

extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;
extern int D_0016100C_m __asm__("D_0016100C") MACRO_ADDR;
extern int D_001DE338[];

/* Advances the index at D_0016100C, wrapping at 0x13. The old note had
   the condition backwards: `slti; movz x,$0,t` is `if (idx >= 0x13)
   idx = 0`. Both globals are MACRO_ADDR, which gives the one-register
   load and the $gp store in the jr slot. */
void func_002348B8(void) {
    int idx = D_0015EE84_m;
    if (idx >= 0x13) {
        idx = 0;
    }
    D_0016100C_m = D_001DE338[idx];
}

/*
 * The VU1 chain state. D_00161000 (the write pointer), D_00161010 (the
 * buffer index) and the two limits D_0015F718/D_0015F71C are all reached
 * with retail's one-instruction macro form, so they are MACRO_ADDR: the
 * assembler expands them through $at, or through $gp where the access
 * lands in a delay slot.
 */
extern int *D_00161000 MACRO_ADDR;
extern int D_00161004 MACRO_ADDR;
extern int D_0016100C;
extern int D_00161010 MACRO_ADDR;
extern int D_0015F698;
extern int D_0015F718 MACRO_ADDR;
extern int D_0015F71C MACRO_ADDR;
extern int D_00160FF8[2];
extern int D_001941C0[];

extern int D_0015F698_m __asm__("D_0015F698") MACRO_ADDR;

/* `end` reads D_00161000 back right after storing it (CSE folds the
   load away), which changes how the stores are scheduled: that, not
   any order of the four stores, gives retail's. */
void func_002348E8(void) {
    int base = D_001941C0[1];
    int end;

    D_00160FF8[0] = base;
    D_00160FF8[1] = D_001941C0[2];
    D_00161010 = 0;
    D_00161000 = (int *)base;
    end = (int)D_00161000 + D_0016100C_m - D_0015F698_m;
    D_0015F718 = end;
    D_0015F71C = end - 0x2000;
}

/* VU1_swapChain(void) */
void func_00234948(void) {
    int idx = 1 - D_00161010;
    int base = D_00160FF8[idx];
    int end;

    D_00161004 = (int)D_00161000;
    D_00161010 = idx;
    D_00161000 = (int *)base;
    end = (int)D_00161000 + D_0016100C_m - D_0015F698_m;
    D_0015F718 = end;
    D_0015F71C = end - 0x2000;
}

extern int D_00161014 MACRO_ADDR;
extern short D_00160FE0;
extern char D_001E8CF8[];
extern int *func_001232E0(int);
extern void func_001235C8(int *, int);

/* VU1_sendChain. D_00160FE0 is volatile, so its store stays out of the
   delay slot (reorg never moves a volatile access there). */
void func_002349B8(void) {
    int size;
    int err;
    int *chan;

    *(volatile int *)&D_00160FE0 |= 0x1F;
    size = (int)D_00161000 - D_00160FF8[D_00161010];
    err = 0;
    if (D_00161014 < size) {
        D_00161014 = size;
        if (D_0016100C_m < size) {
            STUB_printf(D_001E8CF8);
            err = 1;
        }
    }
    if (err == 0) {
        D_00161000[0] = 0x70000000;
        D_00161000[1] = 0;
        D_00161000[2] = 0;
        D_00161000[3] = 0;
        chan = func_001232E0(1);
        *chan |= 0xC0;
        func_00118D80(0);
        func_001235C8(chan, D_00160FF8[D_00161010]);
    } else {
        *(volatile int *)&D_00160FE0 = 0;
    }
}

extern short D_00160FE0;              /* SDA, gp -0x5D20 */
extern char D_001E8D10[];
extern void func_001F2568(void);

/* VU1_syncChain(int) */
void func_00234AC8(int mask) {
    int i = 0;
    while ((*(int *)&D_00160FE0 & mask) != 0) {
        func_001F9988(0x400);
        if (i > 100000) {
            STUB_printf(D_001E8D10);
            ResetVideoPipeline();
            break;
        }
        i++;
    }
}

/*
 * VU1_addDataRef(void *, int). Retail reloads D_00161000 before every
 * field write -- the writes could alias the pointer itself -- which is
 * exactly what writing through the global (rather than through a local
 * copy) produces.
 */
void func_00234B48(void *data, int qwc) {
    D_00161000[0] = qwc | 0x30000000;
    D_00161000[1] = (int)data;
    D_00161000[2] = 0;
    D_00161000[3] = 0;
    D_00161000 += 4;
}

LINKER_REMNANT("asm/remnants/text", func_00234B98);

void func_00234BA0(int arg0, void *src, int qwc) {
    D_00161000[0] = qwc | 0x10000000;
    D_00161000[1] = 0;
    D_00161000[2] = 0x1000404;
    D_00161000[3] = arg0 | (qwc << 16) | 0x6C000000;
    D_00161000 += 4;
    FastMemCopy(D_00161000, src, qwc * 16);
    D_00161000 += qwc * 4;
}

LINKER_REMNANT("asm/remnants/text", func_00234C48);

void func_00234C50(int arg0) {
    D_00161000[0] = 0x10000000;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = arg0;
    D_00161000 += 4;
}

/*
 * VU1_addGSregister(unsigned int, unsigned long). Real name per the
 * Lombyte NTSC project (github.com/mateuszklysz/Lombyte); their NTSC
 * build's version, VU1_addGSregister__FUiUlb, takes a third `bool` arg
 * and is 4 bytes larger (0xb4 vs this PAL build's 176 bytes) -- PAL
 * evidently dropped the bool, confirmed by every real call site in this
 * codebase using only 2 args.
 */
void func_00234C98(int arg0, long arg1) {
    D_00161000[0] = 0x10000002;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000002;
    D_00161000[4] = 0x8001;
    D_00161000[5] = 0x10000000;
    D_00161000[6] = 0xE;
    D_00161000[7] = 0;
    *(long *)((char *)D_00161000 + 0x20) = arg1;
    D_00161000[10] = arg0;
    D_00161000[11] = 0;
    D_00161000 += 0xC;
}

LINKER_REMNANT("asm/remnants/text", func_00234D50);

extern int D_0013E600[];

/* VU1_setScissor(x0, x1, y0, y1): clamps the rectangle to the screen
   (D_0013E600[0]/[1] are its width and height) and queues it as GS
   register 0x40 (SCISSOR_1: x0 | x1 << 16 | y0 << 32 | y1 << 48) in the
   same packet VU1_addGSregister (func_00234C98) builds. Each clamp is a
   `cond ? bound : arg` select assigned back to the argument, and the
   fields are sign-extended to 64 bits, not zero-extended. */
void func_00234D58(int x0, int x1, int y0, int y1) {
    x0 = x0 < 0 ? 0 : x0;
    x1 = x1 > D_0013E600[0] - 1 ? D_0013E600[0] - 1 : x1;
    y0 = y0 < 0 ? 0 : y0;
    y1 = y1 > D_0013E600[1] - 1 ? D_0013E600[1] - 1 : y1;
    D_00161000[0] = 0x10000002;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000002;
    D_00161000[4] = 0x8001;
    D_00161000[5] = 0x10000000;
    D_00161000[6] = 0xE;
    D_00161000[7] = 0;
    *(long *)((char *)D_00161000 + 0x20) =
        (long)x0 | ((long)x1 << 16) | ((long)y0 << 32) | ((long)y1 << 48);
    D_00161000[10] = 0x40;
    D_00161000[11] = 0;
    D_00161000 += 0xC;
}

__asm__(".section .text\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_00234E78);

extern char D_001DF180[];

/* VU1_texFlush(void) */
void func_00234E80(void) {
    D_00161000[0] = 0x30000003;
    D_00161000[1] = (int)D_001DF180;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000003;
    D_00161000 += 4;
}

extern char D_001DE740[];

/* VU1_gsRegsNormal(void) */
void func_00234EE0(void) {
    D_00161000[0] = 0x30000003;
    D_00161000[1] = (int)D_001DE740;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000003;
    D_00161000 += 4;
}

extern char D_001DE770[];

void func_00234F40(void) {
    D_00161000[0] = 0x30000003;
    D_00161000[1] = (int)D_001DE770;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000003;
    D_00161000 += 4;
}

LINKER_REMNANT("asm/remnants/text", func_00234FA0);

extern char D_0013D010[];

void func_00234FA8(void) {
    D_00161000[0] = 0x3000000B;
    D_00161000[1] = (int)D_0013D010;
    D_00161000[2] = 0;
    D_00161000[3] = 0x5000000B;
    D_00161000 += 4;
}

LINKER_REMNANT("asm/remnants/text", func_00235008);

extern int D_00161018 MACRO_ADDR;
extern int D_0016101C MACRO_ADDR;

extern int func_00118AB0(int, void *, void *);
extern void func_00119460(int);
extern void func_00235118(void);
extern void func_00235218(void);

/*
 * DMAC_VIF1_Enable(void): unless the handlers are installed, set CIM1 in
 * D_STAT and install func_00235118 (channel 1) and func_00235218 (0xF),
 * then enable channel 1. The D_STAT read is volatile but the store is
 * plain: a volatile store stops reorg's try_merge_delay_insns at it and
 * leaves a second copy of the handler's lui on the fall-through path.
 */
void func_00235018(void) {
    if (D_00161018 == 0 && D_0016101C == 0) {
        if ((*(volatile int *)0x1000E010 & 0x20000) == 0) {
            *(int *)0x1000E010 = 0x20000;
        }
        D_00161018 = func_00118AB0(1, (void *)func_00235118, (void *)0);
        D_0016101C = func_00118AB0(0xF, (void *)print_register_values_and_halt, (void *)0);
        func_00119460(1);
    }
}

/*
 * DMAC_VIF1_Disable(void). The two handler ids are MACRO_ADDR: the copy
 * scheduled into the branch delay slot is $gp-relative, the copy on the
 * taken path is the lui/%lo expansion of the same macro. The volatile
 * D_STAT write is what forces the reload on that path.
 */
extern int func_00118AD0(int, int);
extern int func_001193F8(int);

void func_002350A8(void) {
    if ((*(volatile int *)0x1000E010 & 0x20000) != 0) {
        *(volatile int *)0x1000E010 = 0x20000;
    }
    func_00118AD0(1, D_00161018);
    func_00118AD0(0xF, D_0016101C);
    func_001193F8(1);
    D_00161018 = 0;
    D_0016101C = 0;
}

ASM_FUNC("asm/handwritten/text", func_00235118);

extern char D_001E8D38[];
extern char D_001E8D50[];
extern char D_001E8D68[];

void func_00235218(void) {
    int a = *(volatile int *)0x10009000;
    int b = *(volatile int *)0x10009030;

    STUB_printf(D_001E8D38);
    STUB_printf(D_001E8D50, a, b);
    STUB_printf(D_001E8D68);
    while (1) {
        ;
    }
}

/*
 * Append one quadword to the packet at D_00161000. This one wants the
 * pointer typed as a quadword: stepping an `int *` by 4 folds the copy
 * retail keeps (`daddu $5,$2,$0`) into the increment and comes out one
 * instruction short, so the same symbol is declared a second time under
 * a quadword type.
 */
typedef struct {
    int w[4];
} Qword;
extern Qword *D_00161000_q __asm__("D_00161000") MACRO_ADDR;

void func_00235290(int arg0) {
    Qword *p = D_00161000_q++;
    p->w[0] = arg0 + 0x90000000;
    p->w[1] = 0;
    p->w[2] = 0;
    p->w[3] = 0;
}
