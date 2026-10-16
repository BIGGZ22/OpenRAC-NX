#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x1224B0-0x122630. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libgraph: graph008.o (sceGsPutDrawEnv) and graph011.o
 * (sceGsSyncV), back to back. Built with Sony's 2.9-ee (Makefile.sn,
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

extern char D_00153178[];

/* Kicks a DMA transfer on the GIF channel (D2 at 0x1000A000), waiting
   for the channel with a timeout. Every register access is volatile. */
int func_001224B0(void *arg0) {
    int p = (int)arg0;
    unsigned int i;

    i = 0;
    while (*(volatile int *)0x1000A000 & 0x100) {
        if (i++ > 0x1000000) {
            func_0011A6C8(D_00153178);
            return -1;
        }
    }
    *(volatile int *)0x1000A020 = (int)(*(long *)arg0 & 0x7FFF) + 1;
    if ((p & 0x70000000) == 0x70000000) {
        *(volatile int *)0x1000A010 = (p & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile int *)0x1000A010 = p & 0x0FFFFFFF;
    }
    *(volatile int *)0x1000A000 = 0x101;
    return 0;
}

/* libgraph's sceGsGParam, as far as this reads it. */
typedef struct {
    short interlace; /* 0x00 */
    short omode;     /* 0x02 */
    short ffmode;    /* 0x04 */
    short version;   /* 0x06 */
    int unk_08;      /* 0x08 */
} GsGParam;
extern GsGParam *func_00121D08(void);
extern void func_00118ED0(void);
extern long func_00118F60(void);

/* Returns the current field, bit 13 of the GS CSR (read directly at
   0x12001000, or through func_00118F60), when the display is
   interlaced, and 1 otherwise. In the second arm the bit is taken from
   the call's result before the interlace test, as retail does. */
int func_00122598(void) {
    GsGParam *gp = func_00121D08();
    long csr;

    if (gp->unk_08 == 0) {
        func_00118ED0();
        if (gp->interlace != 1) {
            return 1;
        }
        return (*(volatile unsigned long *)0x12001000 >> 13) & 1;
    }
    csr = (func_00118F60() >> 13) & 1;
    if (gp->interlace != 1) {
        return 1;
    }
    return csr;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012262C);
