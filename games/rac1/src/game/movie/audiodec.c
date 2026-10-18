#include "common.h"
#include "callees.h"
#include "structs.h"

extern int func_0012F1A8(int, int, int, int, int, int);
extern char *D_001613B8 MACRO_ADDR;
/* Clears the header, sets buffer parameters, and acquires the sound transport handle. */
int audioDecCreate(char *dec, void *buffer, int size, char *staging) {
    FastMemZero16(dec + 8, 0x20);
    *(void **)(dec + 0x34) = buffer;
    *(int *)(dec + 0x40) = size;
    *(int *)(dec + 4) = 3;
    *(int *)dec = 0;
    *(int *)(dec + 0x30) = 0;
    *(int *)(dec + 0x38) = 0;
    *(int *)(dec + 0x3C) = 0;
    *(int *)(dec + 0x44) = 0;
    *(int *)(dec + 0x50) = 0;
    *(int *)(dec + 0x58) = 0;
    *(int *)(dec + 0x5C) = 0;
    *(int *)(dec + 0x60) = 0;
    D_001613B8 = staging;
    *(int *)(dec + 0x4C) = 0x400;
    *(int *)(dec + 0x48) = func_0012F1A8(0x400, 0x1000, 0x400, 0, 5, 3);
    if (*(int *)(dec + 0x48) < 0) return 0;
    return 1;
}
/* AudioDec: only the fields these functions touch are known. */
typedef struct AudioDec {
    int pending;        /* non-zero while data waits for the SPU; audioDecStart sets it to 2 */
    int mode;           /* 0x4 */
    char pad8[0xC];
    int f14;            /* 0x14 */
    int f18;            /* 0x18 */
    char pad1C[0x14];
    int fill;           /* 0x30 */
    unsigned char *buf; /* 0x34 */
    int rd;             /* 0x38 */
    int cnt;            /* 0x3C */
    int size;           /* 0x40 */
    int f44;            /* 0x44 */
    int f48;            /* 0x48 */
    int f4C;            /* 0x4C, rounded down to a multiple of 0x400 for the sound call */
    int bytes;          /* 0x50 */
    char pad54[8];
    int f5C;            /* 0x5C */
} AudioDec;

extern int func_0012F220(void);

/* audioDecDelete(_AudioDec *) -- calls func_0012F220 and returns 1. */
int audioDecDelete(AudioDec *dec) {
    func_0012F220();
    return 1;
}
LINKER_REMNANT("asm/remnants/text", func_0023C080);
extern void func_0012F248(int, int, int, int, int);   /* snd_StartMovieSound */

/* audioDecStart -- starts the movie sound with the decoder's parameters (size rounded down to
 * 0x400) and marks the decoder as started. */
void audioDecStart(AudioDec *dec) {
    func_0012F248(dec->f48, dec->f4C / 0x400 * 0x400, dec->f5C, dec->f14, dec->f18);
    dec->pending = 2;
}
extern void func_0012F1E8(void);

/* audioDecReset(_AudioDec *) */
void audioDecReset(volatile int *dec) {
    func_0012F1E8();
    dec[0x5C / 4] = 0;
    dec[0x0 / 4] = 0;
    dec[0x30 / 4] = 0;
    dec[0x38 / 4] = 0;
    dec[0x3C / 4] = 0;
    dec[0x44 / 4] = 0;
    dec[0x50 / 4] = 0;
    dec[0x58 / 4] = 0;
}
/* audioDecBeginPut(_AudioDec *, unsigned char **, int *, unsigned char **, int *) -- hands out
 * the free part of the ring as up to two (pointer, length) spans. */
