#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x11D960-0x11DBE8. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl): diei.o (DIntr, EIntr) and
 * initsys.o (supplement_crt0, kCopy, kFindAddress, FindAddress,
 * InitSystemCallTableAddress, _InitSys). Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE), like the prebuilt libkernl.a.
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

ASM_FUNC("asm/handwritten/core_text", func_0011D960);

ASM_FUNC("asm/handwritten/core_text", func_0011D9A8);

extern int func_00118C70(void *);
extern int D_00130420;
extern int D_00130424;

void func_0011D9C0(void) {
    int a[8];
    int b[8];
    a[1] = 1;
    a[2] = 1;
    b[1] = 1;
    b[2] = 1;
    FirstSemaphore = func_00118C70(a);
    SecondSemaphore = func_00118C70(b);
}

/*
 * initsys.o's kCopy: a word-at-a-time copy of n bytes (rounded down to
 * words); returns 0. The same function is func_0011D370 (alarm.o), and
 * func_0011DC08/func_0011DDF0 in the other kernel modules.
 *
 * Exact under 2.9-ee: the empty delay slot of the loop branch is
 * 2.9-ee's own short-loop padding (it pads the loop and leaves the bnez
 * unfilled). Under 2.95.3 reorg sinks `dst++` into the slot, 52 bytes
 * against 56, which is what kept this a stub.
 */
int func_0011DA08(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

/* kFindAddress (libkernl.a:initsys.o): scan the words from p up to, not
 * including, end for val; returns the matching address, or 0 if the scan
 * reaches end. */
int *func_0011DA40(int *p, int *end, int val) {
    while (*p != val && p < end)
        p++;
    if (!(p < end))
        p = 0;
    return p;
}

ASM_FUNC("asm/handwritten/core_text", func_0011DA80);

LINKER_REMNANT("asm/remnants/core_text", func_0011DA90);

extern void func_0011DB98(int, int);
extern int func_0011DA80(int, int, void *);
extern int D_00130410[];
extern int D_00130408;

/* InitSystemCallTableAddress (libkernl.a:initsys.o): patches two kernel
   entry points found by scanning for known function bodies
   (FindAddress = func_0011DA80, matching kCopy = func_0011DA08 and a
   second reference routine, func_0011DA40) into the two descriptors of
   D_00130410 (kFindAddress/func_0011DB98, called once per descriptor),
   then records where the two scans converge (they start from the same
   base and offset by different constants, and are each advanced by 4
   bytes -- whichever trails -- until they land on the same address) in
   D_00130408. `a`/`b` have to be unsigned: retail's convergence test is
   `sltu`. */
void func_0011DA98(void) {
    unsigned int a, b;
    int p1, p2;

    func_0011DB98(D_00130410[0], D_00130410[1]);
    func_0011DB98(D_00130410[2], D_00130410[3]);

    p1 = func_0011DA80(0x80000000, 0x80080000, (void *)func_0011DA40);
    p2 = func_0011DA80(0x80000000, 0x80080000, (void *)func_0011DA08);
    a = p1 - 0x20C;
    b = p2 - 0x168;
    while (a != b) {
        if (a < b) {
            p1 = func_0011DA80(p1 + 4, 0x80080000, (void *)func_0011DA40);
            a = p1 - 0x20C;
        } else {
            p2 = func_0011DA80(p2 + 4, 0x80080000, (void *)func_0011DA08);
            b = p2 - 0x168;
        }
    }
    D_00130408 = a;
}

ASM_FUNC("asm/handwritten/core_text", func_0011DB98);

extern void func_0011D9C0(void);
extern void func_0011DA98(void);
extern void func_0011DE38(void);
extern void func_001195A0(void);
extern void func_0011DCB8(void);
extern void func_0011D3C8(void);

/* _InitSys (libkernl.a:initsys.o): the kernel's one-shot startup sequence,
   run once from the crt0 supplement. Chains six sub-system initialisers;
   the last (InitTLBFunctions, func_0011D3C8) is a tail call since this is
   void and it is the final statement. */
void func_0011DBA8(void) {
    func_0011D9C0();
    func_0011DA98();
    func_0011DE38();
    func_001195A0();
    func_0011DCB8();
    func_0011D3C8();
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011DBE4);
