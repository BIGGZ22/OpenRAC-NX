#include "common.h"
#include "callees.h"
#include "structs.h"

extern int D_00161308 MACRO_ADDR;
extern int D_00161314 MACRO_ADDR;
extern int movie_clear_gp SDATA(D_00161318);
extern char *D_0016130C MACRO_ADDR;
extern char *movie_gp_base SDATA(D_0016130C);
extern int func_00118BA0(int,int);
extern int func_0023BB90(int,int,int);
extern int func_0023B740(char*,char*,int*);
extern void func_0023BE38(void);
/* No recovered name. Sets movie globals, runs initialization and playback, then clears them. */
int Movie_Play(int a,int b,int c,char *state,int e) {
    decoder_buffer_address=c;
    decoder_state=state;
    D_00161314=0;
    movie_clear_gp=0;
    func_00118BA0(func_00118BE0(),1);
    D_00161314=1;
    if(initAll(a,b,e)) {
        char *base=movie_gp_base;
        D_00161314=2;
        read_mpeg(base+0xD9048,base,base+0xD9040);
    }
    termAll();
    decoder_buffer_address=0;
    decoder_state=0;
    return 0;
}
extern int D_00161318 MACRO_ADDR;
extern int D_0015EFD8 MACRO_ADDR;
extern int D_0015EFA0 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;
/* pad state at D_0013CA40: two words read at 0x1A0/0x1A4 */
typedef struct MvPad {
    char pad0[0x1A0];
    int f1A0;
    int f1A4;
} __attribute__((aligned(16))) MvPad;
extern MvPad D_0013CA40;
/* global block at D_0013E650: word at 0x5C */
typedef struct MvGlobals {
    char pad0[0x5C];
    int f5C;
} __attribute__((aligned(16))) MvGlobals;
extern MvGlobals D_0013E650;
extern void func_00118D80(int);
extern int func_00122598(int);
extern void func_00218908(void);
extern void func_0023E040(char *);
extern int func_0023CD30(char *, int *);
extern int func_0023CE30(int *, int, int, int);
extern int func_0023CD60(char *, int);
extern void func_0023BF70(void);
extern void func_0023BB40(void);
extern int func_0023CDA8(char *, int *);
extern int func_0012AD10(char *, int, int, char *, int);
extern int func_0023CDF0(char *, int);
extern int func_0023E5C8(char *);
extern void func_0023C960(int);
extern void func_0023C088(char *);
extern int func_0023E050(char *);
extern int func_0023E0D8(char *);
extern int func_0023E1B0(char *);
extern void func_0023C9B0(void);
extern void func_0023C0E0(char *);
extern void func_0012E348(int, int);
extern void func_0012DDC0(void);

/* No recovered name. Playback loop: feeds file data (*file bytes) through the
 * read buffer at `rb` into the video decoder `vd`, starts audio and display once
 * the output buffer fills, aborts on a pad combination, then drains the decoder.
 * D_00161314 records the step reached. Returns 1 if aborted. */
