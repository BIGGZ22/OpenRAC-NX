#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x119868-0x119D88. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl), tty.o: the deci2 TTY queue and
 * protocol handler (sceTtyHandler, sceTtyWrite, sceTtyRead, sceTtyInit).
 * Built with Sony's 2.9-ee (Makefile.sn, EE29_CORE), like the prebuilt
 * libkernl.a.
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
extern void func_00115578(void *arg0, void *arg1);
extern int func_001160D8(void);

/*
 * Close but not exact, same open-question category as func_001160D8/
 * func_00115578 (scratch-register/scheduling choice) but manifesting as
 * store reordering instead: retail schedules `self->field8 = ...;
 * self->field4 = 0;` before the branch and puts `self->fieldC = ...` in
 * the delay slot; this compiler schedules fieldC and field8 before the
 * branch and puts field4's store in the delay slot instead. Confirmed
 * source-order independent -- tried every permutation of the 3
 * assignments, all four produced the identical instruction sequence, so
 * this is the scheduler's own choice, not something this source
 * controls. Logic (D_00154A40's first field = arg0, then fields at
 * 0x4/0x8/0xC of the pointed-to struct get 0/self+0x10/self+0x10, return
 * self) is fully understood and correct either way.
 *
 * extern void *D_00154A40;
 *
 * void *func_00119868(void *arg0) {
 *     char *self = (char *)&D_00154A40;
 *     D_00154A40 = arg0;
 *     *(void **)(self + 0x8) = self + 0x10;
 *     *(int *)(self + 0x4) = 0;
 *     *(void **)(self + 0xC) = self + 0x10;
 *     return self;
 * }
 */
extern void *D_00154A40 NOT_SDA;

void *func_00119868(void *arg0) {
    char *self = (char *)&D_00154A40;
    D_00154A40 = arg0;
    *(int *)(self + 0x4) = 0;
    *(void **)(self + 0xC) = self + 0x10;
    *(void **)(self + 0x8) = self + 0x10;
    return self;
}

void func_00119890(char *self) {
    char *p;
    *(int *)(self + 0x4) += 1;
    p = *(char **)(self + 0xC) + 1;
    *(char **)(self + 0xC) = p;
    if (p == self + (*(int *)self + 0x10)) {
        *(char **)(self + 0xC) = self + 0x10;
    }
}

void func_001198D0(char *self) {
    char *p;
    *(int *)(self + 0x4) -= 1;
    p = *(char **)(self + 0x8) + 1;
    *(char **)(self + 0x8) = p;
    if (p == self + (*(int *)self + 0x10)) {
        *(char **)(self + 0x8) = self + 0x10;
    }
}

typedef struct {
    unsigned short len;
} TtyHdr;

struct TtyRing {
    int size;
    int count;
    char *rp;
    char *wp;
};

typedef struct {
    int sock;
    volatile int wlen;
    volatile int rlen;
    volatile int busy;
    char *wbuf;
    char *rbuf;
    struct TtyRing *queue;  /* func_00119868's input ring */
} TtyHandlerState;

extern void func_0011A690(const char *, ...);
extern int func_001197C0_r(int, char *, unsigned short) __asm__("func_001197C0");
extern int func_001197F8_r(int, char *, unsigned short) __asm__("func_001197F8");
extern char D_00152810[];
extern char D_00152838[];
extern char D_00152850[];
extern char D_00152868[];

/* sceTtyHandler: the DECI2 event callback (1/2 read, 3 write, 4 write done). */
void func_00119910(int event, int param, TtyHandlerState *ti) {
    int n;

    switch (event) {
    case 1:
    case 2:
        if (param != 0) {
            if ((unsigned int)(ti->rlen + param) > 0x140) {
                func_0011A690(D_00152810);
            }
            {
                int off = ti->rlen;

                n = func_001197C0_r(ti->sock, ti->rbuf + off, param);
            }
            if (n < 0) {
                func_0011A690(D_00152838);
            }
            ti->rlen += n;
        } else {
            TtyHdr *h = (TtyHdr *)ti->rbuf;

            for (n = 0xC; n < h->len; n++) {
                *ti->queue->wp = ti->rbuf[n];
                func_00119890((char *)ti->queue);
            }
            ti->rlen = 0;
        }
        break;
    case 3: {
        int w = func_001197F8_r(ti->sock, ti->wbuf, ti->wlen);

        if (w < 0) {
            func_0011A690(D_00152850, w);
            ti->busy = 0;
        } else {
            ti->wbuf += w;
            ti->wlen -= w;
        }
        break;
    }
    case 4:
        if (ti->wlen != 0) {
            func_0011A690(D_00152868, ti->wlen);
        }
        ti->busy = 0;
        break;
    }
}

