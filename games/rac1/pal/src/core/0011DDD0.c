#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x11DDD0-0x11DF10. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
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

ASM_FUNC("asm/handwritten/core_text", func_0011DDD0);

ASM_FUNC("asm/handwritten/core_text", func_0011DDE0);

/* kCopy: alarm.o's own copy of the word-at-a-time copy helper (nbytes
 * rounded down to words); returns 0. Same body as func_0011DA08
 * (initsys.o), func_0011D370 and func_0011DC08 (libosd.o's copy) -- each
 * kernel module linked its own instance of this helper. */
int func_0011DDF0(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

ASM_FUNC("asm/handwritten/core_text", func_0011DE28);

extern void func_0011DDD0(int, int);
extern void func_0011DDE0(void *, void *, int);
extern int func_0011DE28(int);
extern int D_00131358[];
extern char D_00130BF0[];
extern char D_00131330[];

typedef struct { int a; int b; } D_00131358_pair;

/*
 * Same-size near-miss (8/0xD4 bytes). Same shape as func_0011D3C8:
 * guarded by a hardware status bit at 0x10001810 (skip everything if
 * already done), two (addr,value) pokes through func_0011DDD0, two
 * func_0011DDE0 block copies (to a fixed load address and to
 * scratchpad at 0x82000), an interrupt disable/enable bracket, a
 * third func_0011DDD0 poke, then a 6-entry table loop where
 * func_0011DE28 reads the entry's current value back before
 * func_0011DDD0 rewrites it. All three callees are handwritten
 * syscall wrappers.
 */
void func_0011DE38(void) {
    int *t;
    D_00131358_pair *p;
    unsigned int i;
    int old;

    if (*(volatile unsigned int *)0x10001810 & 0x100) {
        return;
    }

    t = D_00131358;
    func_0011DDD0(t[0], t[1]);
    func_0011DDE0((void *)0x80076000, D_00130BF0, 0x740);
    func_0011DDE0((void *)0x82000, D_00131330, 0x28);
    func_00118D80(0);
    func_00118D80(2);
    func_0011DDD0(t[2], t[3]);

    p = (D_00131358_pair *)(t + 4);
    for (i = 2; i < 8; i++) {
        old = func_0011DE28(p->a);
        func_0011DDD0(p->a, old);
        p++;
    }
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011DF0C);
