/* from src/core/00125E58.c: sceGsSetDefDrawEnv, exact bytes only with an inline sync */
/*
 * Everything else is plain C and reproduces the body; the one barrier at +0x1B8
 * is the retail sync word, and __asm__ __volatile__("sync") does emit it. AGENTS.md
 * forbids inline assembly in a function, so the body stays a near miss and the
 * barrier belongs to the ASM_FUNC route. Measurement kept for the maintainers:
 * none of the ~2900 C functions already in src/ contains 0x0000000F.
 */
#include "common.h"
#include "structs.h"

extern void *r1_func_00121D08(void);

extern short func_00125E58(short psm, short w, short h);

/* rac1-decomp src/core/00122200.c: func_00122200 */
/*
 * sceGszbufaddr-style size: fbw = (w + 63) / 64 pages across; fbh =
 * (h + 63) / 64 for a psm with bit 1 set, else (h + 31) / 32; returned as
 * a short, doubled unless the first 8 bytes of sceGsGetGParam()
 * (r1_func_00121D08) masked with 0x0000FFFF0000FFFF equal 1 (interlace mode
 * 1, field mode 0).
 *
 * The old decode's "clamps" are plain signed divisions by 64 and 32:
 * gcc's own expansion of `x / 64` is the `addiu; slt -1; movn` sequence
 * retail has, where the hand-written clamps always gave slti/movz (192
 * against 200). The GParam test has to be the explicit 64-bit mask (two
 * short field compares fold shorter, SIZE 196). Exact under 2.9-ee with
 * the `== 1` arm first; 2.95.3's best is 8/200, a prologue-order residual.
 */
short func_00125E58(short psm, short w, short h) {
    unsigned long *g = r1_func_00121D08();
    int fbw, fbh;

    fbw = (w + 63) / 64;
    if (psm & 2) {
        fbh = (h + 63) / 64;
    } else {
        fbh = (h + 31) / 32;
    }
    if ((*g & 0x0000FFFF0000FFFFUL) == 1) {
        return fbw * fbh;
    }
    return fbw * fbh * 2;
}

/* Sony's sceGsDrawEnv1 (libgraph.h): eight (value, GS-register-address)
 * pairs, all 64-bit ("long" on this ABI). Offsets confirmed against the
 * SET_FRAME/SET_ZBUF/SET_XYOFFSET/SET_SCISSOR/SET_TEST bit layouts and
 * the FRAME_1/ZBUF_1/XYOFFSET_1/SCISSOR_1/PRMODECONT/COLCLAMP/DTHE/
 * TEST_1 register ids in eestruct.h. */
typedef struct {
    unsigned long frame1, frame1addr;
    unsigned long zbuf1, zbuf1addr;
    unsigned long xyoffset1, xyoffset1addr;
    unsigned long scissor1, scissor1addr;
    unsigned long prmodecont, prmodecontaddr;
    unsigned long colclamp, colclampaddr;
    unsigned long dthe, dtheaddr;
    unsigned long test1, test1addr;
} DrawEnv1_125F20;

/* rac1-decomp src/core/00122200.c: func_001222C8 */
/* sceGsSetDefDrawEnv (libgraph.h), graph006.o: fills in a default drawing
 * environment for a psm/w/h framebuffer. zbuf1's page count reuses
 * func_00125E58 (the same page-count helper FRAME's own fbw comes from) on
 * the color psm/w/h, since the Z buffer is laid out right after the color
 * buffer; ztest==0 disables Z (mask writes, ZTE/ZTST forced on).
 * prmodecont/colclamp are read-modify-write: only their AC/CLAMP bit is
 * set, whatever else the caller's memory already held is left alone.
 *
 * rac1 writes the closing barrier as `__asm__ __volatile__("sync")`; the
 * `sync` at +0x1B8 is that statement, not a missing C construct. */
int func_00125F20(DrawEnv1_125F20 *draw, short psm, short w, short h,
                  short ztest, short zpsm) {
    short fbw = ((w + 63) >> 6) & 0x3F;
    short zbp;

    draw->frame1addr = 0x4C;
    draw->frame1 = ((unsigned long)fbw << 16) | ((unsigned long)(psm & 0xF) << 24);
    draw->zbuf1addr = 0x4E;
    if (ztest == 0) {
        zbp = func_00125E58(psm, w, h);
        draw->zbuf1 = (unsigned long)zbp | ((unsigned long)(zpsm & 0xF) << 24) | (1UL << 32);
    } else {
        zbp = func_00125E58(psm, w, h);
        draw->zbuf1 = (unsigned long)zbp | ((unsigned long)(zpsm & 0xF) << 24);
    }
    draw->xyoffset1addr = 0x18;
    {
        long ofx = 2048L - (w >> 1);
        long ofy = 2048L - (h >> 1);
        draw->xyoffset1 = (unsigned long)(ofx << 4) |
                          ((unsigned long)(ofy << 4) << 32);
    }
    draw->scissor1addr = 0x40;
    draw->scissor1 = ((unsigned long)(w - 1) << 16) | ((unsigned long)(h - 1) << 48);
    draw->prmodecontaddr = 0x1A;
    draw->prmodecont |= 1;
    draw->colclampaddr = 0x46;
    draw->colclamp |= 1;
    draw->dtheaddr = 0x45;
    if (psm & 2) {
        draw->dthe |= 1;
    } else {
        draw->dthe &= ~1UL;
    }
    draw->test1addr = 0x47;
    if (ztest != 0) {
        draw->test1 = ((unsigned long)(ztest & 3) << 17) | 0x10000;
    } else {
        draw->test1 = 0x30000;
    }
    __asm__ __volatile__("sync");
    return 8;
}

INCLUDE_ASM("asm/core", func_00126104);
