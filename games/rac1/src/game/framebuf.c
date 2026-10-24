#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * framebuf.cpp in the original source; text 0x1FAA28-0x1FBC80.
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

ASM_FUNC("asm/handwritten/text", func_001FAA28);

LINKER_REMNANT("asm/remnants/text", func_001FAB20);

typedef unsigned int u128_FAB40 __attribute__((mode(TI)));

typedef struct {
    unsigned long NLOOP:15;
    unsigned long EOP:1;
    unsigned long pad16:16;
    unsigned long id:14;
    unsigned long PRE:1;
    unsigned long PRIM:11;
    unsigned long FLG:2;
    unsigned long NREG:4;
    unsigned long REGS0:4;
    unsigned long REGS1:4;
    unsigned long REGS2:4;
    unsigned long REGS3:4;
    unsigned long REGS4:4;
    unsigned long REGS5:4;
    unsigned long REGS6:4;
    unsigned long REGS7:4;
    unsigned long REGS8:4;
    unsigned long REGS9:4;
    unsigned long REGS10:4;
    unsigned long REGS11:4;
    unsigned long REGS12:4;
    unsigned long REGS13:4;
    unsigned long REGS14:4;
    unsigned long REGS15:4;
} GifTag_FAB40;

typedef struct {
    unsigned long FBP:9;
    unsigned long rest:55;
} GsFbp_FAB40;

typedef struct {
    unsigned long pmode;
    unsigned long smode2;
    GsFbp_FAB40 dispfb;
    unsigned long display;
    unsigned long bgcolor;
    unsigned long pad028;
    GifTag_FAB40 giftag0;       /* 0x030 */
    GsFbp_FAB40 frame0;         /* 0x040 */
    unsigned long draw0[15];
    GifTag_FAB40 giftag1;       /* 0x0C0 */
    GsFbp_FAB40 frame1;         /* 0x0D0 */
    unsigned long draw1[15];
    short display_width;        /* 0x150 */
    short display_height;
    short psm;
    short fbp0;
    short storage_width;        /* 0x158 */
    short storage_height;
    short storage_psm;
    short fbp1;
    short target_width;         /* 0x160 */
    short target_height;
    short target_psm;
    short target_fbp;
    short display_offset_x;     /* 0x168 */
    short display_offset_y;
    short zpsm;
    short zbp;
    int reserved170;            /* 0x170 */
} FsAa_FAB40;

extern FsAa_FAB40 D_00151880;
extern void *D_0015EFB8 MACRO_ADDR;
extern int D_0015EF80 MACRO_ADDR;
extern int D_0015EF84 MACRO_ADDR;
extern int D_0015EF88 MACRO_ADDR;
extern char D_0013CD90[];
extern char D_0013CED0[];
extern char D_00151A00[];
extern char D_00151C60[];
extern char D_00151EF0[];
extern char D_00152140[];
extern void func_00121DC8(void *, short, short, short, short, short);
extern int func_001222C8(void *, short, short, short, short, short);

#define FA ((FsAa_FAB40 *)D_0015EFB8)

