#include "common.h"
#include "structs.h"

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
    struct TtyRing *queue;  /* func_0011BAC8's input ring */
} TtyHandlerState;

extern void r1_func_0011A690(const char *, ...);

extern int r1_func_001197C0_r(int, char *, unsigned short) __asm__("r1_func_001197C0");

extern int r1_func_001197F8_r(int, char *, unsigned short) __asm__("r1_func_001197F8");

extern char r1_D_00152810[];

extern char r1_D_00152838[];

extern char r1_D_00152850[];

extern char r1_D_00152868[];

extern void func_0011BB38();

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

extern struct TtyState r1_D_00154B50_19AA8 __asm__("r1_D_00154B50");

extern u8 r1_D_00154B80[];

extern s32 r1_func_00119768(s32, s32);

extern s32 r1_func_0011D960();

extern s32 r1_func_0011D9A8();

extern void r1_func_00119798(s32);

struct TtyInitState {
    s32 unk0;
    volatile s32 unk4;
    volatile s32 unk8;
    volatile s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};

extern struct TtyInitState r1_D_00154B50;

/* rac1-decomp src/core/00119868.c: func_00119890 */
void func_0011BAF0(char *self) {
    char *p;
    *(int *)(self + 0x4) += 1;
    p = *(char **)(self + 0xC) + 1;
    *(char **)(self + 0xC) = p;
    if (p == self + (*(int *)self + 0x10)) {
        *(char **)(self + 0xC) = self + 0x10;
    }
}

/* rac1-decomp src/core/00119868.c: func_00119910 */
/* sceTtyHandler: the DECI2 event callback (1/2 read, 3 write, 4 write done). */
void func_0011BB38(int event, int param, TtyHandlerState *ti) {
    int n;

    switch (event) {
    case 1:
    case 2:
        if (param != 0) {
            if ((unsigned int)(ti->rlen + param) > 0x140) {
                r1_func_0011A690(r1_D_00152810);
            }
            {
                int off = ti->rlen;

                n = r1_func_001197C0_r(ti->sock, ti->rbuf + off, param);
            }
            if (n < 0) {
                r1_func_0011A690(r1_D_00152838);
            }
            ti->rlen += n;
        } else {
            TtyHdr *h = (TtyHdr *)ti->rbuf;

            for (n = 0xC; n < h->len; n++) {
                *ti->queue->wp = ti->rbuf[n];
                func_0011BAF0((char *)ti->queue);
            }
            ti->rlen = 0;
        }
        break;
    case 3: {
        int w = r1_func_001197F8_r(ti->sock, ti->wbuf, ti->wlen);

        if (w < 0) {
            r1_func_0011A690(r1_D_00152850, w);
            ti->busy = 0;
        } else {
            ti->wbuf += w;
            ti->wlen -= w;
        }
        break;
    }
    case 4:
        if (ti->wlen != 0) {
            r1_func_0011A690(r1_D_00152868, ti->wlen);
        }
        ti->busy = 0;
        break;
    }
}

/* rac1-decomp src/core/00119868.c: func_00119AA8 */
/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/debug/sce_tty_write.c, sceTtyWrite. */
s32 func_0011BCD0(s8 *text, s32 text_len) {
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
    if (r1_D_00154B50_19AA8.unkC == 0) {
        r1_func_0011D960();
        r1_D_00154B50_19AA8.unkC = 1;
        mmio = (struct Mmio *)((u32)r1_D_00154B80 | 0x20000000);
        r1_D_00154B50_19AA8.unk10 = (s32)mmio;
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
        r1_D_00154B50_19AA8.unk4 = window_bytes + 0xC;
        mmio->unk0 = r1_D_00154B50_19AA8.unk4;
        if (r1_func_00119768(r1_D_00154B50_19AA8.unk0, mmio->unk7) < 0) {
            *(s32 *)&r1_D_00154B50_19AA8.unkC = 0;
            r1_func_0011D9A8();
            return -1;
        }
        if (r1_D_00154B50_19AA8.unkC != 0) {
            do {
                r1_func_00119798(r1_D_00154B50_19AA8.unk0);
            } while (r1_D_00154B50_19AA8.unkC != 0);
        }
        r1_func_0011D9A8();
        return written_count;
    }
    return -1;
}
