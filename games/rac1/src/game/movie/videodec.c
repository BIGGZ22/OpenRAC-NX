#include "common.h"
#include "structs.h"

/* Retail-disassembly reconstruction of decoder buffer operations. */
extern int func_0012B918(void *, unsigned char *, int);
extern int func_0012BC50(void *, int, int, void *);
extern int func_0023E450(void *, int *, void *);
extern int func_0023E478(void *, void *, void *);
extern int func_0023E4B0(void);
extern int func_0023E4E0(void);
extern int func_0023E510(int, char *);
extern void movie_reset(void *) __asm__("func_0023E000");
extern int func_0023D018(void *, void *, void *, int, void *, int);
/* videoDecCreate(VideoDec *, unsigned char *, int, unsigned long long *, unsigned long long *, int, TimeStamp *, int) */
int videoDecCreate(char *dec, unsigned char *data, int size, void *input, void *tags, int count, void *stamps, int stampCount) {
    func_0012B918(dec, data, size);
    func_0012BC50(dec, 0, (int)mpegError, 0);
    func_0012BC50(dec, 1, (int)mpegNodata, 0);
    func_0012BC50(dec, 2, (int)mpeg_stop_dma, 0);
    func_0012BC50(dec, 3, (int)mpeg_restart_video_dma, 0);
    func_0012BC50(dec, 5, (int)get_mpeg_timestamp, 0);
    movie_reset(dec);
    viBufCreate(dec + 0x48, input, tags, count, stamps, stampCount);
    return 1;
}
LINKER_REMNANT("asm/remnants/text", func_0023DF98);
/* VideoDec: only the fields these functions touch are known. */
typedef struct VideoDec {
    int width;          /* 0x0, dimensions passed to the frame-copy routine */
    int height;         /* 0x4 */
    int f08;            /* 0x8, controls the frame-copy path */
    char pad0C[0x3C];
    char viBuf[0x60];   /* ViBuf, the video input buffer */
    int state;          /* 0xA8 */
} VideoDec;

extern int func_0012B008(void *, int, int, int, int);
extern void func_0023D1F0(void *, unsigned char **, int *, unsigned char **, int *);
extern void func_0023D2E8(void *);
extern int func_0023D9E0(void *);

/* videoDecSetStream(VideoDec *, int, int, int (*)(sceMpeg *, sceMpegCbData *, void *), void *)
 * -- hands its five arguments to func_0012B008 and returns 1. */
int videoDecSetStream(VideoDec *dec, int a1, int a2, int (*cb)(void *, void *, void *), void *arg) {
    func_0012B008(dec, a1, a2, (int)cb, (int)arg);
    return 1;
}
/* videoDecBeginPut(VideoDec *, unsigned char **, int *, unsigned char **, int *)
 * -- viBufBeginPut on the decoder's input buffer. */
void videoDecBeginPut(VideoDec *dec, unsigned char **p1, int *n1, unsigned char **p2, int *n2) {
    viBufBeginPut(dec->viBuf, p1, n1, p2, n2);
}
/* videoDecEndPut(VideoDec *, int size) -- viBufEndPut on the decoder's input buffer;
 * the size in $a1 passes straight through. */
extern void vibuf_end_put(void *, int) __asm__("func_0023D2E8");
void videoDecEndPut(VideoDec *dec, int size) {
    vibuf_end_put(dec->viBuf, size);
}
/* videoDecReset(VideoDec *) -- clears the decoder state. */
void videoDecReset(VideoDec *dec) {
    dec->state = 0;
}
extern int func_0023D988(void *);
extern int func_0012BB20(void *);

/* videoDecDelete(VideoDec *) */
int videoDecDelete(VideoDec *dec) {
    viBufDelete(dec->viBuf);
    func_0012BB20(dec);
    return 1;
}
/* videoDecAbort(VideoDec *) -- sets the decoder state to 1. */
void videoDecAbort(VideoDec *dec) {
    dec->state = 1;
}
/* videoDecGetState -- returns the decoder state. */
int videoDecGetState(VideoDec *dec) {
    return dec->state;
}
/* videoDecSetState(VideoDec *, unsigned int) -- stores the new state, returns the old one. */
int videoDecSetState(VideoDec *dec, unsigned int state) {
    int old = dec->state;
    dec->state = state;
    return old;
}
extern void func_0023DBE0(char *, void *);
extern char *D_0016130C MACRO_ADDR;

/* videoDecPutTs(VideoDec *, long, long, unsigned char *, int) */
void videoDecPutTs(VideoDec *dec, long pts, long dts, int pos, int len) {
    struct {
        long pts;
        long dts;
        int diff;
        int len;
    } ts;
    ts.pts = pts;
    ts.dts = dts;
    ts.diff = pos - *(int *)dec->viBuf;
    ts.len = len;
    viBufPutTs(decoder_state + 0xD9090, &ts);
}
/* videoDecInputCount(VideoDec *) -- viBufCount of the input buffer. */
int videoDecInputCount(VideoDec *dec) {
    return viBufCount(dec->viBuf);
}
LINKER_REMNANT("asm/remnants/text", func_0023E0D0);
/* Decoder control reconstructed from retail disassembly. */
typedef struct MovieFourBytes { unsigned char bytes[4]; } MovieFourBytes;
typedef struct MoviePutSpans { unsigned char *first; int firstSize; unsigned char *second; int secondSize; } MoviePutSpans;
extern MovieFourBytes D_00161320;
extern int func_0023CBE0(unsigned char *, int, unsigned char *, int, void *, int, void *, int);
extern void movie_end_put(void *, int) __asm__("func_0023DFE0");
extern void func_0023DA30(void *);
/* videoDecFlush(VideoDec *) */
int videoDecFlush(VideoDec *dec) {
    MovieFourBytes marker = D_00161320;
    MoviePutSpans spans;
    videoDecBeginPut(dec, &spans.first, &spans.firstSize, &spans.second, &spans.secondSize);
    if (spans.firstSize + spans.secondSize < 4) return 0;
    {
        unsigned char *first = (unsigned char *)(((unsigned int)spans.first & 0x0FFFFFFF) | 0x20000000);
        unsigned char *second = (unsigned char *)(((unsigned int)spans.second & 0x0FFFFFFF) | 0x20000000);
        int copied = cpy2area(first, spans.firstSize, second, spans.secondSize, &marker, 4, 0, 0);
        movie_end_put(decoder_state + 0xD9048, copied);
    }
    viBufFlush(dec->viBuf);
    if (dec->state == 0) dec->state = 2;
    return 1;
}
extern int func_0023E0B0(VideoDec *);
extern unsigned int func_0012BB98(void *);