void SetupFS_AA_buffer(int display_width, int display_height, int storage_width, int storage_height,
                   int display_offset_x, int display_offset_y) {
    unsigned long *q;

    active_fs_aa_buffer = &D_00151880;
    D_00151880.display_width = display_width;
    D_00151880.display_height = display_height;
    D_00151880.storage_width = storage_width;
    D_00151880.storage_height = storage_height;
    D_00151880.display_offset_x = display_offset_x;
    D_00151880.display_offset_y = display_offset_y;
    D_00151880.reserved170 = 0;
    D_00151880.storage_psm = 0;
    D_00151880.psm = 0;
    D_00151880.target_psm = 0;
    D_00151880.zpsm = 0x31;
    D_00151880.fbp1 = D_0015EF80 >> 13;
    D_00151880.fbp0 = D_0015EF84 >> 13;
    D_00151880.zbp = D_0015EF88 >> 13;
    func_00121DC8(&D_00151880, 0, storage_width, storage_height, display_offset_x,
                  display_offset_y);
    FA->dispfb.FBP = D_00151880.fbp1;
    func_001222C8(&FA->frame0, D_00151880.psm, D_00151880.display_width,
                  D_00151880.display_height, 3, D_00151880.zpsm);
    FA->frame0.FBP = D_00151880.fbp0;
    *(unsigned long *)&FA->draw0[1] =
        (long)D_00151880.zbp | ((unsigned long)(D_00151880.zpsm & 0xF) << 24);
    *(u128_FAB40 *)&FA->giftag0 = 0;
    FA->giftag0.NLOOP = 8;
    FA->giftag0.EOP = 1;
    FA->giftag0.NREG = 1;
    FA->giftag0.REGS0 = 0xE;
    func_001222C8(&FA->frame1, D_00151880.storage_psm, D_00151880.storage_width,
                  D_00151880.storage_height, 0, 0);
    FA->frame1.FBP = D_00151880.fbp1;
    *(unsigned long *)&FA->draw1[1] = 1UL << 32;
    *(u128_FAB40 *)&FA->giftag1 = 0;
    FA->giftag1.NLOOP = 8;
    FA->giftag1.EOP = 1;
    FA->giftag1.NREG = 1;
    FA->giftag1.REGS0 = 0xE;

    q = (unsigned long *)fs_aa_transfer_packet;
    q[0] = 0x408B400000000001UL;
    q[1] = 0xEEEE;
    q[2] = 0x30000;
    q[3] = 0x47;
    q[4] = 5;
    q[5] = 8;
    q[6] = 0x100000261UL;
    q[7] = 0x14;
    q[8] = ((long)FA->fbp0 << 5) | ((unsigned long)((FA->display_width >> 6) & 0x3F) << 14) |
           ((long)FA->psm << 20) | 0xEA8000000UL;
    q[9] = 6;
    q[10] = 0x4400000000008010UL;
    q[11] = 0x5353;
    {
        unsigned long *p;
        int i;
        for (i = 0, p = q + 12; i < 16; i++) {
            *p++ = i * FA->display_width;
            *p++ = (i * FA->storage_width + (0x8000 - FA->storage_width * 8)) |
                   ((long)(0x7FF8 - FA->storage_height * 8) << 16);
            *p++ = (i + 1) * FA->display_width | ((long)FA->display_height << 20);
            *p++ = ((i + 1) * FA->storage_width + (0x8000 - FA->storage_width * 8)) |
                   ((long)(FA->storage_height * 8 + 0x7FF8) << 16);
        }
    }
    q[76] = 0x4400000000008001UL;
    q[77] = 0x4410;
    q[78] = 0x181;
    q[79] = 0x80000000UL;
    q[80] = 0x6FF8 | ((long)(0x7FF8 - FA->storage_height * 8) << 16);
    q[81] = 0x6FF8 | ((long)(FA->storage_height * 8 + 0x7FF8) << 16);

    q = (unsigned long *)fs_aa_resample_packet;
    q[0] = 0x308B400000000001UL;
    q[1] = 0xEEE;
    q[2] = 0x30000;
    q[3] = 0x47;
    q[4] = 0x100000261UL;
    q[5] = 0x14;
    q[6] = ((long)FA->fbp0 << 5) | ((unsigned long)((FA->display_width >> 6) & 0x3F) << 14) |
           ((long)FA->psm << 20) | 0xEA8000000UL;
    q[7] = 6;
    q[8] = 0x4400000000008010UL;
    q[9] = 0x5353;
    {
        unsigned long *p = q + 10;
        int i;
        for (i = 0; i < 16; i++) {
            *p++ = i * FA->display_width;
            *p++ = (i * 0x200 + 0x6FF8) | ((long)(0x7FF8 - FA->storage_height * 8) << 16);
            *p++ = (i + 1) * FA->display_width | 0x1A000000;
            *p++ = (i * 0x200 + 0x71F8) | ((long)(FA->storage_height * 8 + 0x7FF8) << 16);
        }
    }

    q = (unsigned long *)fs_aa_draw_packet;
    q[0] = 0x408B400000000001UL;
    q[1] = 0xEEEE;
    q[2] = 0x30000;
    q[3] = 0x47;
    q[4] = 5;
    q[5] = 8;
    q[6] = 0x100000261UL;
    q[7] = 0x14;
    q[8] = ((long)FA->fbp0 << 5) | ((unsigned long)((FA->display_width >> 6) & 0x3F) << 14) |
           ((long)FA->psm << 20) | 0xEA8000000UL;
    q[9] = 6;
    q[10] = 0x4400000000008010UL;
    q[11] = 0x5353;
    {
        unsigned long *p = q + 12;
        int i;
        for (i = 0; i < 16; i++) {
            *p++ = i * 0x200;
            *p++ = (i * 0x200 + 0x7000) | ((long)(0x8000 - FA->display_height * 8) << 16);
            *p++ = (i * 0x200 + 0x200) | ((long)FA->display_height << 20);
            *p++ = (i * 0x200 + 0x7200) | ((long)(FA->display_height * 8 + 0x7FF0) << 16);
        }
    }

    q = (unsigned long *)fs_aa_clear_packet;
    q[0] = 0x1000000000000001UL;
    q[1] = 0xE;
    q[2] = 0x30000;
    q[3] = 0x47;
    q[4] = 0x2400000000008001UL;
    q[5] = 0x10;
    q[6] = 0x106;
    q[7] = 0x80008000UL;
    q[8] = 0x2400000000008010UL;
    q[9] = 0x44;
    {
        unsigned long *p = q + 10;
        int i;
        for (i = 0; i < 16; i++) {
            *p++ = (i * 0x200 + 0x6FF8) | ((long)(0x7FF8 - FA->display_height * 8) << 16);
            *p++ = (i * 0x200 + 0x71F8) | ((long)(FA->display_height * 8 + 0x7FF8) << 16);
        }
    }

    q = (unsigned long *)first_clear_packet;
    {
        unsigned long *p = q + 8;
        int i;
        for (i = 0; i < 16; i++) {
            *p++ = (i * 0x200 + 0x6FF8) | ((long)(0x7FF8 - FA->display_height * 8) << 16);
            *p++ = (i * 0x200 + 0x71F8) | ((long)(FA->display_height * 8 + 0x7FF8) << 16);
        }
    }

    q = (unsigned long *)second_clear_packet;
    {
        unsigned long *p = q + 8;
        int i;
        for (i = 0; i < 16; i++) {
            *p++ = (i * 0x200 + 0x6FF8) | ((long)(0x7FF8 - FA->storage_height * 8) << 16);
            *p++ = (i * 0x200 + 0x71F8) | ((long)(FA->storage_height * 8 + 0x7FF8) << 16);
        }
    }
}