int read_mpeg(char *vd, char *rb, int *file) {
    int aborted = 0;
    int started = 0;
    int left = *file;
    int toread;
    int space;
    int got;
    int avail;
    int used;
    int put;
    int get;

    D_00161314 = 3;
    func_00118D80(0);
    toread = left;
    func_00118D80(2);
    func_00122598(0);
    D_00161314 = 4;

    while (left >= 5 && videoDecGetState(vd) != 3) {
        int hit;

        D_00161314 = 5;
        D_00161318++;
        UpdatePad();
        D_00161314 = 6;
        if (D_0015EFD8 != -1) {
            D_00161314 = 7;
            if (D_0015EFD8 == 2 && D_0013CA40.f1A4 != 0) {
                hit = 1;
            } else if ((D_0015EFA0 || D_0015EF20 || D_0015EFD8 || D_0015EE84 <= 0)
                       && (D_0013CA40.f1A4 & 0x800)) {
                hit = 1;
            } else if ((D_0013CA40.f1A0 & 0xF) == 0xF && (D_0013CA40.f1A4 & 0x800)) {
                hit = 1;
            } else {
                hit = 0;
            }
            if (hit) {
                aborted = 1;
                videoDecAbort(decoder_state + 0xD9048);
            }
            D_00161314 = 8;
        }
        D_00161314 = 9;
        space = readBufBeginPut(rb, &put);
        D_00161314 = 10;
        if (toread > 0 && space > 0xFFFF) {
            D_00161314 = 11;
            got = read_cd_stream_sectors(file, put, 0x10000, 0);
            D_00161314 = 12;
            readBufEndPut(rb, got);
            toread -= got;
            D_00161314 = 13;
        }
        D_00161314 = 14;
        proceedAudio();
        D_00161314 = 15;
        switchThread();
        D_00161314 = 16;
        avail = readBufBeginGet(rb, &get);
        D_00161314 = 17;
        if (avail > 0) {
            int size = *(int *)(rb + 0x50008);

            D_00161314 = 18;
            used = func_0012AD10(vd, get, avail, rb, size);
            D_00161314 = 19;
            readBufEndGet(rb, used);
            left -= used;
            D_00161314 = 20;
        }
        D_00161314 = 21;
        proceedAudio();
        D_00161314 = 22;
        if (!started && voBufIsFull(decoder_state + 0xD9168) && isAudioOK()) {
            D_00161314 = 23;
            started = 1;
            startDisplay(1);
            D_00161314 = 24;
            audioDecStart(decoder_state + 0xD9100);
            D_00161314 = 25;
        }
    }

    while (!videoDecFlush(vd)) {
        proceedAudio();
        switchThread();
    }
    while (!videoDecIsFlushed(vd) && videoDecGetState(vd) != 3) {
        proceedAudio();
        switchThread();
    }
    endDisplay();
    audioDecReset(decoder_state + 0xD9100);
    func_0012E348(5, D_0013E650.f5C);
    func_0012DDC0();
    D_00161314 = 0;
    D_00161318 = 0;
    return aborted;
}
extern int func_00118BC0(int);

/* switchThread -- calls func_00118BC0(1). */
void switchThread(void) {
    func_00118BC0(1);
}
extern int func_0023C2B0(char *);

/* isAudioOK -- true when the audio decoder at offset 0xD9100 of the movie state holds 0x1000
 * bytes or more. */
