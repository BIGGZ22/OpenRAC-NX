/* libmpeg _getRef0: queues the forward/backward reference fetch of one
 * macroblock, luma and chroma, for the IPU/DMA stage.
 *
 * The body below is complete and semantically correct: it compiles to exactly
 * 1052 bytes, the retail size, and check.py aligns it instruction for
 * instruction against retail with no structural divergence - every branch,
 * every load, every store and every struct offset lands at the same address.
 * What differs is register allocation and instruction scheduling:
 *
 *   this cc1 d -> $t9   ref -> $s6   fieldpred -> $t8   dx -> $s0
 *   retail   d -> $t8   ref -> $fp   fieldpred -> $t5   dx -> $s6
 *
 * (In a check.py --diff dump the candidate is the LEFT column and retail the
 * RIGHT one; the candidate's bytes are the ones carrying relocations, so they
 * print as `lui $0` / `jal func_000000`.)
 *
 * The first word already differs: `daddu $t9,$a0,$zero` here against
 * `daddu $t8,$a0,$zero` in retail. Retail keeps the long-lived `ref` in a
 * callee-saved register ($fp) and `fieldpred` in the temp `$t5`; this cc1 does
 * the opposite - `ref` in `$s6`, `fieldpred` in `$t8`. The local stack slots
 * shift with it (work spills to 0x8($sp) here, 0xC($sp) in retail) and the
 * four queue stores at the tail come out in another order.
 * The function contains no call, so this is local-alloc's choice between a
 * temporary and a callee-saved register, not a source shape that can be
 * spelled differently. Tried: the declarations and the statement order follow
 * the upstream file exactly; no profile change applies (the prologue - 0xB0
 * frame, the same ten saved words, the same 0x140/0x1C queue constants -
 * already matches).
 *
 * What would unblock it: a source form for this body whose RTL keeps `dy` in a
 * temporary, or any measured lever that makes local-alloc pick $t9 over $t8
 * for the `d` copy. Compiling this file alone reproduces the diff above.
 *
 * Adapted from Lombyte (MIT), src/assembly/sdk/library/_getRef0.c, commit
 * 5ccfa1fb314e9cb43f6ff9a43249523e1c4a5310, sha256
 * 15078783b09fd6987da0fbab1668ab41e16449c7a2c318a1b0707b60a1b0bab4. Offsets
 * re-measured against RAC2's own accesses: mc 0x590, cur 0x810, work 0x81C,
 * queue n 0x12C, y[4] 0x48, c[4] 0xB8, frame base 0x0 and width 0x10, MC
 * routine tables 0x137ED0 (luma) and 0x137EF0 (chroma).
 */

typedef unsigned char u8;

typedef struct {
    u8 *base; /* 0x00 */
    u8 pad04[0xC];
    int width; /* 0x10 */
} MpegRefImageC;

typedef void (*MpegMcFuncC)(void);

typedef struct {
    u8 *addr;
    int x;
    int n0;
    int n1;
    int step;
    u8 *p0;
    u8 *p1;
} MpegMcFetchC; /* 0x1C */

typedef struct {
    u8 *buf; /* 0x00 */
    int pad04;
    u8 *yref[4]; /* 0x08 */
    u8 *cref[4]; /* 0x18 */
    MpegMcFuncC yfunc[4]; /* 0x28 */
    MpegMcFuncC cfunc[4]; /* 0x38 */
    MpegMcFetchC y[4]; /* 0x48 */
    MpegMcFetchC c[4]; /* 0xB8 */
    int pad128;
    int n; /* 0x12C */
    u8 pad130[0x10];
} MpegMcQueueC; /* 0x140 */

typedef struct {
    u8 pad000[0x590];
    MpegMcQueueC mc[2]; /* 0x590 */
    int cur; /* 0x810 */
    u8 pad814[0x81C - 0x814];
    u8 *work; /* 0x81C */
} MpegDecoderDA8;

extern MpegMcFuncC D_00137ED0[];

extern MpegMcFuncC D_00137EF0[];

