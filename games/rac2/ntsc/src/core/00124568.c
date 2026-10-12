#include "common.h"
#include "structs.h"

extern int r1_func_00118C70(void *);

extern void r1_func_001208E4();

extern void r1_func_00118B20(int, void *, int);

extern void r1_func_00118C80(int);

extern int func_00124B88(int);

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void *r1_D_00159840;

/*
 * _sceCd_cd_callback: the N-command completion callback. Latch *arg0 into
 * r1_D_00131414 and r1_D_00131418; status 0xB clears r1_D_00131414/r1_D_001313F0 and
 * returns; otherwise signal the r1_D_001313E8 semaphore (r1_func_00118CA0,
 * iSignalSema, int return) and r1_D_001313E0 too if r1_D_001313D4 and the
 * handler r1_D_00159840 are set, else clear r1_D_001313F0; finally clear
 * r1_D_00131414.
 *
 * Every global it touches is read and written as `volatile`, through
 * `__asm__` aliases (the file's plain `extern int` declarations below
 * stay as they are). That gives retail's two reloads of r1_D_00131414, the
 * bare `nop`s after both jal (the semaphore-id loads cannot go into the
 * slots) and the unfilled `b` slot; with fewer volatiles the size
 * changes. Exact under 2.9-ee only: 2.95.3's GCSE copies the %hi of
 * r1_D_00131414 into $16 (`lui $2` + `move $16,$2`) where retail loads
 * `lui $16` directly (SIZE 164/160).
 */
extern volatile int r1_D_00131414_v __asm__("r1_D_00131414");

extern volatile int r1_D_00131418_v __asm__("r1_D_00131418");

extern volatile int r1_D_001313D4_v __asm__("r1_D_001313D4");

extern volatile int r1_D_001313E0_v __asm__("r1_D_001313E0");

extern volatile int r1_D_001313E8_v __asm__("r1_D_001313E8");

extern volatile int r1_D_001313F0_v __asm__("r1_D_001313F0");

extern void *volatile r1_D_00159840_v __asm__("r1_D_00159840");

extern int r1_func_00118CA0(int);

extern int r1_D_001313E0;

extern int r1_D_001313E8;

extern int r1_D_001313F0;

extern int r1_D_001313D4;

extern int r1_D_001313EC;

/* DeleteSema returns int; declared void, the lui after each call would take
   $v0 instead of retail's $v1. */
extern int r1_func_00118C80_i(int) __asm__("r1_func_00118C80");

extern void r1_func_00118C90(int);

extern void r1_func_0011AA68(int);

extern void r1_func_0011AA38(int arg0, int arg1, int arg2);

extern int r1_D_001313E4;

extern int r1_D_001313FC;

extern void r1_func_00120C58(void);

typedef struct {
    char pad[0x24];
    void *serve;
} SifRpcClientData;

extern int r1_func_00120B28(void);

extern int r1_func_00118CC0(int);

extern int r1_func_00118BF0(int, void *);

extern void r1_func_0011AE20(int);

extern int r1_func_0011B2F8(void *, unsigned int, int);

extern void r1_func_0011A6C8();

extern int r1_D_001313DC;

extern int r1_D_001313D0;

extern int r1_D_001313F8;

extern int r1_D_00159850;

extern char r1_D_00159858[];

extern char r1_D_00152FB0[];

extern char r1_D_00152FD8[];

extern char r1_D_00132590[];

extern int func_00124980(int);

/* rac1-decomp src/core/001208E8.c: func_00120910 */
/* sceCdDelayThread: sleep on an alarm. Same stack-descriptor idiom as
   r1_func_0011BBF0 (a SemaParam: initCount [2] = 0, maxCount [1] = 1,
   option [5] = 0). The second argument to r1_func_00118B20 (SetAlarm) is the
   address of r1_func_001208E4's *second* instruction -- retail builds it as
   one %hi/%lo pair on `r1_func_001208E4 + 4`, so it is a code address the
   source names by symbol, not a separate label. `unsigned short arg0` is
   what puts `andi $17,$4,0xFFFF` in the prologue.

   Exact under 2.9-ee with the stores written initCount, maxCount, option
   (as r1_func_0011B710). Under 2.95.3 it was 24/104: that build front-loads
   `sd $31` where retail interleaves the descriptor stores between the
   `$16` and `$31` saves. */
void func_00124568(unsigned short arg0) {
    int buf[8];
    int h;
    buf[2] = 0;
    buf[1] = 1;
    buf[5] = 0;
    h = r1_func_00118C70(buf);
    r1_func_00118B20(arg0, (char *)r1_func_001208E4 + 4, h);
    r1_func_00118CB0(h);
    r1_func_00118C80(h);
}