int isAudioOK(void) {
    return is_state_field_large(decoder_state + 0xD9100);
}
struct AudioDecoderState {
    char pad0[0xD90F8];
    int dmac_handler_id;
    int intc_handler_id;
};
struct AudioDecoderThreadArgs {
    char pad0[4];
    void (*entry)();
    void *stack;
    int stack_size;
    void *gp;
    int priority;
    char pad18[8];
    int gs_tag;
    char pad24[0xC];
};
extern int D_00161310 MACRO_ADDR;
extern char D_001E8E20[];
extern char D_00166D00[] MACRO_ADDR;
extern void func_0012B870(void);
extern int func_0023CD10(char *);
extern int func_0023DE98(void *, int, int, void *, int, int, void *, int);
extern int func_0023BFA0(void *, void *, int, int);
extern int func_0023DFA0(void *, int, int, void *, char *);
extern void func_0023E560(void *, int, int, int);
extern int func_00118B50(struct AudioDecoderThreadArgs *);
extern int func_00118B70(int, void *);
extern int func_0023CE18(void *, int, int);
extern int func_00118A90(int, void *, int);
extern int func_00119390(int);
extern int func_00118AB0(int, void *, int);
extern int func_00119460(int);
extern void func_0023C7A8();
extern void func_0023C910();
extern void func_0023E1F8();
extern int func_0023C9C0();
extern int func_0023CAF8();
/* initAll(int, int, int) */
int initAll(int stream_source, int source_mode, int callback_context) {
    struct AudioDecoderThreadArgs decoder_thread;
    int thread_id;
    int opened;
    *((volatile int *)0x1000E000) |= 3;
    *((volatile int *)0x1000E010) = 4;
    readBufCreate(movie_gp_base);
    func_0012B870();
    videoDecCreate(((char *)decoder_state) + 0xD9048, decoder_buffer_address + 0x1C85C0, 0xEB768,
                  ((char *)decoder_state) + 0x52040, decoder_buffer_address + 0x1C7180, 0x100,
                  ((char *)decoder_state) + 0xD6040, 0x200);
    audioDecCreate(((char *)decoder_state) + 0xD9100, ((char *)decoder_state) + 0x50040, 0x2000,
                  decoder_buffer_address + 0x1C8190);
    videoDecSetStream(((char *)decoder_state) + 0xD9048, 0, 0, videoCallback, decoder_state);
    videoDecSetStream(((char *)decoder_state) + 0xD9048, 3, callback_context, pcmCallback, decoder_state);
    voBufCreate(((char *)decoder_state) + 0xD9168,
                  (decoder_buffer_address & 0x0FFFFFFF) | 0x20000000,
                  decoder_buffer_address + 0x1A0000, 2);
    decoder_thread.stack = ((char *)decoder_state) + 0xD2040;
    decoder_thread.entry = videoDecMain;
    decoder_thread.stack_size = 0x4000;
    decoder_thread.priority = 1;
    decoder_thread.gp = D_00166D00;
    decoder_thread.gs_tag = 0;
    thread_id = func_00118B50(&decoder_thread);
    D_00161310 = thread_id;
    func_00118B70(thread_id, ((char *)decoder_state) + 0xD9048);
    if (set_state_fields(((char *)decoder_state) + 0xD9040, stream_source, source_mode) == 0) {
        opened = 0;
        STUB_printf(movie_open_error_text);
    } else {
        opened = 1;
    }
    ((struct AudioDecoderState *)decoder_state)->intc_handler_id = func_00118A90(2, vblankHandler, 0);
    func_00119390(2);
    ((struct AudioDecoderState *)decoder_state)->dmac_handler_id = func_00118AB0(2, handler_endimage, 0);
    func_00119460(2);
    return opened;
}
extern int movie_thread_gp SDATA(D_00161310);
extern int func_00120F30(int);
extern void func_0023CD28(char*);
extern void func_0023E5B0(char*);
extern int func_00118B80(int);
extern int func_00118B60(int);
extern int func_001193F8(int);
extern int func_00118AD0(int,int);
extern int func_00119328(int);
extern int func_00118AA0(int,int);
extern int func_0023E008(char*);
extern int func_0023C060(char*);
extern int func_0023CE28(char*);
/* termAll(void): shuts down workers and decoders, then clears the DMA enable bit. */
void termAll(void) {
    func_00120F30(0);
    readBufDelete(decoder_state);
    voBufDelete(decoder_state+0xD9168);
    func_00118B80(movie_thread_gp);
    func_00118B60(movie_thread_gp);
    func_001193F8(2);
    func_00118AD0(2,*(int*)(decoder_state+0xD90F8));
    func_00119328(2);
    func_00118AA0(2,*(int*)(decoder_state+0xD90FC));
    videoDecDelete(decoder_state+0xD9048);
    audioDecDelete(decoder_state+0xD9100);
    get_state_callback_result(decoder_state+0xD9040);
    func_00120F30(0);
    *(volatile unsigned int*)0x1000E000 &= ~2u;
}
extern void func_001E9730(char *, ...);
extern char D_001612F8[];

/* ErrMessage -- passes the message to func_001E9730 (STUB_printf) with the string at
 * D_001612F8 as its first argument. */
void ErrMessage(char *fmt) {
    STUB_printf(movie_error_message, fmt);
}
extern void func_0023C2C0(char *);

/* proceedAudio(void) -- audioDecSend on the audio decoder at offset 0xD9100 of the movie state. */
void proceedAudio(void) {
    audioDecSend(decoder_state + 0xD9100);
}
