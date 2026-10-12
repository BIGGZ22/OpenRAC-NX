#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x11DD68-0x11DDD0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl), exit.o: kCopy (the byte
 * version), a TLB re-init stub and Exit. Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE), like the prebuilt libkernl.a, which emits
 * both of its tail calls natively.
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
extern void func_0011D4A0(void);

/*
 * exit.o's kCopy: a byte copy loop returning 0. Exact under 2.9-ee, which
 * pads the short loop itself and leaves the branch's delay slot empty,
 * with `dst++` before the branch as retail has it. (Under 2.95.3 reorg
 * sinks `dst++` into the slot, 44 bytes against 48: the old revert.)
 */
int func_0011DD68(char *dst, char *src, unsigned int n) {
    unsigned int i;
    for (i = 0; i < n; i++) {
        *dst++ = *src++;
    }
    return 0;
}

/* Tail call: retail is `j func_0011D4A0` + nop, with no frame at all.
   2.9-ee emits it natively for a void function that ends in a call. */
void func_0011DD98(void) {
    func_0011D4A0();
}

/*
 * Exit(status): re-init the TLB (func_0011DD98), then the _Exit syscall
 * (func_00118A60) with the status, as a bare tail jump. Exact under
 * 2.9-ee, which emits "call, then tail call" itself; under 2.95.3 it was
 * 4 bytes over (2.95.3 has no sibling calls). The status lives in $16
 * across the first call.
 */
extern void func_00118A60(int);

void func_0011DDA0(int status) {
    func_0011DD98();
    func_00118A60(status);
}

LINKER_REMNANT("asm/remnants/core_text", func_0011DDC8);
