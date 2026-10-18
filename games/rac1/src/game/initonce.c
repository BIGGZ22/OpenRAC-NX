#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * initonce.cpp in the original source; text 0x201E10-0x202260.
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

extern char D_0024272F[];
extern int D_0016100C MACRO_ADDR;
extern int D_001941C0[];

/*
 * InitMemSlots(void): the memory slot table D_001941C0, from the
 * 16 KB-aligned end of the image up.
 *
 * The store order is the scheduler's, and it is predictable: before
 * reload, ties in priority go to the insn that frees a register. The
 * source's last store also frees the table base, so it goes first;
 * stores that are the last use of their value follow in source order,
 * and a value stored twice (d, into [5] and [6]) has its first store
 * emitted last. The allocation follows the schedule.
 */
void InitMemSlots(void) {
    int base = (int)D_0024272F & 0xFFFFC000;
    int size = D_0016100C;
    int a = base + size;
    int b = a + size;
    int c = b + 0x64000;
    int d = c + 0x30000;
    D_001941C0[0] = base;
    D_001941C0[5] = d;
    D_001941C0[6] = d;
    D_001941C0[8] = 0x7000000;
    D_001941C0[9] = 0x7100000;
    D_001941C0[1] = base;
    D_001941C0[2] = a;
    D_001941C0[3] = b;
    D_001941C0[4] = c;
    D_001941C0[10] = 0x7200000;
}

/* Externs this function needs that initonce.c does not already declare. */
extern void func_001207B8(void);
extern int func_00123308(int);
extern void func_001211B0(int);
extern void func_00121490(int);
extern int func_0011D248(void *);
extern int func_0011D210(void);
extern int func_001E9730(); /* debug-log stub: void func_001E9730(int, ...) */
extern void func_00118D60(int);
extern void func_0011AE20(int);
extern int func_0011D960(void); /* DIntr */
extern void func_0011CB40(void);
extern void func_0011D9A8(void); /* EIntr */
extern int func_00121688(int);
extern int func_0011BF48(void);
extern int func_0012F348(int lba, int sectors, void *buf);
extern void func_00118D80(int);
extern void func_00209A60(void *arg0);
extern void func_00121B78(int, int, int, int);
extern void func_0020C268(void);
extern void func_001F3890(void);
extern void func_002347F0(void *);
extern int func_0012F3F8(void);
extern void func_0020C468(void *, void *);
extern int func_00201D58(void *arg0, void *arg1);
extern void func_00217EE8(void);
extern void func_0012F308(void); /* vsync_callback(int) */
extern int func_00123168(void *arg0);
extern void func_0020BAA8(void);
extern void func_002348E8(void);
extern void func_0022DBE8(void);
extern int func_00122630(void *, short, short, short, short, short, short, short);
extern int func_00122958(void *src, void *dst);
extern void func_00120858(int, unsigned short);
extern void func_001E96B8(void);
extern void func_00233FF8(void);
extern void func_001F7BF8(void);
extern int func_0012D380(void);

extern char D_001E7E10[]; /* "cdrom0:\\IOPRP243.IMG;1" */
extern char D_0015FB40[]; /* "Rebooted IOP\n" */
extern int D_0015EF90 MACRO_ADDR;
extern int D_0015EE80 MACRO_ADDR;
extern int D_0010E4C0;
extern int D_00137C80[];
extern char D_1FF8000[];
extern char D_001942C0[];
extern char D_001E7E28[]; /* "loaded irx modules\n" */
extern int D_0015EE88 MACRO_ADDR;
extern char D_001E7E40[]; /* "Unsupported language %d\n" */

/* InitOnce(void). Boots the IOP with cdrom0:\IOPRP243.IMG, waits for it,
 * reads the boot-info sector (lba 0x121) into a stack buffer, sets the
 * NTSC/PAL-ish flag from byte 0x33 of that sector, sets up video and the
 * VU/DMA chain, loads a set of overlay segments from a table right below
 * the 16 KB-aligned end of the image, sets up memory slots, loads a
 * palette/font image, and finally maps sceCdGetDiskType's result to the
 * current-language global.
 * Adapted from Lombyte (MIT) for PAL: runtime/startup/init_once.c, init_once. */
