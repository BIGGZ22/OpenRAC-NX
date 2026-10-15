#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * music.cpp in the original source; text 0x2161E0-0x217588.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);
extern void func_002023E0(int);
extern void func_002027C0(int);
extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;
extern int D_001A0468[];
extern void func_00205830(int a, int b);
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");
extern int D_001A01F0[];
extern int *D_001602E0;
extern unsigned char D_0013D49C NOT_SDA;
extern unsigned char D_0013D49D NOT_SDA;
extern unsigned char D_0013D4A5 NOT_SDA;
extern short D_0015FE24;
extern unsigned char D_0013D4AC NOT_SDA;
extern unsigned char D_0013D4AD NOT_SDA;
extern unsigned char D_0013D4AE NOT_SDA;
extern unsigned char D_0013D4AF NOT_SDA;
extern unsigned char D_0013D4B5 NOT_SDA;
extern int D_001A04B4 NOT_SDA;
extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;
extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;
extern unsigned char D_0013D4E0;
extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;
extern unsigned char D_0013D4E9 NOT_SDA;
extern unsigned char D_0013D502 NOT_SDA;
extern unsigned char D_0013D503 NOT_SDA;
extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);
extern short D_0015EE84;
extern int D_0015EE84_far __asm__("D_0015EE84") NOT_SDA;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;
extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);
extern int func_001236F0(void);
extern int func_001E9730();
extern char D_001E8690[];
extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;
extern int D_0013D9B4 NOT_SDA;
extern unsigned char D_0013D490[];
extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;
extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;
extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;
extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;
extern int D_0013D5C8 NOT_SDA;
extern unsigned char D_0013D4B0 NOT_SDA;
extern unsigned char D_0013DE55 NOT_SDA;
extern unsigned char D_0013D5DD NOT_SDA;
extern unsigned char D_0013D5E7 NOT_SDA;
extern int D_001B2F40[];
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_00118D80(int);
extern char D_00165600[];
extern int D_0015F718;
extern short D_0015F71C;
extern char D_001B3200[];
extern int func_001160D8(void);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9DC0(void *, void *, float);
extern void func_001FA460(void *);
extern void func_002150B0(void *, void *);
extern void func_001FA480(void *, void *);
extern float func_0020D830(void);
extern float func_00215A98(int, float);
extern unsigned char D_0014BFC0[];
extern unsigned char D_0013E620[];
extern unsigned char D_0013D510[];

extern short D_001517D0[];
extern int func_0012EC60(int, int, int, int);
extern int func_0012DDC0(void);
extern void func_00216270(void);

/* Music init: resets the music state at D_001517D0 (+0x30 = 0x20, the
   other bytes and the three playing records' state and +0x0A flag
   cleared, +0x1C/+0x22 set to -1), starts VAG streaming through
   func_0012EC60 (snd_InitVAGStreamingEx), waits until func_0012DDC0
   (snd_FlushSoundCommands) has nothing left, and calls func_00216270.
   The two -1 stores are one chained assignment written last: that
   shares one register between the word and the byte store, and leaves
   the other stores in source order, as retail has them. */
void func_002161E0(void) {
    char *s = (char *)D_001517D0;

    *(char *)(s + 0x30) = 0x20;
    *(int *)(s + 0x00) = 0;
    *(char *)(s + 0x31) = 0;
    *(char *)(s + 0x32) = 0;
    *(char *)(s + 0x33) = 0;
    *(int *)(s + 0x34) = 0;
    *(short *)(s + 0x3E) = 0;
    *(int *)(s + 0x50) = 0;
    *(short *)(s + 0x5A) = 0;
    *(int *)(s + 0x6C) = 0;
    *(short *)(s + 0x76) = 0;
    *(char *)(s + 0x22) = *(int *)(s + 0x1C) = -1;
    func_0012EC60(4, 0xF000, 0, 1);
    while (func_0012DDC0() != 0) {
    }
    register_audio_stream_callback();
}

