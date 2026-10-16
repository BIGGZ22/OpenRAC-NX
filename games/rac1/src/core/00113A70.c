#include "common.h"
#include "structs.h"

/*
 * core_text object 0x113A70-0x113B70. Boundaries are retail's linker fill
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

void func_00113A70(void *arg0, int arg1, int arg2, int arg3) {
    Node1E4 *self = (Node1E4 *)arg0;
    self->owner = (void *)arg3;
    self->flags = arg1;
    self->handle = arg2;
    self->fn20 = func_001162B8;
    self->fn24 = func_00116320;
    self->fn28 = func_001163A0;
    self->fn2C = func_00116408;
    self->unk04 = 0;
    self->unk08 = 0;
    self->unk10 = 0;
    self->unk18 = 0;
    self->self = self;
    self->unk00 = 0;
}

extern void func_00113968(void);
extern void func_00114438(void *, void *);

/* Retail is a bare tail jump: Sony's 2.9-ee, which builds this SDK object,
   emits it for a void function that ends in a call. */
void func_00113AC8(void *arg0) {
    func_00114438(arg0, func_00113968);
}

LINKER_REMNANT("asm/remnants/core_text", func_00113AD8);

/* newlib's __sinit (findfp.c): the reent's three built-in FILEs become
   stdin, stdout and stderr through std() (func_00113A70), then the glue
   list points at them; the cleanup hook is _cleanup_r (func_00113AC8). */
void func_00113AE0(void *arg0) {
    char *p = (char *)arg0;
    char *n0 = p + 0x1E4;

    *(void (**)(void *))(p + 0x3C) = func_00113AC8;
    *(int *)(p + 0x38) = 1;
    func_00113A70(n0, 4, 0, (int)p);
    func_00113A70(p + 0x23C, 9, 1, (int)p);
    func_00113A70(p + 0x294, 0xA, 2, (int)p);
    *(void **)(p + 0x1E0) = n0;
    *(int *)(p + 0x1DC) = 3;
    *(int *)(p + 0x1D8) = 0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00113B6C);
