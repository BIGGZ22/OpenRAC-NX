#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x122200-0x1224B0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libgraph: graph005.o (sceGszbufaddr, func_00122200) and
 * graph006.o (sceGsSetDefDrawEnv, func_001222C8), back to back. Built with
 * Sony's 2.9-ee (Makefile.sn, EE29_CORE).
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

extern void *func_00121D08(void);

/*
 * sceGszbufaddr-style size: fbw = (w + 63) / 64 pages across; fbh =
 * (h + 63) / 64 for a psm with bit 1 set, else (h + 31) / 32; returned as
 * a short, doubled unless the first 8 bytes of sceGsGetGParam()
 * (func_00121D08) masked with 0x0000FFFF0000FFFF equal 1 (interlace mode
 * 1, field mode 0).
 *
 * The old decode's "clamps" are plain signed divisions by 64 and 32:
 * gcc's own expansion of `x / 64` is the `addiu; slt -1; movn` sequence
 * retail has, where the hand-written clamps always gave slti/movz (192
 * against 200). The GParam test has to be the explicit 64-bit mask (two
 * short field compares fold shorter, SIZE 196). Exact under 2.9-ee with
 * the `== 1` arm first; 2.95.3's best is 8/200, a prologue-order residual.
 */
short func_00122200(short psm, short w, short h) {
    unsigned long *g = func_00121D08();
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
} DrawEnv1_1222C8;

extern short func_00122200(short psm, short w, short h); /* sceGszbufaddr */

/* sceGsSetDefDrawEnv (libgraph.h): fills in a default drawing
 * environment for a psm/w/h framebuffer. zbuf1's page count reuses
 * func_00122200 (the same page-count helper FRAME's own fbw comes from)
 * on the color psm/w/h, since the Z buffer is laid out right after the
 * color buffer; ztest==0 disables Z (mask writes, ZTE/ZTST forced on).
 * prmodecont/colclamp are read-modify-write: only their AC/CLAMP bit is
 * set, whatever else the caller's memory already held is left alone. */
int func_001222C8(DrawEnv1_1222C8 *draw, short psm, short w, short h,
                   short ztest, short zpsm) {
    short fbw = ((w + 63) >> 6) & 0x3F;
    short zbp;

    draw->frame1addr = 0x4C;
    draw->frame1 = ((unsigned long)fbw << 16) | ((unsigned long)(psm & 0xF) << 24);
    draw->zbuf1addr = 0x4E;
    if (ztest == 0) {
        zbp = func_00122200(psm, w, h);
        draw->zbuf1 = (unsigned long)zbp | ((unsigned long)(zpsm & 0xF) << 24) | (1UL << 32);
    } else {
        zbp = func_00122200(psm, w, h);
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

INCLUDE_ASM("asm/nonmatchings/core_text", func_001224AC);