extern long D_00152178 NOT_SDA;

void SetBackgroundColor(int arg0, int arg1, int arg2) {
    D_00152178 = (long)arg0 | ((long)arg1 << 8) | ((long)arg2 << 16) | 0x80000000L;
}

extern void *D_0015EFB8 MACRO_ADDR;
extern void func_00122140(void *);

/* PutDispBuffer(void) */
void PutDispBuffer(void) {
    func_00122140(active_fs_aa_buffer);
}

extern int *D_00161000 MACRO_ADDR;
extern void func_001224B0(void *);

/* PutDrawBufferLarge: queue the draw environment at D_0015EFB8+0x30 as a
   DMA ref tag when a packet buffer is open, else send it directly. */
void PutDrawBufferLarge(void) {
    if (D_00161000 != 0) {
        D_00161000[0] = 0x30000009;
        D_00161000[1] = ((int)active_fs_aa_buffer + 0x30) & 0xFFFFFFF;
        D_00161000[2] = 0;
        D_00161000[3] = 0x50000009;
        D_00161000 += 4;
    } else {
        func_001224B0((char *)active_fs_aa_buffer + 0x30);
    }
}

extern char D_00152140[];

/* Appends a DMA tag pair (0x30000015 / &D_00152140 / 0 / 0x50000015) to
   the packet buffer D_00161000 points at, if there is one. */
void append_gif_transfer_packet(void) {
    if (D_00161000 != 0) {
        D_00161000[0] = 0x30000015;
        D_00161000[1] = (int)fs_aa_clear_packet;
        D_00161000[2] = 0;
        D_00161000[3] = 0x50000015;
        D_00161000 += 4;
    }
}

