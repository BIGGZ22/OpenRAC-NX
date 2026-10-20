#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x121D18-0x122140. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libgraph, graph003.o (checkModelVersion, _GetGsDxDyOffset,
 * sceGsSetDefDispEnv). Built with Sony's 2.9-ee (Makefile.sn,
 * EE29_CORE), like the prebuilt libgraph.a.
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

extern int func_0011BF80(void *path, int mode);
extern int func_0011C5C0(int fd, void *buf, int len);
extern void func_0011C208(int fd);
extern int func_00112380(void *arg0);
extern char D_00153130[];

/* Read a small text file a byte at a time into a 0x100-byte stack buffer,
 * stopping at a NUL or the buffer limit, then strtol() the tail starting
 * 9 bytes before the end and compare it against a build number. Same
 * open/read/close triplet as func_0012D2A0 above. */
int func_00121D18(void) {
    int fd;
    char buf[0x100];
    char *p;
    unsigned int n;

    fd = func_0011BF80(D_00153130, 1);
    if (fd < 0) {
        return -1;
    }
    n = 0;
    p = buf;
    while (n < 0x100) {
        func_0011C5C0(fd, p, 1);
        if (*p++ == 0) {
            break;
        }
        n++;
    }
    func_0011C208(fd);
    return 0x1315670 < func_00112380(&buf[n - 9]);
}

ASM_FUNC("asm/handwritten/core_text", func_00121DB8);

typedef struct {
    short interMode;
    short outMode;
    short ffMode;
    short version;
} GsGParam;

typedef struct {
    unsigned long pmode;
    unsigned long smode2;
    unsigned long dispfb;
    unsigned long display;
    unsigned long bgcolor;
} GsDispEnv;

extern GsGParam *func_00121D08(void);
extern void func_00121DB8(int, int *, int *, int *, int *);
extern char D_00153140[];

void func_00121DC8(GsDispEnv *disp, short psm, short w, short h, short dx, short dy) {
    GsGParam *gp;
    int dx1, dy1, dx2, dy2;
    int mode;
    short omode;
    unsigned short imode;

    gp = func_00121D08();
    if (gp->outMode != 2 && gp->outMode != 3 && func_00121D18() != 0) {
        func_00121DB8(gp->outMode, &dx1, &dy1, &dx2, &dy2);
    } else {
        dy2 = 0;
        dx2 = 0;
        dy1 = 0;
        dx1 = 0;
    }
    imode = gp->interMode;
    disp->pmode = 0x66;
    if (gp->interMode != 0) {
        if (gp->ffMode != 0) {
            disp->smode2 = 3;
        } else {
            disp->smode2 = 1;
        }
    } else {
        disp->smode2 = 2;
    }
    disp->dispfb = ((unsigned long)(psm & 0xF) << 15) |
                   ((unsigned long)(((w + 63) >> 6) & 0x3F) << 9);
    omode = gp->outMode;
    if (omode == 2) {
        if ((short)imode == 1) {
            int magh = (w + 2559) / w;
            unsigned long dxo = (unsigned long)dx1 + 636;
            disp->display = ((((unsigned long)(magh - 1) << 23) |
                            (((dx * magh + dxo) & 0xFFF) |
                             ((unsigned long)(gp->ffMode ? (h << 1) - 1 : h - 1) << 44))) |
                            (((unsigned long)(dy + dy1 + 50) & 0xFFF) << 12)) |
                            ((unsigned long)(magh * w - 1) << 32);
        } else {
            int magh = (w + 2559) / w;
            unsigned long dxo = (unsigned long)dx1 + 636;
            magh = (w + 2559) / w;
            disp->display = ((((unsigned long)((w + 2559) / w - 1) << 23) |
                            (((dx * ((w + 2559) / w) + dxo) & 0xFFF) |
                             ((unsigned long)(h - 1) << 44))) |
                            (((unsigned long)(dy + dy1 + 25) & 0xFFF) << 12)) |
                            ((unsigned long)((w + 2559) / w * w - 1) << 32);
        }
    } else if (omode == 3) {
        if ((short)imode == 1) {
            int magh = (w + 2559) / w;
            unsigned long dxo = (unsigned long)dx1 + 656;
            disp->display = ((((unsigned long)(magh - 1) << 23) |
                            (((dx * magh + dxo) & 0xFFF) |
                             ((unsigned long)(gp->ffMode ? (h << 1) - 1 : h - 1) << 44))) |
                            (((unsigned long)(dy + dy1 + 72) & 0xFFF) << 12)) |
                            ((unsigned long)(magh * w - 1) << 32);
        } else {
            int magh = (w + 2559) / w;
            unsigned long dxo = (unsigned long)dx1 + 656;
            magh = (w + 2559) / w;
            disp->display = ((((unsigned long)((w + 2559) / w - 1) << 23) |
                            (((dx * ((w + 2559) / w) + dxo) & 0xFFF) |
                             ((unsigned long)(h - 1) << 44))) |
                            (((unsigned long)(dy + dy1 + 36) & 0xFFF) << 12)) |
                            ((unsigned long)((w + 2559) / w * w - 1) << 32);
        }
    } else if (omode == 0x50) {
        unsigned long dxo = (unsigned long)(dx * 2) + 232;
        disp->display = ((((unsigned long)(h - 1) << 44) |
                          ((((720 - w) / 2 * 2 + (unsigned long)dx1 + dxo) & 0xFFF) | 0x800000)) |
                         (((unsigned long)(dy + dy1 + 35) & 0xFFF) << 12)) |
                        ((unsigned long)(w * 2 - 1) << 32);
    } else {
        func_0011A6C8(D_00153140);
    }
    disp->bgcolor = 0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012213C);
