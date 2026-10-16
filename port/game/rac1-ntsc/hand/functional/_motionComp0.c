/* Carried from the PAL decompilation's src/core/00125630.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern void func_0012C2F0(void *, char *, int);
extern void func_00125740(struct MpegDecoder *d, int bx, int by, int mb_type, int motion_type,
                          int pmv[2][2][2], int mvfs[2][2], int *dmvector);
extern void func_00129FB8(void *);
extern char D_001536E8[];
typedef struct {
    char pad0[0x128];
    int unk128;
    int unk12C;
    int unk130;
    int unk134;
    int unk138;
    int unk13C;
} Elem00125630_cmotioncomp0;
typedef struct {
    int unk0;
    int pad4[3];
    int unk10;
} Ref00125630_cmotioncomp0;
typedef struct {
    char pad0[0x11C];
    int unk11C;
    char pad120[0xC];
    int unk12C;
    char pad130[0x44];
    int unk174;
    char pad178[0x48];
    Ref00125630_cmotioncomp0 *unk1C0;
    char pad1C4[0xC];
    Ref00125630_cmotioncomp0 *unk1D0;
    char pad1D4[0xC];
    Ref00125630_cmotioncomp0 *unk1E0;
    char pad1E4[0x3AC];
    Elem00125630_cmotioncomp0 e[2];
    int unk810;
} Dec00125630_cmotioncomp0;



/* Motion-compensate one macroblock (or wait for the intra IDCT) and record its output slot. */
int _motionComp0(void *arg0, int mbAddr, int arg2, int mbType, int motionType,
                  void *pmv, void *mvfs, void *dmv) {
    Dec00125630_cmotioncomp0 *d = (Dec00125630_cmotioncomp0 *)arg0;
    int intra;
    int x;
    int y;
    int bx;
    int by;
    Ref00125630_cmotioncomp0 *ref;

    intra = mbType & 1;
    y = mbAddr / d->unk12C;
    x = mbAddr % d->unk12C;
    by = y * 16;
    bx = x * 16;
    if (intra) {
        while ((*(volatile unsigned int *)0x1000D400 >> 8) & 1)
            ;
        d->e[d->unk810].unk138 = 0;
    } else {
        if ((unsigned int)(motionType - 1) >= 3) {
            func_0012C2F0(d, D_001536E8, motionType);
            d->unk11C = 1;
            return 0;
        }
        func_00125740((struct MpegDecoder *)d, bx, by, mbType, motionType,
                      pmv, mvfs, dmv);
        while ((*(volatile unsigned int *)0x1000D400 >> 8) & 1)
            ;
        func_00129FB8(d);
        d->e[d->unk810].unk138 = 1;
    }
    if (arg2 == 1 && (mbType & 2)) {
        d->e[d->unk810].unk134 = arg2;
    } else {
        d->e[d->unk810].unk134 = 0;
    }
    d->e[d->unk810].unk130 = intra;
    if (d->unk174 == 3) {
        d->e[d->unk810].unk128 = d->unk1C0->unk0 + (x * d->unk1C0->unk10 + y) * 0x180;
    } else {
        if (d->unk174 == 2) {
            ref = d->unk1E0;
        } else {
            ref = d->unk1D0;
        }
        d->e[d->unk810].unk128 = ref->unk0 + (x * ref->unk10 + y) * 0x180;
    }
    return 1;
}
