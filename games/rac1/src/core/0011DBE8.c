#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x11DBE8-0x11DD68. Boundaries are retail's linker fill
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

ASM_FUNC("asm/handwritten/core_text", func_0011DBE8);

ASM_FUNC("asm/handwritten/core_text", func_0011DBF8);

/* kCopy: libosd.o's own copy of the word-at-a-time copy helper (nbytes
 * rounded down to words); returns 0. Same body as func_0011DA08
 * (initsys.o), func_0011D370 (alarm.o) and func_0011DDF0 (alarm.o's other
 * copy) -- each kernel module linked its own instance of this helper. */
int func_0011DC08(int *dst, int *src, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes >> 2; i++) {
        *dst++ = *src++;
    }
    return 0;
}

ASM_FUNC("asm/handwritten/core_text", func_0011DC40);


/* Reads a word through func_00118CF0, rewrites its 13..15 bit field to 1,
   pushes it back, re-reads it and restores the original. Returns whether
   the field read back as 0. `srl` (not `sra`) at the end is the tell that
   the scratch word is unsigned. */
int func_0011DC50(void) {
    unsigned int saved;
    unsigned int cur;
    func_00118CF0(&saved);
    cur = (saved & 0xFFFF1FFF) | 0x2000;
    func_00118CE0(&cur);
    func_00118CF0(&cur);
    func_00118CE0(&saved);
    return ((cur >> 13) & 7) == 0;
}

extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);

typedef struct {
    int num;
    int fn;
} SysEnt;

/* When func_0011DC50 says so, installs the kernel entries of the
   {num, fn} table at D_00130BD0 (the third through func_0011DC40) and
   copies 0x7A8 bytes of D_00130428 to 0x80074000. Indexing the table in
   the loop keeps its base live, so the argument loads keep source
   order. */
void func_0011DCB8(void) {
    SysEnt *t;
    unsigned int i;

    if (func_0011DC50() == 0) {
        return;
    }
    t = (SysEnt *)D_00130BD0;
    func_0011DBE8(t[0].num, t[0].fn);
    func_0011DBF8((int)0x80074000, D_00130428, 0x7A8);
    func_00118D80(0);
    func_00118D80(2);
    func_0011DBE8(t[1].num, t[1].fn);
    for (i = 2; i < 3; i++) {
        func_0011DBE8(t[i].num, func_0011DC40(t[i].num));
    }
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011DD64);