/* videoDecIsFlushed(VideoDec *) */
int videoDecIsFlushed(VideoDec *dec) {
    int res = 0;
    if (videoDecInputCount(dec) == 0) {
        res = func_0012BB98(dec) > 0;
    }
    return res;
}
/* Decoder control reconstructed from retail disassembly. */
extern void func_0023D090(void *);
extern void func_0023E5B8(void *);
extern int func_0023E298(VideoDec *);
/* videoDecMain(void *) */
void videoDecMain(VideoDec *dec) {
    viBufReset(dec->viBuf);
    voBufReset(decoder_state + 0xD9168);
    sceMpegGetPicture(dec);
    while (*(int *)(decoder_state + 0xD9174) != 0 && videoDecGetState(dec) != 1) {
    }
    videoDecSetState(dec, 3);
}
/* Decoder control reconstructed from retail disassembly. */
extern char *movie_state_gp SDATA(D_0016130C);
/* View from movie state +0xD8000; the last five words form the decoded-frame ring. */
typedef struct MovieFrameState {
    char pad[0x1168];
    char *frames;
    char *entries;
    int wr;
    int pending;
    int count;
} MovieFrameState;
extern char D_001E8E80[], D_001E8E68[];
extern char *func_0023E658(void *);
extern void func_0023E5E0(void *);
extern int func_0012BB30(void *, void *, int);
extern int func_0012BB88(void *);
extern void func_0012BBA8(void *);
extern void func_0023BF48(char *);
extern void func_0023BB40(void);
extern void func_001E9730(char *, ...);
extern void func_0023C5E0(void *, void *, int, int);
/* sceMpegGetPicture(VideoDec *) */
int sceMpegGetPicture(VideoDec *dec) {
    int result = 1;
    while (!func_0012BB88(dec)) {
        char *frame;
        if (videoDecGetState(dec) == 1) {
            result = -1;
            STUB_printf(D_001E8E68);
            break;
        }
        while ((frame = voBufGetData(decoder_state + 0xD9168)) == 0) switchThread();
        if (func_0012BB30(dec, frame, 0x340) < 0) ErrMessage(D_001E8E80);
        {
            char *ringState;
            if (dec->f08 == 0) {
                char *state = decoder_state;
                int width = dec->width;
                int height = dec->height;
                int index = 0;
                for (index = 0; index < ((MovieFrameState *)(state + 0xD8000))->count; index++) {
                    setImageTag(((MovieFrameState *)(state + 0xD8000))->entries + index * 0x138C0 + 0x40,
                                 ((MovieFrameState *)(state + 0xD8000))->frames + index * 0xD0000, width, height);
                    state = decoder_state;
                }
                ringState = decoder_state;
            } else {
                ringState = movie_state_gp;
            }
            voBufIncCount(ringState + 0xD9168);
        }
        switchThread();
    }
    func_0012BBA8(dec);
    return result;
}
extern char D_00161328[];
/* mpegError(sceMpeg *, sceMpegCbDataError *, void *) */
int mpegError(void *mpeg, int *cbdata, void *arg) {
    STUB_printf(video_error_message, cbdata[1]);
    return 1;
}
extern int func_0023D340(char *);

/* mpegNodata(sceMpeg *, sceMpegCbData *, void *) -- switchThread, then viBufAddDMA on the
 * buffer at offset 0xD9090 of the movie state. Returns 1. */
int mpegNodata(void *mpeg, void *cbdata, void *arg) {
    switchThread();
    viBufAddDMA(decoder_state + 0xD9090);
    return 1;
}
extern int func_0023D540(char *);   /* viBufStopDMA */

/* No recovered name. viBufStopDMA on the
 * ViBuf at offset 0xD9090 of the movie state. Returns 1. */
int mpeg_stop_dma(void) {
    viBufStopDMA(decoder_state + 0xD9090);
    return 1;
}
extern int func_0023D650(char *);   /* viBufRestartDMA */

/* No recovered name. viBufRestartDMA on the ViBuf at
 * offset 0xD9090 of the movie state. Returns 1. */
int mpeg_restart_video_dma(void) {
    viBufRestartDMA(decoder_state + 0xD9090);
    return 1;
}
typedef struct { long first, second; long pad[2]; } TimeStamp;   /* 0x20 bytes: retail reserves that much */
extern void func_0023DCF0(char *, TimeStamp *);   /* viBufGetTs */

/* No recovered name. Reads the
 * timestamp of the ViBuf at offset 0xD9090 of the movie state with viBufGetTs and stores it
 * at out+8. Returns 1. */
int get_mpeg_timestamp(int unused, char *out) {
    TimeStamp ts;
    viBufGetTs(decoder_state + 0xD9090, &ts);
    *(long *)(out + 8) = ts.first;
    *(long *)(out + 0x10) = ts.second;
    return 1;
}