/* rac1-decomp src/core/001208E8.c: func_00120978 */
void *func_001245D0(void *arg0) {
    void *old;
    int r;
    if (func_00124B88(1) != 0) {
        return 0;
    }
    r = r1_func_0011D960();
    old = r1_D_00159840;
    r1_D_00159840 = arg0;
    if (r != 0) {
        r1_func_0011D9A8();
    }
    return old;
}

/* rac1-decomp src/core/001208E8.c: func_001209D8 */
void func_00124630(int *arg0) {
    r1_D_00131414_v = *arg0;
    r1_D_00131418_v = r1_D_00131414_v;
    if (r1_D_00131414_v == 0xB) {
        r1_D_00131414_v = 0;
        r1_D_001313F0_v = 0;
        return;
    }
    r1_func_00118CA0(r1_D_001313E8_v);
    if (r1_D_001313D4_v != 0 && r1_D_00159840_v != 0) {
        r1_func_00118CA0(r1_D_001313E0_v);
    } else {
        r1_D_001313F0_v = 0;
    }
    r1_D_00131414_v = 0;
}

INCLUDE_ASM("asm/core", func_001246D0);

INCLUDE_ASM("asm/core", func_00124780);

/* rac1-decomp src/core/001208E8.c: func_00120BC0 */
/*
 * cdvd_exit (libcdvd.a cdvd000.o): if the callback thread exists
 * (r1_D_001313D4, cb_thid), set the callback number (r1_D_00131414,
 * sceCdCbfunc_num) to -1 and SignalSema its semaphore (r1_D_001313E0,
 * cb_semid); DeleteSema the N-command, S-command and callback
 * semaphores; then remove SIF command handler 0x80000012 with interrupts
 * off, re-enabling them if DIntr said they were on.
 *
 * The do-while (0) around the store leaves loop notes in the block, which
 * the scheduler treats as a barrier: the store stays ahead of the
 * semaphore-id load, as in retail, and the load still fills the jal's
 * delay slot. Without it the load is hoisted above the store.
 */
void func_00124818(void) {
    int di;
    if (r1_D_001313D4 != 0) {
        do {
            r1_D_00131414_v = -1;
        } while (0);
        r1_func_00118C90(r1_D_001313E0);
    }
    r1_func_00118C80_i(r1_D_001313E8);
    r1_func_00118C80_i(r1_D_001313EC);
    r1_func_00118C80_i(r1_D_001313E0);
    di = r1_func_0011D960();
    r1_func_0011AA68(0x80000012);
    if (di != 0) {
        r1_func_0011D9A8();
    }
}

INCLUDE_ASM("asm/core", func_001248B0);

/* rac1-decomp src/core/001208E8.c: func_00120CA0 */
int func_001248F8(void) {
    int r;

    r1_D_001313E4 = 1;
    r = r1_func_0011D960();
    r1_func_0011AA38(0x80000012, (int)((char *)r1_func_00120C58 + 8), 0);
    if (r != 0) {
        r1_func_0011D9A8();
    }
    r1_D_001313E4 = 0;
    r1_D_001313FC = 1;
    return 1;
}

INCLUDE_ASM("asm/core", func_00124970);

/* rac1-decomp src/core/001208E8.c: func_00120D28 */
/* _sceCd_scmd_prechk (libcdvd.a cdvd000.o) */
int func_00124980(int arg0) {
    int i;
    volatile int *sem = &r1_D_001313E8;

    r1_func_00120B28();
    if (*sem != r1_func_00118CC0(*sem)) {
        if (r1_D_001313D0 > 0) {
            r1_func_0011A6C8(r1_D_00152FB0, arg0, r1_D_001313DC);
        }
        return 0;
    }
    r1_D_001313DC = arg0;
    r1_func_00118BF0(r1_D_00159850, r1_D_00159858);
    if (func_00124B88(1) != 0) {
        r1_func_00118C90(*sem);
        return 0;
    }
    r1_func_0011AE20(0);
    if (r1_D_001313F8 >= 0) {
        return 1;
    }
    while (1) {
        if (r1_func_0011B2F8(r1_D_00132590, 0x80000595, 0) < 0) {
            if (r1_D_001313D0 > 0) {
                r1_func_0011A6C8(r1_D_00152FD8);
            }
            for (i = 0x100000; i != -1; i--)
                ;
        } else {
            if (((SifRpcClientData *)r1_D_00132590)->serve != 0) {
                break;
            }
            for (i = 0x100000; i != -1; i--)
                ;
        }
    }
    r1_D_001313F8 = 0;
    return 1;
}