void audioDecBeginPut(AudioDec *a, unsigned char **p1, int *n1, unsigned char **p2, int *n2) {
    if (a->pending == 0) {
        if (a->mode != 4) {
            *p1 = (unsigned char *)a + (a->fill + 8);
            *n1 = 0x28 - a->fill;
            *p2 = a->buf;
            *n2 = a->size;
            return;
        }
        *p1 = a->buf;
        *n1 = a->size;
    none:
        *p2 = 0;
        *n2 = 0;
        return;
    } else {
        int avail = a->size - a->cnt;
        if (a->size - a->rd >= avail) {
            *p1 = a->buf + a->rd;
            *n1 = avail;
            goto none;
        }
        *p1 = a->buf + a->rd;
        *n1 = a->size - a->rd;
        *p2 = a->buf;
        *n2 = avail - (a->size - a->rd);
        return;
    }
}
/* Accounts for header bytes first, then advances the ring cursor and queued byte counts. */
void audioDecEndPut(AudioDec *dec, int count) {
    if (dec->pending == 0) {
        if (dec->mode != 4) {
            int used;
            {
                int header = 0x28 - dec->fill;
                header = header < count ? header : count;
                used = header;
            }
            dec->fill += used;
            if (dec->fill >= 0x28) dec->pending = 1;
            count -= used;
        } else {
            dec->pending = 1;
        }
    }
    dec->size = dec->size / 0x400 * 0x400;
    dec->rd = (dec->rd + count) % dec->size;
    dec->cnt += count;
    dec->f44 += count;
}
/* No recovered name. True once 0x1000 bytes or more are queued. */
int audioDecIsPageFull(AudioDec *dec) {
    return dec->bytes >= 0x1000;
}
extern void func_0023C390(AudioDec *);

/* audioDecSend -- sendADPCM while data is pending. */
void audioDecSend(AudioDec *dec) {
    if (dec->pending) {
        sendADPCM(dec);
    }
}
typedef struct { int src; int dst; int size; int mode; } SpuDma;

extern void func_00118D80(int);
extern unsigned int func_00118E20(SpuDma *, int);
extern int func_00118E10(int);
extern int func_0012F288(int, int);

/* sendToSPU(_AudioDec *, unsigned char *, int, int) -- queues one transfer of len bytes to the
 * address in the decoder at 0x48, waits for it to finish, then calls func_0012F288(len, flag). */
void sendToSPU(AudioDec *dec, unsigned char *src, int len, int flag) {
    SpuDma dma;
    unsigned int id;
    func_00118D80(0);
    dma.src = (int)src;
    dma.dst = dec->f48;
    dma.size = len;
    dma.mode = 0;
    while ((id = func_00118E20(&dma, 1)) == 0) {
    }
    while (func_00118E10(id) >= 0) {
    }
    func_0012F288(len, flag);
}
typedef struct AdpcmDec_0023C390 {
    int mode;           /* 0x0 */
    char pad4[0x14];
    int nch;            /* 0x18 */
    int chunk;          /* 0x1C */
    char pad20[0x14];
    unsigned char *buf; /* 0x34 */
    int rd;             /* 0x38 */
    int cnt;            /* 0x3C */
    int size;           /* 0x40 */
    char pad44[0xC];
    int bytes;          /* 0x50 */
    char pad54[0xC];
    int pos;            /* 0x60 */
} AdpcmDec_0023C390;
extern int func_0012F2B8(void);

/* sendADPCM(_AudioDec *) */
void sendADPCM(AudioDec *dec) {
    AdpcmDec_0023C390 *d = (AdpcmDec_0023C390 *)dec;
    int avail;
    int ch;
    int done;
    int next;
    unsigned char *src;
    unsigned char *dst;
    int j;
    int v;
    int w;

    avail = 0;
    switch (d->mode) {
    case 1:
        if (d->cnt < 0x1000) {
            return;
        }
        avail = 0x1000 - d->bytes;
        break;
    case 2:
        avail = (func_0012F2B8() - d->pos) & 0xFFF;
        break;
    case 3:
        return;
    }
    while (avail >= 0x400 && d->cnt >= (d->nch << 10)) {
        next = avail - 0x400;
        for (ch = 0; ch < d->nch; ch++) {
            done = 0;
            src = d->buf + (d->rd - d->cnt + d->size) % d->size;
            src += ch * d->chunk;
            dst = (unsigned char *)D_001613B8;
            do {
                for (j = 0; j < d->chunk; j++) {
                    *dst++ = *src++;
                    done++;
                }
                src += d->chunk * (d->nch - 1);
            } while (done < 0x400);
            if (d->pos + 0x400 == 0x1000) {
                D_001613B8[0x3F1] = 3;
            }
            if (d->pos == 0) {
                char *p = D_001613B8;
                p[1] = 6;
                p[0x11] = 2;
            }
            sendToSPU(dec, (unsigned char *)D_001613B8, 0x400, (ch << 12) + d->pos);
        }
        v = d->pos + 0x400;
        w = (v > -1) ? v : v + 0xFFF;
        d->pos = v - (w >> 12 << 12);
        d->cnt -= d->nch << 10;
        d->bytes += 0x400;
        avail = next;
    }
}