/* PutDrawBufferSmall(void) */
void PutDrawBufferSmall(void) {
    D_00161000[0] = 0x30000009;
    D_00161000[1] = ((int)active_fs_aa_buffer + 0xC0) & 0xFFFFFFF;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000009;
    D_00161000 += 4;
}

typedef struct {
    char unk_000[0x160];
    short w;        /* 0x160 */
    short h;        /* 0x162 */
    short psm;      /* 0x164 */
    short fbp;      /* 0x166 */
    short unk_168[2];
    short zpsm;     /* 0x16C */
    short zbp;      /* 0x16E */
} FrameCfg;
extern FrameCfg D_00151880_f __asm__("D_00151880");
extern long D_0015EFD0 MACRO_ADDR;
/* sceGsSetDefDrawEnv */
extern int func_001222C8(void *, short, short, short, short, short);

typedef struct {
    unsigned long FBP:9;
    unsigned long pad09:7;
    unsigned long FBW:6;
    unsigned long pad22:2;
    unsigned long PSM:6;
    unsigned long pad30:2;
    unsigned long FBMSK:32;
} GsFrame; /* sceGsFrame */

/* Sets up a (1 << a) x (1 << b) render target at GS address c (see
   func_0023A948): records TEX0 for it in D_0015EFD0 (buffer width
   1 << max(a - 6, 1) pages, PSMCT32, TCC), its size and FBP (c >> 13) in
   D_00151880, then appends a packet to D_00161000: a default draw
   environment (sceGsSetDefDrawEnv with ztest 3) patched with the FBP and
   the configured Z buffer, followed by TEST_1 0x30003 and a black sprite
   over the whole target. FBP is set through the sceGsFrame bitfield, the
   form that sign-extends the halfword before masking as retail does. */