void InitOnce(void) {
    unsigned char buf[0x800];
    char *seg;
    int base;
    int *tbl;
    int region;
    int lang;

    func_001207B8();
    func_00123308(1);
    func_001211B0(0);
    func_00121490(0);
    do {
    } while (func_0011D248(D_001E7E10) == 0);
    do {
    } while (func_0011D210() == 0);
    STUB_printf(D_0015FB40);
    func_00118D60(3);
    func_0011AE20(0);
    func_0011D960();
    func_0011CB40();
    func_0011D9A8();
    func_001211B0(0);
    func_00121490(0);
    func_00121688(2);
    func_0011BF48();
    func_0012F348(0x121, 1, buf);
    func_00118D80(0);

    region = buf[0x33] != 0x4E;
    D_0015EF90 = D_0015EE80 = region;
    MemCard_BuildNameStamp(buf);

    func_00121B78(0, 1, (D_0015EE80 != 0) ? 3 : 2, 0);
    InitDma();
    SetPalMode();
    VU0_loadMicroProgram(&D_0010E4C0);
    func_0012F3F8();

    base = (int)D_0024272F & 0xFFFFC000;
    tbl = (int *)(base + 0x2C0000);
    seg = D_1FF8000 - (D_00137C80[0x4B1] << 0xB);
    func_0012F348(D_00137C80[0x4B0], D_00137C80[0x4B1], seg);
    func_00118D80(0);
    func_0020C468(seg, tbl);
    func_00118D80(0);
    LoadIRXModule((void *)(tbl[0x1C] + (int)tbl), (void *)(int)tbl[0x1D]);
    LoadIRXModule((void *)(tbl[0x1E] + (int)tbl), (void *)(int)tbl[0x1F]);
    LoadIRXModule((void *)(tbl[0x20] + (int)tbl), (void *)(int)tbl[0x21]);
    LoadIRXModule((void *)(tbl[0x22] + (int)tbl), (void *)(int)tbl[0x23]);
    LoadIRXModule((void *)(tbl[0x24] + (int)tbl), (void *)(int)tbl[0x25]);
    LoadIRXModule((void *)(tbl[0x26] + (int)tbl), (void *)(int)tbl[0x27]);
    LoadIRXModule((void *)(tbl[0x26] + (int)tbl), (void *)(int)tbl[0x27]);
    LoadIRXModule((void *)(tbl[0x2A] + (int)tbl), (void *)(int)tbl[0x2B]);
    LoadIRXModule((void *)(tbl[0x2C] + (int)tbl), (void *)(int)tbl[0x2D]);
    LoadIRXModule((void *)(tbl[0x28] + (int)tbl), (void *)(int)tbl[0x29]);

    STUB_printf(D_001E7E28);
    func_00121490(0);
    InitPads();
    func_00123168(func_0012F308);
    D_0016100C = 0x160000;
    InitMemSlots();
    func_00121490(0);
    memcard_Init();
    InitViewContext();
    UpdateViewContext();
    VU1_initChain();
    func_00121490(0);
    Sound_InitState();
    FastMemSet(D_001942C0, 0x80808080, 0x100);
    func_00122630(buf, 0x3FFB, 1, 0, 0, 0, 8, 8);
    func_00118D80(0);
    func_00122958(buf, D_001942C0);
    func_00120858(0, 0);
    LoadDebugFont();
    *(volatile int *)0x10000810 = 0x82;
    *(volatile int *)0x10000800 = 0;
    initialize_sif_rpc();
    buildBitSwapLut();

    lang = func_0012D380();
    switch (lang) {
    case 2:
        D_0015EE88 = 2;
        return;
    case 4:
        D_0015EE88 = 3;
        return;
    case 3:
        D_0015EE88 = 4;
        return;
    case 5:
        D_0015EE88 = 5;
        return;
    default:
        STUB_printf(D_001E7E40, lang);
        /* fallthrough */
    case 1:
        D_0015EE88 = 0;
        return;
    }
}

LINKER_REMNANT("asm/remnants/text", func_00202258);