extern void func_0012F068(void *);
extern void func_002177F0(int);

void func_00216270(void) {
    func_0012F068(func_002177F0);
}

/* The sound tables in the WAD table of contents at D_00137C80, as far as
   the play functions show them: 8-byte records per sound id whose first
   word is the stream handle, except the 50000 range, which has six
   handles per id (D_0015EE88 picks one). The [1] bounds are placeholders,
   not known counts. Reaching a table as a member of this struct, rather
   than by pointer arithmetic, is what gives retail's addu operand order.

   D_001517D0 holds three playing records (music_Playing, 0x1C bytes)
   at 0x34, 0x50 and 0x6C: +0x00 state (-1 while starting), +0x04 the
   id, +0x06 the value also passed as func_0012ED48's fifth argument,
   +0x08 another argument, +0x0A and +0x10 flags, +0x14 10, +0x18 48000
   (the sample rate). The stores are written in the order retail emits
   them. The first one must not need a constant in a register: if it
   does, the allocator spends $a0 on the constant, and the handle can no
   longer load straight into $a0. An older note here recorded that
   residual as an allocator dead end. */
typedef struct {
    int handle;
    int unk_04;
} SndRec;
typedef struct {
    char _pad0000[0x1A0];
    SndRec ids10000[1]; /* 0x01A0 */
    char _pad01A8[0xF00 - 0x1A8];
    int ids50000[1][6]; /* 0x0F00 */
    char _pad0F18[0x13C0 - 0xF18];
    SndRec ids40000[1]; /* 0x13C0 */
    char _pad13C8[0x1618 - 0x13C8];
    SndRec ids60000[1]; /* 0x1618 */
    char _pad1620[0x2988 - 0x1620];
    SndRec ids20000[1]; /* 0x2988 */
} SndToc;
extern int D_00137C80[];
extern short D_001517D0[];
extern int D_0015EE88 MACRO_ADDR;
extern void func_00217860(int, long);
extern void func_0012ED48(int, int, int, int, short, int, int, int,
                          int, void (*)(int, long), long);

/* Plays sound arg0 in 60000..: the other play functions below differ
   only in their table. */