void setup_render_target(int a, int b, int c) {
    int x;
    long *q;
    unsigned long *d;
    long *r;

    x = a - 6;
    if (x <= 0) {
        x = 1;
    }
    D_00151880_f.fbp = c >> 13;
    D_00151880_f.w = 1 << a;
    D_00151880_f.h = 1 << b;
    D_0015EFD0 = (unsigned long)(c >> 8) | ((unsigned long)(1 << x) << 14)
               | ((unsigned long)0 << 20) | ((unsigned long)a << 26)
               | ((unsigned long)b << 30) | ((unsigned long)1 << 34);
    FastMemZero16(D_00161000, 0xF0);
    D_00161000[0] = 0x1000000E;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x5000000E;
    D_00161000 += 4;
    q = (long *)D_00161000;
    q[0] = 0x1000000000000008L;
    q[1] = 0xE;
    D_00161000 += 4;
    d = (unsigned long *)D_00161000;
    func_001222C8(d, D_00151880_f.psm, D_00151880_f.w, D_00151880_f.h, 3, 0);
    ((GsFrame *)d)->FBP = D_00151880_f.fbp;
    d[2] = D_00151880_f.zbp | ((unsigned long)(D_00151880_f.zpsm & 0xF) << 24);
    D_00161000 += 0x20;
    r = (long *)D_00161000;
    r[0] = 0x1000000000000001L;
    r[1] = 0xE;
    r[2] = 0x30003;
    r[3] = 0x47;
    r[4] = 0x4400000000008001L;
    r[5] = 0x4410;
    r[6] = 0x106;
    r[7] = 0;
    r[8] = (long)(0x8000 - D_00151880_f.w * 8) | ((long)(0x8000 - D_00151880_f.h * 8) << 16);
    r[9] = (long)(D_00151880_f.w * 8 + 0x8000) | ((long)(D_00151880_f.h * 8 + 0x7FF0) << 16);
    D_00161000 += 0x14;
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern char D_00151A00[];

/* AA_BlurPass(void) */
void AA_BlurPass(void) {
    D_00161000[0] = 0x30000026;
    D_00161000[1] = (int)fs_aa_draw_packet;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000026;
    D_00161000 += 4;
}

extern char D_00151C60[];

void append_palette_transfer_packet(void) {
    D_00161000[0] = 0x30000029;
    D_00161000[1] = (int)fs_aa_transfer_packet;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000029;
    D_00161000 += 4;
}

extern void func_00234C98_l(int, long) __asm__("func_00234C98");

/* Clears the screen through a GIF packet appended to D_00161000: a fixed header then n = w / 32 pairs of sprite corner registers stepping 0x200 per column across a w x h area centred on 0x8000. Adapted from Lombyte (GPL-3.0) for PAL: src/rendering/fun_001fb740.c, FUN_001fb740. */
void draw_centered_capture_background(int w, int h) {
    int *tag;
    long *q;
    long *c;
    int n;
    int i;
    int zero;
    int step;
    int x;
    int y;
    long lo;
    long hi;

    n = w / 32;
    func_00234C98_l(0x42, 0x800000004AL);
    D_00161000[0] = (n + 5) | 0x10000000;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = (n + 5) | 0x50000000;
    tag = D_00161000;
    q = (long *)(tag + 4);
    D_00161000 = tag + 4;
    zero = 0;
    q[zero] = 0x1000000000000001L;
    q[1] = 0xE;
    q[2] = 0x32003;
    q[3] = 0x47;
    q[4] = 0x2400000000000001L;
    q[5] = 0x10;
    q[6] = 0x146;
    q[7] = 0x80008080L;
    q[8] = (long)(n | 0x8000) | 0x2400000000000000L;
    q[9] = 0x44;
    i = zero;
    if (n > zero) {
        lo = (long)(0x8000 - h * 8) << 16;
        x = 0x8000 + -(w * 8);
        c = (long *)((char *)tag + 0x60);
        y = 0x8200 + -(w * 8);
        hi = (long)(h * 8 + 0x7FF0) << 16;
        do {
            step = 0x200;
            *c++ = x | lo;
            *c++ = y | hi;
            y += step;
            x += step;
            i++;
        } while (i < n);
    }
    D_00161000 = D_00161000 + (n * 4 + 20);
}

extern void func_00234C50(int);
extern void func_00234C98(int, long);

/* Draws an alpha-blended sprite through a GIF packet appended to
   D_00161000 (a DMA/VIF header, TEST_1 0x33003, then PRIM 0x106 with
   RGBAQ rgba and two XYZ2 corners), with ALPHA_1 (GS register 0x42) set
   to 0x64 around it by func_00234C98 and a VIF flush (func_00234C50,
   0x13000000) on each side. The corners are (x << 4) + 0x8000 minus
   half the w/h extent, in 12.4 fixed point. The int sums are kept in their own locals
   (fold would otherwise share the 0x8000 with the extent), and the four
   64-bit corner values are converted in the order that leaves retail's
   last uses, which the scheduler follows. */
void draw_framebuffer_rect(int x0, int y0, int x1, int y1, int w, int h, unsigned int rgba) {
    long *q;
    int ax, ay, bx, by;
    int hw, hh;

    vu1_add_vif_code(0x13000000);
    VU1_addGSregister(0x42, 0x64);
    ax = (x0 << 4) + 0x8000;
    ay = (y0 << 4) + 0x8000;
    bx = (x1 << 4) + 0x8000;
    by = (y1 << 4) + 0x8000;
    hw = w << 3;
    hh = h << 3;
    D_00161000[0] = 0x10000006;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000006;
    D_00161000 += 4;
    q = (long *)D_00161000;
    q[0] = 0x1000000000000001L;
    q[1] = 0xE;
    q[2] = 0x33003;
    q[3] = 0x47;
    q[4] = 0x2400000000000001L;
    q[5] = 0x10;
    q[6] = 0x106;
    q[7] = rgba;
    q[8] = 0x2400000000008001L;
    q[9] = 0x44;
    {
        long X1 = bx - hw;
        long X0 = ax - hw;
        long Y0 = ay - hh;
        long Y1 = by - hh;

        q[10] = X0 | (Y0 << 16);
        q[11] = X1 | (Y1 << 16);
    }
    D_00161000 += 0x18;
    VU1_addGSregister(0x42, 0x8000000044L);
    vu1_add_vif_code(0x13000000);
}

LINKER_REMNANT("asm/remnants/text", func_001FBC78);
