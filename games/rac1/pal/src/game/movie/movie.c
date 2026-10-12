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
extern void func_0023B740(void*,void*,void*);
extern void func_0023BE38(void);
/* No recovered name. Sets movie globals, runs initialization and playback, then clears them. */
int func_0023B670(int a,int b,int c,char *state,int e) {
    D_00161308=c;
    D_0016130C=state;
    D_00161314=0;
    movie_clear_gp=0;
    func_00118BA0(func_00118BE0(),1);
    D_00161314=1;
    if(initAll(a,b,e)) {
        char *base=movie_gp_base;
        D_00161314=2;
        func_0023B740(base+0xD9048,base,base+0xD9040);
    }
    termAll();
    D_00161308=0;
    D_0016130C=0;
    return 0;
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023B740);
extern int func_00118BC0(int);

/* switchThread -- calls func_00118BC0(1). */
void func_0023BB40(void) {
    func_00118BC0(1);
}
extern int func_0023C2B0(char *);

/* isAudioOK -- true when the audio decoder at offset 0xD9100 of the movie state holds 0x1000
 * bytes or more. */
int func_0023BB60(void) {
    return audioDecIsPageFull(D_0016130C + 0xD9100);
}
INCLUDE_ASM("asm/nonmatchings/text", func_0023BB90); /* initAll(int, int, int) */
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
void func_0023BE38(void) {
    func_00120F30(0);
    readBufDelete(D_0016130C);
    voBufDelete(D_0016130C+0xD9168);
    func_00118B80(movie_thread_gp);
    func_00118B60(movie_thread_gp);
    func_001193F8(2);
    func_00118AD0(2,*(int*)(D_0016130C+0xD90F8));
    func_00119328(2);
    func_00118AA0(2,*(int*)(D_0016130C+0xD90FC));
    videoDecDelete(D_0016130C+0xD9048);
    audioDecDelete(D_0016130C+0xD9100);
    strFileDelete(D_0016130C+0xD9040);
    func_00120F30(0);
    *(volatile unsigned int*)0x1000E000 &= ~2u;
}
extern void func_001E9730(char *, ...);
extern char D_001612F8[];

/* ErrMessage -- passes the message to func_001E9730 (STUB_printf) with the string at
 * D_001612F8 as its first argument. */
void func_0023BF48(char *fmt) {
    STUB_printf(D_001612F8, fmt);
}
extern void func_0023C2C0(char *);

/* proceedAudio(void) -- audioDecSend on the audio decoder at offset 0xD9100 of the movie state. */
void func_0023BF70(void) {
    audioDecSend(D_0016130C + 0xD9100);
}