void func_00216290(int arg0, int arg1, int arg2) {
    SndToc *toc = (SndToc *)D_00137C80;
    int i = arg0 - 60000;
    int h;
    char *s;

    h = toc->ids60000[i].handle;
    if (h == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

void func_00216368(int arg0, int arg1, int arg2) {
    SndToc *toc = (SndToc *)D_00137C80;
    int i = arg0 - 50000;
    int h;
    char *s;

    h = toc->ids50000[i][D_0015EE88];
    if (h == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

void func_00216450(int arg0, int arg1, int arg2) {
    SndToc *toc = (SndToc *)D_00137C80;
    int i = arg0 - 40000;
    int h;
    char *s;

    h = toc->ids40000[i].handle;
    if (h == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

/* A byte flag; declared int-sized so MACRO_ADDR keeps it off $gp (-G2). */
extern int D_0015EF1C_w __asm__("D_0015EF1C") MACRO_ADDR;
extern char D_0011C278[];
extern char D_00151820[];

/* The same through a per-language table at D_0011C278; volume only
   when the D_0015EF1C flag is set. */
void func_00216528(int arg0, int arg1, int arg2) {
    int *p = (int *)(D_0011C278 + (arg0 * 4 + D_0015EE88 * 0x258));
    char *s;

    if (*p == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(*p, 0, 0, 0, *(unsigned char *)&D_0015EF1C_w ? arg2 : 0,
                  0, 2, 0, 0x21, func_00217860, (long)(unsigned int)D_00151820);
}

void func_00216620(int arg0, int arg1, int arg2) {
    SndToc *toc = (SndToc *)D_00137C80;
    int i = arg0 - 20000;
    int h;
    char *s;

    h = toc->ids20000[i].handle;
    if (h == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

void func_002166F0(int arg0, int arg1, int arg2) {
    SndToc *toc = (SndToc *)D_00137C80;
    int i = arg0 - 10000;
    int h;
    char *s;

    h = toc->ids10000[i].handle;
    if (h == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

extern char D_0013A764[];

/* music_Play dispatcher: routes to the range-specific play function
   (siblings above, one per id range) for arg0 >= 10000; below that it's
   the same music_Playing setup those siblings do, inline, with the sound
   handle read from D_0013A764[arg0][D_0015EE88] (stride 0x250) instead of
   a SndToc table. */
void func_002167C0(int arg0, int arg1, int arg2) {
    char *s;
    int handle;

    if (arg0 >= 60000) {
        func_00216290(arg0, arg1, arg2);
        return;
    }
    if (arg0 >= 50000) {
        func_00216368(arg0, arg1, arg2);
        return;
    }
    if (arg0 >= 40000) {
        func_00216450(arg0, arg1, arg2);
        return;
    }
    if (arg0 >= 30000) {
        func_00216528(arg0, arg1, arg2);
        return;
    }
    if (arg0 >= 20000) {
        func_00216620(arg0, arg1, arg2);
        return;
    }
    if (arg0 >= 10000) {
        func_002166F0(arg0, arg1, arg2);
        return;
    }
    handle = *(int *)((char *)D_0013A764 + arg0 * 0x250 + D_0015EE88 * 4);
    if (handle == 0) {
        return;
    }
    s = (char *)D_001517D0;
    if (*(int *)(s + 0x50) != 0) {
        return;
    }
    *(unsigned int *)(s + 0x50) = 0xFFFFFFFF;
    *(short *)(s + 0x5A) = 1;
    *(short *)(s + 0x54) = arg0;
    *(short *)(s + 0x58) = arg1;
    *(int *)(s + 0x64) = 10;
    *(int *)(s + 0x68) = 0xBB80;
    *(short *)(s + 0x56) = arg2;
    *(short *)(s + 0x60) = 0;
    func_0012ED48(handle, 0, 0, 0, arg2, 0, 2, 0, 0x21,
                  func_00217860, (long)(unsigned int)(s + 0x50));
}

extern short D_001517D0[];
extern void func_0012EDE0(void *);

int func_00216960(void) {
    char *d = (char *)D_001517D0;
    void *p = *(void **)(d + 0x50);
    if (p != 0) {
        if (*(short *)(d + 0x5A) == 3) {
            func_0012EDE0(p);
            *(short *)(d + 0x5A) = 4;
            return 1;
        }
    }
    return 0;
}

extern void func_002178C0(int, long);

/* music_PreseekTrack(int, int, int) */
void func_002169B8(int arg0, int arg1, int arg2) {
    char *s = (char *)D_001517D0;
    char *base;
    int *tbl;
    long h;

    if (*(int *)(s + 0x34) != 0) {
        return;
    }
    base = (char *)D_00137C80;
    tbl = (int *)(base + 0x2AA8);
    if (tbl[arg0] == 0) {
        return;
    }
    *(short *)(s + 0x3E) = 1;
    h = tbl[arg0];
    *(short *)(s + 0x38) = arg0;
    *(int *)(s + 0x48) = 10;
    *(int *)(s + 0x4C) = 0xBB80;
    *(unsigned int *)(s + 0x34) = 0xFFFFFFFF;
    *(short *)(s + 0x3C) = arg1;
    *(short *)(s + 0x3A) = arg2;
    *(short *)(s + 0x44) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 1, 0, 0x21,
                  func_002178C0, (long)(unsigned int)(s + 0x34));
}

extern void func_002179C8(int, long);

/* music_StartTrack(int, int, int) */
void func_00216A90(int arg0, int arg1, int arg2) {
    char *s = (char *)D_001517D0;
    char *base;
    int *tbl;
    long h;

    if (*(int *)(s + 0x34) != 0) {
        return;
    }
    base = (char *)D_00137C80;
    tbl = (int *)(base + 0x2AA8);
    if (tbl[arg0] == 0) {
        return;
    }
    *(short *)(s + 0x3E) = 1;
    h = tbl[arg0];
    *(short *)(s + 0x38) = arg0;
    *(int *)(s + 0x48) = 10;
    *(int *)(s + 0x4C) = 0xBB80;
    *(unsigned int *)(s + 0x34) = 0xFFFFFFFF;
    *(short *)(s + 0x3C) = arg1;
    *(short *)(s + 0x3A) = arg2;
    *(short *)(s + 0x44) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 1, 0, 0x20,
                  func_002179C8, (long)(unsigned int)(s + 0x34));
}

extern void func_00217970(int, long);

/* music_StartTrackBody(int, int, int): when the music record at +0x34 is
   already playing (its word neither 0 nor -1) and not in state 9, starts
   track arg0 + 1 of the table at D_00137C80 + 0x2AA8 on it: state 9, the
   track, arg1 and arg2 recorded, 10 and 48000 stored, then
   func_0012ED48 with the current word passed on as its eighth argument
   and 0x24 or 0x20 by arg1's bit 0. The handle is read twice as in
   func_00216A90 (CSE merges the loads, and the address stays in a
   register as in retail), and the 0x24/0x20 choice comes before the
   stores, which keeps arg1 in $a1. */
void func_00216B68(int arg0, int arg1, int arg2) {
    char *s = (char *)D_001517D0;
    char *base;
    int *tbl;
    unsigned int cur;
    long h;
    int i;
    int flags;

    if (*(short *)(s + 0x3E) == 9) {
        return;
    }
    cur = *(unsigned int *)(s + 0x34);
    if (cur == 0 || cur == 0xFFFFFFFF) {
        return;
    }
    base = (char *)D_00137C80;
    tbl = (int *)(base + 0x2AA8);
    i = arg0 + 1;
    if (tbl[i] == 0) {
        return;
    }
    h = tbl[i];
    flags = (arg1 & 1) ? 0x24 : 0x20;
    *(short *)(s + 0x38) = arg0;
    *(short *)(s + 0x3C) = arg1;
    *(short *)(s + 0x3E) = 9;
    *(int *)(s + 0x48) = 10;
    *(int *)(s + 0x4C) = 0xBB80;
    *(short *)(s + 0x3A) = arg2;
    *(short *)(s + 0x44) = 0;
    func_0012ED48(h, 0, 0, 0, arg2, 0, 1, cur, flags,
                  func_00217970, (long)(unsigned int)(s + 0x34));
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern void func_00217920(int, long);

/* The success path ends in the call, so `return 1` does not break the
   block's trailing call run (the scheduler then keeps retail's order). */
int func_00216C50(int arg0, int arg1, int arg2, int arg3) {
    char *s = (char *)D_001517D0;
    char *base;
    int *tbl;
    long h;

    if (*(int *)(s + 0x6C) == 0) {
        base = (char *)D_00137C80;
        tbl = (int *)(base + 0x2AA8);
        if (tbl[arg1] != 0) {
            *(short *)(s + 0x74) = arg2;
            h = tbl[arg1];
            *(int *)(s + 0x80) = 10;
            *(int *)(s + 0x84) = 0xBB80;
            *(unsigned int *)(s + 0x6C) = 0xFFFFFFFF;
            *(short *)(s + 0x70) = arg0;
            *(short *)(s + 0x76) = 1;
            *(short *)(s + 0x7C) = 1;
            *(short *)(s + 0x72) = arg3;
            func_0012ED48(h, 0, 0, 0, arg3, 0, 1, 0, 0x20,
                          func_00217920, (long)(unsigned int)(s + 0x6C));
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    return 1;
}

/* A block-local `char *e` for the final stores (%hi kept, %lo rebuilt),
   written in the order the rotation rule gives. */
void func_00216D30(int arg0, int arg1) {
    char *d = (char *)D_001517D0;

    if (*(short *)(d + 0x38) == arg0) {
        if (*(signed char *)(d + 0x22) == -1) {
            return;
        }
    }
    if (*(short *)(d + 0x3E) == 0) {
        if (*(short *)(d + 0x76) == 0) {
            *(short *)(d + 0x38) = arg0;
            return;
        }
    }
    {
        char *e = (char *)D_001517D0;
        e[0x22] = arg0;
        e[0x23] = arg1;
    }
}

extern void func_0012ED10(void);
extern void func_0012EE70(int);

/* music_Stop: waits (func_0012DDC0, snd_FlushSoundCommands) while each
   of the three playing records at +0x34, +0x6C and +0x50 is still
   starting (state 0xFFFFFFFF), stops the streams (func_0012ED10), flushes
   until nothing is left, calls func_0012EE70(1), then clears the records
   and the current id (+0x22 kept in +0x38 when set). Each wait loop has
   its own pointer assigned in the loop condition: jump.c's copy of the
   exit test in front of the loop then gets its own pseudo, and the loop
   keeps retail's copy in $s1. The stores are in the order that gives
   retail's schedule. */
void func_00216D88(void) {
    {
        char *d;
        while (d = (char *)D_001517D0, *(unsigned int *)(d + 0x34) == 0xFFFFFFFF) {
            func_0012DDC0();
        }
    }
    {
        char *d;
        while (d = (char *)D_001517D0, *(unsigned int *)(d + 0x6C) == 0xFFFFFFFF) {
            func_0012DDC0();
        }
    }
    {
        char *d;
        while (d = (char *)D_001517D0, *(unsigned int *)(d + 0x50) == 0xFFFFFFFF) {
            func_0012DDC0();
        }
    }
    func_0012ED10();
    while (func_0012DDC0() != 0) {
    }
    func_0012EE70(1);
    {
        char *s = (char *)D_001517D0;
        *(short *)(s + 0x3E) = 0;
        *(short *)(s + 0x3C) = 0;
        *(int *)(s + 0x34) = 0;
        if (*(signed char *)(s + 0x22) != -1) {
            *(short *)(s + 0x38) = *(signed char *)(s + 0x22);
        }
        *(short *)(s + 0x5A) = 0;
        *(short *)(s + 0x58) = 0;
        *(int *)(s + 0x50) = 0;
        *(short *)(s + 0x76) = 0;
        *(short *)(s + 0x74) = 0;
        *(int *)(s + 0x6C) = 0;
        *(short *)(s + 0x20) = 0;
        *(char *)(s + 0x22) = -1;
        *(char *)(s + 0x23) = -1;
    }
}

extern short D_001517D0[];

/* music_Pause(int) */
void func_00216EF0(int arg0) {
    if (arg0 != 0) {
        D_001517D0[0x2F] = 0;
        D_001517D0[0x2E] = -0x8000;
    }
    D_001517D0[0x3D] = 0;
    D_001517D0[0x20] = -0x8000;
    D_001517D0[0x21] = 0;
    D_001517D0[0x3C] = -0x8000;
}

/* music_Unpause(void) */
void func_00216F28(void) {
    short *p = D_001517D0;
    p[0x20] = 4;
    p[0x3C] = 4;
    p[0x2E] = 4;
}

typedef struct {
    unsigned int handle; /* 0x00: 0 none, 0xFFFFFFFF starting/released */
    short id;            /* 0x04 */
    short unk06;
    short unk08;
    short state;         /* 0x0A: low bits the state, 0x8000 paused */
    short fade;          /* 0x0C: 0x8000 fading */
    short fadeT;         /* 0x0E */
} MusicPlaying;

extern void func_0012E4A8(int);
extern void func_0012EDB0(int);
extern int func_001F9938(void *);
extern void func_0012EE10(int, void (*)(int, long), long);
extern void func_0012E588(int, void (*)(int, long), long);
extern void func_0012EE40(int, void (*)(int, long), long);
extern void func_00217A60(int, long);
extern void func_00217A08(int, long);
extern void func_00217830(int, long);

/* music_UpdateStream(music_Playing &): with a live handle and state other
   than 9, state 5 stops the stream (state 6) and state 6 without a
   handle resets; a fading record (+0xC bit 15) pauses the handle once
   (+0xA bit 15) and polls func_001F9938 until it reports 2 (+0xC = 4),
   otherwise a paused one is resumed. Unpaused, states 1/8/9 are left
   alone, 2 hands the handle to func_0012EE40 unless it is 0xFFFFFFFF,
   and anything but 2/3 releases the handle through func_0012EE10 and
   func_0012E588. With no live handle (or state 9) the state is cleared
   when it is 7 or the handle is 0. The handle is unsigned (retail builds
   0xFFFFFFFF with lui/ori), the special case is the else arm (retail
   places it last), and the dispatch reads p->state directly so the 2/3
   range test works on the loaded halfword as retail's does. */
void func_00216F48(MusicPlaying *p) {
    int h;

    if (p->state != 9 && p->handle != 0 && p->handle != 0xFFFFFFFF) {
        if (p->state == 5) {
            if (p->handle != 0) {
                func_0012E4A8(p->handle);
                p->state = 6;
            } else {
                p->state = 0;
            }
        } else if (p->state == 6) {
            if (p->handle == 0) {
                p->state = 0;
            }
        }
        if (p->handle == 0) {
            return;
        }
        if (p->fade & 0x8000) {
            if (!(p->state & 0x8000)) {
                func_0012EDB0(p->handle);
                p->state |= 0x8000;
            }
            if (FastDecTimer(&p->fadeT) == 2) {
                p->fade = 4;
            }
        } else if (p->state & 0x8000) {
            func_0012EDE0((void *)p->handle);
            p->state ^= 0x8000;
        }
        if (p->state & 0x8000) {
            return;
        }
        if (p->state == 1 || p->state == 8 || p->state == 9) {
            return;
        }
        if (p->state != 2 && p->state != 3) {
            h = p->handle;
            p->handle = 0xFFFFFFFF;
            func_0012EE10(h, func_00217A60, (long)(unsigned int)p);
            func_0012E588(h, func_00217A08, (long)(unsigned int)p);
            return;
        }
        if (p->handle != 0xFFFFFFFF && p->state == 2) {
            func_0012EE40(p->handle, func_00217830, (long)(unsigned int)p);
        }
    } else if (p->state == 7 || p->handle == 0) {
        p->state = 0;
    }
}

struct MusicStreamChannel {
    s32 handle; /* 0x00: stream handle from the start callback; -1 while starting */
    s16 track; /* 0x04 */
    s16 volume; /* 0x06 */
    s16 flags; /* 0x08 */
    s16 state; /* 0x0A: set 1 on start, 1 -> 2 in the start callback */
    s16 fade_flags; /* 0x0C: -0x8000 on music_pause, 4 on music_unpause */
    s16 unkE; /* 0x0E: cleared by music_pause */
    s16 crossfade_enabled; /* 0x10: remaining-time callback starts the crossfade */
    u8 pad_12[0x2];
    s32 poll_interval; /* 0x14 */
    s32 remaining_time; /* 0x18: set by music_remaining_time_callback */
};
struct MusicCdReadMode {
    u8 trycount; /* 0x20 from reset_music */
    u8 spindlctrl;
    u8 datapattern;
    u8 pad;
};
struct MusicStreamState {
    s32 unk0; /* 0x00: cleared by reset_music */
    u8 pad_4[0x4];
    s16 read_state; /* 0x08: 0 idle, 1 reading, 2 done */
    u8 break_requested; /* 0x0A: set by request_audio_stream_break */
    u8 updates_suspended; /* 0x0B: set around space transitions */
    s32 read_sector; /* 0x0C */
    s32 read_sector_count; /* 0x10 */
    s32 read_dst; /* 0x14 */
    u8 pad_18[0x4];
    s32 queued_secondary_track; /* 0x1C: -1 = none; overlays queue track + 40000 (l13 lines: + 50000) and wait for secondary.state == 3 */
    s16 crossfade_state; /* 0x20: 1 -> 2 in music_remaining_time_callback */
    s8 requested_track; /* 0x22: -1 = none */
    s8 requested_transition_track; /* 0x23 */
    s32 crossfade_remaining_time; /* 0x24 */
    s32 crossfade_interval; /* 0x28: remaining time / 4 */
    s32 retry_timer; /* 0x2C */
    struct MusicCdReadMode cd_mode; /* 0x30: 4th sceCdRead argument */
    struct MusicStreamChannel primary; /* 0x34 */
    struct MusicStreamChannel secondary; /* 0x50 */
    struct MusicStreamChannel transition; /* 0x6C */
};
extern struct MusicStreamState D_001517D0_17130 __asm__("D_001517D0");
extern void func_0012E600(s32, s32, s32, s32, s32, s32, void (*)(u32, s64), s64);
extern void func_00217A08_17130(u32, s64) __asm__("func_00217A08");
extern void func_0012EDE0_17130(s32) __asm__("func_0012EDE0");
extern s32 func_0012EF48(s32);
extern s32 func_001F98C0(s32);
extern s32 func_001F9908(void *);
extern void func_002167C0(s32, s32, s32);
extern void func_002169B8(s32, s32, s32);
extern void func_00216A90(s32, s32, s32);
extern void func_00216B68(s32, s32, s32);
extern s32 func_00216C50(s32, s32, s32, s32);
extern void func_00216F48_17130(void *) __asm__("func_00216F48");
extern s32 func_00217628(s32, s32, s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/audio/music/music_update.c, music_update. */
void func_00217130(void) {
    s8 requested_track;
    s32 fade_volume;
    s32 handle;
    s16 pending_state;

    if (D_001517D0_17130.updates_suspended != 0) {
        return;
    }
    if (!(D_001517D0_17130.primary.fade_flags & 0x8000) && !(D_001517D0_17130.primary.state & 0x8000)) {
        if (D_001517D0_17130.primary.handle == 0 && (D_001517D0_17130.primary.flags & 1) &&
            D_001517D0_17130.requested_track == -1) {
            music_StartTrack(D_001517D0_17130.primary.track, (s16)D_001517D0_17130.primary.flags,
                              D_001517D0_17130.primary.volume);
        } else if (D_001517D0_17130.primary.state != 9 && D_001517D0_17130.primary.handle != 0) {
            if (D_001517D0_17130.primary.handle != 0xFFFFFFFF && D_001517D0_17130.primary.state == 8) {
                music_StartTrackBody(D_001517D0_17130.primary.track, D_001517D0_17130.primary.flags,
                                       D_001517D0_17130.primary.volume);
            }
        }
    }
    if (func_001F9908(&D_001517D0_17130.retry_timer) != 0) {
        requested_track = D_001517D0_17130.requested_track;
        if (requested_track != -1 && D_001517D0_17130.crossfade_state == 0) {
            if (D_001517D0_17130.primary.track != requested_track) {
                if (D_001517D0_17130.requested_transition_track == -1 ||
                    music_Transition(requested_track, D_001517D0_17130.requested_transition_track,
                                     D_001517D0_17130.primary.flags, D_001517D0_17130.primary.volume) != 0) {
                    D_001517D0_17130.retry_timer = func_001F98C0(7) * 60.0f;
                }
            } else {
                D_001517D0_17130.requested_track = -1;
            }
        }
    }
    if (D_001517D0_17130.queued_secondary_track >= 0) {
        if (D_001517D0_17130.secondary.handle != 0) {
            if ((u16)D_001517D0_17130.secondary.state - 6 >= 2U) {
                D_001517D0_17130.secondary.state = 5;
            }
        } else {
            music_start_track_by_id(D_001517D0_17130.queued_secondary_track, 0, 0x400);
            D_001517D0_17130.queued_secondary_track = -1;
        }
    }
    if (D_001517D0_17130.primary.state != 9 && D_001517D0_17130.primary.handle != 0xFFFFFFFF &&
        !(D_001517D0_17130.primary.fade_flags & 0x8000) && !(D_001517D0_17130.primary.state & 0x8000) &&
        !(D_001517D0_17130.transition.fade_flags & 0x8000) && !(D_001517D0_17130.transition.state & 0x8000)) {
        switch (D_001517D0_17130.crossfade_state) {
        case 2:
            /* Fade the primary stream against the transition's remaining time. */
            fade_volume =
                D_001517D0_17130.primary.volume *
                (D_001517D0_17130.crossfade_interval -
                 (D_001517D0_17130.crossfade_remaining_time - D_001517D0_17130.transition.remaining_time)) /
                D_001517D0_17130.crossfade_interval;
            if (fade_volume <= 0 ||
                ((D_001517D0_17130.transition.state != 4 ||
                  D_001517D0_17130.transition.crossfade_enabled == 0) &&
                 D_001517D0_17130.transition.handle == 0) ||
                (handle = D_001517D0_17130.primary.handle) == 0) {
                D_001517D0_17130.crossfade_state = 3;
                D_001517D0_17130.primary.state = 5;
            } else if (D_001517D0_17130.primary.state != 9) {
                /* The callback receives the old handle; mark its slot pending before submission. */
                *(u32 *)&D_001517D0_17130.primary.handle = 0xFFFFFFFF;
                func_0012E600(handle, 5, fade_volume, 0, 0, 0, func_00217A08_17130,
                                        (s64)&D_001517D0_17130.primary.handle);
            }
            break;
        case 3:
            if (D_001517D0_17130.primary.state == 0 && (D_001517D0_17130.transition.crossfade_enabled != 0 ||
                                                   D_001517D0_17130.transition.handle == 0)) {
                music_PreseekTrack(D_001517D0_17130.requested_track, D_001517D0_17130.primary.flags,
                                    D_001517D0_17130.primary.volume);
                D_001517D0_17130.requested_track = -1;
                D_001517D0_17130.crossfade_state = 4;
            }
            break;
        case 4:
            if (D_001517D0_17130.primary.state == 3) {
                if (D_001517D0_17130.transition.remaining_time < D_001517D0_17130.crossfade_interval ||
                    D_001517D0_17130.transition.state != 4 ||
                    (D_001517D0_17130.transition.crossfade_enabled == 0 &&
                     D_001517D0_17130.transition.handle == 0)) {
                    func_0012EDE0_17130(D_001517D0_17130.primary.handle);
                    D_001517D0_17130.primary.state = 8;
                    D_001517D0_17130.crossfade_state = 5;
                }
            }
            break;
        case 5:
            if (D_001517D0_17130.transition.state != 4 ||
                D_001517D0_17130.transition.crossfade_enabled == 0) {
                D_001517D0_17130.crossfade_state = 0;
            }
            break;
        }
    }
    func_00216F48_17130(&D_001517D0_17130.primary.handle);
    func_00216F48_17130(&D_001517D0_17130.transition.handle);
    func_00216F48_17130(&D_001517D0_17130.secondary.handle);
    if (D_001517D0_17130.break_requested != 0) {
        if (func_0012EF48(1) == 0) {
            D_001517D0_17130.read_state = 0;
            D_001517D0_17130.break_requested = 0;
        }
    } else {
        pending_state = D_001517D0_17130.read_state;
        if (pending_state == 2) {
            D_001517D0_17130.read_state = 0;
            start_audio_stream_read(D_001517D0_17130.read_dst, D_001517D0_17130.read_sector,
                                    D_001517D0_17130.read_sector_count);
            if (D_001517D0_17130.read_state == 0) {
                D_001517D0_17130.read_state = pending_state;
            }
        }
    }
}
