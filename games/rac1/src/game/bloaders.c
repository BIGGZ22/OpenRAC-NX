#include "common.h"
#include "structs.h"

/*
 * bloaders.cpp in the original source; text 0x1E94E8-0x1E9730.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

typedef struct {
    long q[12];
} GsLoadImage __attribute__((aligned(16))); /* sceGsLoadImage */

typedef struct {
    void *clut;     /* 0x00 */
    void *image;    /* 0x04 */
    int unk08[3];
    int clutSize;   /* 0x14 */
    int imageSize;  /* 0x18 */
    int unk1C[4];
    int tbp;        /* 0x2C */
    int unk30[3];
    int tbw;        /* 0x3C */
    int unk40[3];
    int tw;         /* 0x4C */
    int th;         /* 0x50 */
} PifTex; /* 0x54 */

typedef struct {
    char unk00[8];
    int w;          /* 0x08 */
    int h;          /* 0x0C */
    int unk10;
    int psm;        /* 0x14 */
} PifHeader;

extern void func_001F99B0(void *, int, int);
extern int func_001F9968(int);
/* sceGsSetDefLoadImage, FlushCache, sceGsExecLoadImage, sceGsSyncPath */
extern int func_00122630(GsLoadImage *, short, short, short, short, short, short, short);
extern void func_00118D80(int);
extern int func_00122958(GsLoadImage *, void *);
extern int func_00120858(int, unsigned short);

/* LoadPifAsPSMT8H: uploads a PIF image (header arg0, CLUT at +0x20 of
   0x200 bytes when +0x14 is set, else 0x400, then w * h pixels) to GS
   memory: the 16x16 CLUT to block cbp >> 8, the pixels as PSMT8H to
   tbp >> 8 with a buffer width of w / 64 (at least 1), and writes the
   matching TEX0 (SCE_GS_SET_TEX0 with TCC 1, CLD 4) plus 1, 0 to arg1.
   The descriptor is a zeroed stack struct, so its fields are re-read
   after the calls as retail does. */
void func_001E94E8(void *arg0, void *arg1, int tbp, int cbp) {
    PifTex d;
    GsLoadImage li;
    PifHeader *pif = arg0;
    unsigned long *out = arg1;
    int c;

    FastMemSet(&d, 0, sizeof(d));
    d.clut = (char *)pif + 0x20;
    if (pif->psm == 0) {
        d.clutSize = 0x400;
    } else {
        d.clutSize = 0x200;
    }
    c = cbp >> 8;
    d.tw = log2dim(pif->w);
    d.th = log2dim(pif->h);
    d.image = (char *)pif + (d.clutSize + 0x20);
    d.imageSize = pif->w * pif->h;
    func_00122630(&li, c, 1, pif->psm, 0, 0, 16, 16);
    func_00118D80(0);
    func_00122958(&li, d.clut);
    func_00120858(0, 0);
    d.tbw = pif->w >> 6;
    if (d.tbw <= 0) {
        d.tbw = 1;
    }
    d.tbp = tbp >> 8;
    func_00122630(&li, d.tbp, d.tbw, 0x1B, 0, 0, pif->w, pif->h);
    func_00118D80(0);
    func_00122958(&li, d.image);
    func_00120858(0, 0);
    out[0] = (unsigned long)d.tbp | ((unsigned long)d.tbw << 14)
           | ((unsigned long)0x1B << 20) | ((unsigned long)d.tw << 26)
           | ((unsigned long)d.th << 30) | ((unsigned long)1 << 34)
           | ((unsigned long)0 << 35) | ((unsigned long)c << 37)
           | ((unsigned long)pif->psm << 51) | ((unsigned long)0 << 55)
           | ((unsigned long)0 << 56) | ((unsigned long)4 << 61);
    out[1] = 1;
    out[2] = 0;
}

extern int D_00137C80[];
extern int func_002176C8(void *, int, int);
extern int D_0015EF88;
extern void func_001E94E8(void *arg0, void *arg1, int arg2, int arg3);
extern long D_0015EFC8;
extern unsigned char D_001AAF40[];

extern int D_0015EF88_m __asm__("D_0015EF88") MACRO_ADDR;
extern long D_0015EFC8_m __asm__("D_0015EFC8") MACRO_ADDR;

/*
 * LoadDebugFont: forwards the D_00137C80[2]/[3] stream position to
 * func_002176C8, then loads a PSMT8H texture (D_001AAF40) into VRAM via
 * func_001E94E8, caching the returned descriptor's first field into
 * D_0015EFC8. Retail reaches both globals with assembler macros (MACRO_ADDR
 * aliases). func_002176C8 returns int (its result unused), which puts the
 * D_0015EF88 load in $v1 as retail has it; the level copy is the same C
 * (func_L00_001EB380).
 */
void func_001E96B8(void) {
    long localbuf[3];

    Load(D_001AAF40, D_00137C80[2], D_00137C80[3]);
    LoadPifAsPSMT8H(D_001AAF40, localbuf, D_0015EF88_m + 0xC0000, 0x3FFC00);
    D_0015EFC8_m = localbuf[0];
}

LINKER_REMNANT("asm/remnants/text", func_001E9728);