void func_00129DA8(MpegDecoderDA8 *d, MpegRefImageC *ref, int sfield, int dfield,
                   int yofs, int h, int bx, int by, int dx, int dy, int fieldpred,
                   int avg) {
    MpegMcFetchC *ye;
    MpegMcFetchC *ce;
    int n;
    u8 *buf;
    int x, y, xm, ym, xr, yr;
    int cx, cy, cxm, cym, cxr, cyr;
    int cdx, cdy;
    int xh, yh;
    int k;
    int mb;
    int ch;
    u8 *work;
    int yidx;

    work = d->work;
    x = bx + (dx >> 1);
    n = d->mc[d->cur].n;
    ye = &d->mc[d->cur].y[n];
    ce = &d->mc[d->cur].c[n];
    if (fieldpred) {
        y = (((dy >> 1) * 2) + by) + (sfield + yofs);
    } else {
        y = (dy >> 1) + by;
        y += sfield + yofs;
    }
    xm = x >> 4;
    mb = xm * ref->width;
    ym = y >> 4;
    mb += ym;
    xr = x - xm * 16;
    yr = y - ym * 16;
    ye->x = xr;
    ye->addr = work + (dfield + yofs) * 32;
    yh = dy & 1;
    xh = dx & 1;
    if (yh) {
        if (15 < yr + (h << fieldpred)) {
            k = (16 >> fieldpred) - (yr >> fieldpred) - 1;
            ye->n0 = k;
            ye->n1 = h - k;
        } else {
            ye->n0 = h;
            ye->n1 = 0;
        }
    } else {
        if (yr + (h << fieldpred) > 16) {
            k = (16 >> fieldpred) - (yr >> fieldpred);
            ye->n0 = k;
            ye->n1 = h - k;
        } else {
            ye->n0 = h;
            ye->n1 = 0;
        }
    }
    buf = d->mc[d->cur].buf + n * 0x600;
    ye->p0 = buf + yr * 16;
    ye->p1 = buf + yr * 16 + 0x300;
    ye->step = 16 << fieldpred;
    yidx = (avg << 2) | xh << 1 | yh;
    avg <<= 2;

    cdy = dy / 2;
    cdx = dx / 2;
    ch = h >> 1;
    cx = (cdx >> 1) + (bx >> 1);
    if (fieldpred) {
        cy = (cdy >> 1) * 2 + (by >> 1);
        cy += (yofs >> 1) + sfield;
    } else {
        cy = (cdy >> 1) + (by >> 1);
        cy += (yofs >> 1) + sfield;
    }
    cxm = cx >> 3;
    cym = cy >> 3;
    cxr = cx - cxm * 8;
    cyr = cy - cym * 8;
    ce->x = cxr;
    ce->addr = work + 0x200 + (dfield + (yofs >> 1)) * 16;
    if ((cdy & 1)) {
        if (cyr + (ch << fieldpred) >= 8) {
            k = (8 >> fieldpred) - (cyr >> fieldpred) - 1;
            ce->n0 = k;
            ce->n1 = ch - k;
        } else {
            ce->n0 = ch;
            ce->n1 = 0;
        }
    } else {
        if (cyr + (ch << fieldpred) > 8) {
            k = (8 >> fieldpred) - (cyr >> fieldpred);
            ce->n0 = k;
            ce->n1 = ch - k;
        } else {
            ce->n0 = ch;
            ce->n1 = 0;
        }
    }
    ce->step = 8 << fieldpred;
    buf = buf + ((cxm - xm) * 2 + (cym - ym)) * 0x180;
    ce->p0 = buf + cyr * 8 + 0x100;
    ce->p1 = buf + cyr * 8 + 0x400;

    d->mc[d->cur].yfunc[n] = D_00137ED0[yidx];
    d->mc[d->cur].cfunc[n] = D_00137EF0[avg | (cdx & 1) << 1 | (cdy & 1)];
    d->mc[d->cur].yref[n] = ref->base + mb * 0x180;
    d->mc[d->cur].cref[n] = ref->base + (mb + ref->width) * 0x180;
    d->mc[d->cur].n++;
}
