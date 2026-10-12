#include "common.h"
#include "structs.h"

extern void func_0012F9C8(void *);

extern void func_00130178(void *arg0);

extern void func_00130088(void *arg0);

extern int func_0012FA18(void *);

/* The callback table at +0x40: {callback, data} pairs from +0xC, one
   per callback type. */
typedef int (*MpegCbFn)(void *, void *, void *);

typedef struct { MpegCbFn func; void *data; } MpegCbEnt;

typedef struct { char pad[0xC]; MpegCbEnt cb[1]; } MpegCbTbl;

extern void func_0012FAE8(int);

extern void func_0012FB48(void *, int, int);

extern void func_0012FB60(void *);

/* rac1-decomp src/core/0012AC80.c: func_0012BB98 */
int func_0012F9B8(void *arg0) {
    return ((Wrapper *)arg0)->obj->unk004 == 0;
}

/* rac1-decomp src/core/0012AC80.c: func_0012BBA8 */
/*
 * sceMpegReset (libmpeg.a:mpeg.o): clear the object's decode state
 * (unk000/004/008/0AC, the handler-table slot at +0x80, and +0x118), and
 * the wrapper's own +0x08, then call _clearEach (func_00130178) and hand
 * the object to func_00130088 (zero +0x848, kick the IPU chain via
 * r1_func_00127378).
 */
void func_0012F9C8(void *arg0) {
    char *w = (char *)arg0;
    char *inner = *(char **)(w + 0x40);

    *(int *)(inner + 0x0) = 0;
    *(int *)(inner + 0x4) = 0;
    *(int *)(inner + 0x8) = 0;
    *(int *)(w + 0x8) = 0;
    *(int *)(inner + 0xAC) = 0;
    *(int *)(inner + 0x80) = -1;
    func_00130178(inner);
    *(int *)(inner + 0x118) = 0;
    func_00130088(inner);
}

/* rac1-decomp src/core/0012AC80.c: func_0012BBF8 */
int func_0012FA18(void *arg0) {
    char *b = *(char **)((char *)arg0 + 0x40);
    char *p;
    p = *(char **)(b + 0x1B8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1C8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1D8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1BC); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1CC); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1DC); if (p != 0) *(int *)(p + 0x28) = 0;
    return 1;
}

/* rac1-decomp src/core/0012AC80.c: func_0012BC50 */
/*
 * sceMpegAddCallback: install (callback arg2, data arg3) for type arg1 and
 * return the old callback.
 *
 * Typing the entry as {function pointer, void *} is the lever: 2.9-ee's
 * type-based alias analysis (-fstrict-aliasing is its default) then knows
 * the `data` store cannot alias the `func` load, so sched1 computes
 * `t + 0xC` before `t + off` and the registers fall as in retail. Under
 * 2.95.3 (no type-based aliasing) the same C is 17/36; the old int-offset
 * spelling was 12/36 under both.
 */
int func_0012FA70(void *arg0, int arg1, int arg2, int arg3) {
    MpegCbTbl *t = *(MpegCbTbl **)((char *)arg0 + 0x40);
    MpegCbFn old;
    t->cb[arg1].data = (void *)arg3;
    old = t->cb[arg1].func;
    t->cb[arg1].func = (MpegCbFn)arg2;
    return (int)old;
}

/* rac1-decomp src/core/0012AC80.c: func_0012BC78 */
/*
 * Dispatch the callback registered for *arg1 (the callback data's type
 * field) with its data; returns the callback's result, 0 if none.
 *
 * The table entry is indexed afresh for each use (test `.func`, call
 * `.func(arg0, arg1, .data)`) instead of through one `entry` pointer. CSE
 * merges the two `.func` loads, but the `.data` address is rebuilt after
 * the branch, which is retail's second `addu` in a plain beqz slot, and
 * `ret` then lands in $a3 with the table in $a2. Exact under both
 * compilers (one `entry` pointer is SIZE 84/80; the old spelling 12/80).
 */
void *func_0012FA98(void *arg0, int *arg1) {
    int ret = 0;
    if (arg0 != 0) {
        MpegCbTbl *t = *(MpegCbTbl **)((char *)arg0 + 0x40);
        if (t != 0) {
            if (t->cb[*arg1].func != 0) {
                ret = t->cb[*arg1].func(arg0, arg1, t->cb[*arg1].data);
            }
        }
    }
    return (void *)ret;
}

/* rac1-decomp src/core/0012AC80.c: func_0012BCC8 */
/* No declaration needed: the definition above precedes this caller. The
   extern that used to sit here guessed `void (int, void *)` and now
   conflicts with the real signature. */
void func_0012FAE8(int arg0) {
    int local[8];
    local[0] = 1;
    func_0012FA98(arg0, local);
}

INCLUDE_ASM("asm/core", func_0012FB10);

/* rac1-decomp src/core/0012AC80.c: func_0012BD28 */
void func_0012FB48(void *arg0, int arg1, int arg2) {
    int *p = (int *)arg0;
    p[1] = arg2;
    p[0] = arg1;
    p[2] = arg1;
    p[3] = arg1;
}

/* rac1-decomp src/core/0012AC80.c: func_0012BD40 */
void func_0012FB60(void *arg0) {
    char *p = (char *)arg0;
    *(int *)(p + 0xC) = *(int *)(p + 0x8);
}