struct TtyState {
    volatile s32 unk0; /* deci2 handle from sceDeci2Open */
    volatile s32 unk4; /* published payload length */
    volatile s32 unk8;
    volatile s32 unkC; /* busy flag, also written by sceTtyHandler */
    s32 unk10; /* MMIO block pointer (with 0x20000000 flag bit) */
    s32 unk14;
    s32 unk18;
};
struct Mmio {
    u16 unk0; /* transmit length */
    u16 unk2;
    u16 unk4;
    u8 unk6;
    s8 unk7; /* CallDebugCharacter mode byte (read with `lb`) */
    u32 unk8;
    u8 data[0xF4]; /* payload window, addressed at block + 0xC */
};
extern struct TtyState D_00154B50_19AA8 __asm__("D_00154B50");
extern u8 D_00154B80[];
extern s32 func_00119768(s32, s32);
extern s32 func_0011D960();
extern s32 func_0011D9A8();
extern void func_00119798(s32);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/debug/sce_tty_write.c, sceTtyWrite. */
s32 func_00119AA8(s8 *text, s32 text_len) {
    s32 written_count; /* characters consumed -> return value */
    s32 remaining; /* countdown, hits -1 after len+1 tests */
    s32 window_bytes; /* bytes resident in the payload window */
    s8 *src; /* source cursor */
    struct Mmio *mmio;
    u8 *dst; /* payload cursor */

    written_count = 0;
    remaining = text_len;
    window_bytes = 0;
    src = text;
    if (D_00154B50_19AA8.unkC == 0) {
        func_0011D960();
        D_00154B50_19AA8.unkC = 1;
        mmio = (struct Mmio *)((u32)D_00154B80 | 0x20000000);
        D_00154B50_19AA8.unk10 = (s32)mmio;
        dst = mmio->data;
        do {
            remaining = remaining - 1;
            if (remaining == -1) {
                break;
            }
            if (*src == 0xA) {
                *dst = 0xD;
                window_bytes = window_bytes + 1;
                dst = dst + 1;
                if (window_bytes >= 0x100) {
                    break;
                }
            }
            *dst = (u8)*src;
            window_bytes = window_bytes + 1;
            src = src + 1;
            dst = dst + 1;
            written_count = written_count + 1;
        } while (window_bytes < 0x100);
        D_00154B50_19AA8.unk4 = window_bytes + 0xC;
        mmio->unk0 = D_00154B50_19AA8.unk4;
        if (func_00119768(D_00154B50_19AA8.unk0, mmio->unk7) < 0) {
            *(s32 *)&D_00154B50_19AA8.unkC = 0;
            func_0011D9A8();
            return -1;
        }
        if (D_00154B50_19AA8.unkC != 0) {
            do {
                func_00119798(D_00154B50_19AA8.unk0);
            } while (D_00154B50_19AA8.unkC != 0);
        }
        func_0011D9A8();
        return written_count;
    }
    return -1;
}

/* The input ring func_00119868 sets up: size, count, read/write heads. */
typedef struct {
    int size;
    int count;
    char *rp;
    char *wp;
} TtyQueue;

typedef struct {
    int sock;
    int unk04;
    int unk08;
    int unk0C;
    char *wbuf;
    char *rbuf;
    TtyQueue *queue;    /* 0x18 */
} TtyState2;

extern TtyState2 D_00154B50_t __asm__("D_00154B50");
extern TtyQueue *D_00154B68;

/* sceTtyRead: block for each byte until the handler queues one; stop
   after a newline. */
int func_00119BF8(char *buf, int len) {
    int i;

    for (i = 0; i < len; i++) {
        while (((volatile TtyQueue *)D_00154B68)->count == 0) {
        }
        {
            TtyState2 *t = &D_00154B50_t;

            buf[i] = *t->queue->rp;
            func_001198D0((char *)t->queue);
        }
        if (buf[i] == '\n' || buf[i] == '\r') {
            return i + 1;
        }
    }
    return i;
}

struct TtyInitState {
    s32 unk0;
    volatile s32 unk4;
    volatile s32 unk8;
    volatile s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};
struct TtyMmio {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u8 unk6;
    u8 unk7;
    u32 unk8;
};
extern struct TtyInitState D_00154B50;
extern u8 D_00154CC0[];
extern s32 func_00118D80();
extern s32 func_00119868_tty(int) __asm__("func_00119868");
extern s32 func_00119718();
extern void func_00119910();
/* Opens the TTY DECI2 endpoint and initializes its uncached packet headers. */
s32 func_00119CC8(void) {
    struct TtyInitState *state = &D_00154B50;
    struct TtyMmio *p;
    struct TtyMmio *q;
    u32 mask;
    s32 baud;
    s32 cmd;

    func_00118D80(0);
    *(volatile s32 *)&state->unk0 = func_00119718(0x210, state, &func_00119910);
    if (state->unk0 < 0) {
        return 0;
    }
    state->unkC = 0;
    mask = 0x20000000;
    q = (struct TtyMmio *)((u32)D_00154CC0 | mask);
    state->unk4 = 0;
    p = (struct TtyMmio *)((u32)D_00154B80 | mask);
    state->unk8 = 0;
    state->unk14 = (s32)q;
    state->unk10 = (s32)p;
    baud = 0x210;
    cmd = 0x45;
    p->unk2 = 0;
    p->unk4 = baud;
    p->unk6 = cmd;
    p->unk7 = 0x48;
    p->unk8 = 0;
    state->unk18 = func_00119868_tty(0x100);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00119D84);
