#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * help.cpp in the original source; text 0x1FDF10-0x1FF668.
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

extern int D_001941C0[];
extern int D_0016100C MACRO_ADDR;

/* Start of a `size`-byte block at the end of each of the two VU1 chain
   buffers (D_001941C0[1]/[2], D_0016100C bytes long, as VU1_initChain
   uses them). Both outputs are 0 and the result -1 when size is over
   0x20000. */
/* Returns int; the size is unsigned and D_0016100C MACRO_ADDR. */
int func_001FDF10(unsigned int size, int *out1, int *out2) {
    if (size > 0x20000) {
        *out1 = 0;
        *out2 = 0;
        return -1;
    }
    *out1 = D_001941C0[1] + D_0016100C - size;
    *out2 = D_001941C0[2] + D_0016100C - size;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_001FDF78);

typedef struct {
    int state;    /* 0x00 */
    int x04;      /* 0x04 */
    int pad[7];   /* 0x08 */
    int x24;      /* 0x24 */
    int x28;      /* 0x28 */
    int count;    /* 0x2C: entries in the D_0015F780 table */
} HelpState;
extern char D_001997D0[];

/* Advances the help screen's state (D_001997D0). Every access goes
   through the global, and cases 6 and 7 are spelled out: the jump table
   has eight entries. */
void func_001FE438(void) {
    switch (((HelpState *)D_001997D0)->state) {
    case 0:
        ((HelpState *)D_001997D0)->x24 = -1;
        break;
    case 1:
    case 2:
        ((HelpState *)D_001997D0)->state = 7;
        ((HelpState *)D_001997D0)->x04 = 0;
        break;
    case 3:
        ((HelpState *)D_001997D0)->state = 7;
        ((HelpState *)D_001997D0)->x04 = 0;
        break;
    case 4:
        ((HelpState *)D_001997D0)->state = 6;
        ((HelpState *)D_001997D0)->x04 = 4 - ((HelpState *)D_001997D0)->x04;
        break;
    case 5:
        ((HelpState *)D_001997D0)->state = 6;
        ((HelpState *)D_001997D0)->x04 = 0;
        break;
    case 6:
    case 7:
        break;
    }
}

LINKER_REMNANT("asm/remnants/text", func_001FE4C0);

typedef struct {
    char *text;   /* 0x0 */
    int id;       /* 0x4 */
    int unk_08;
    int unk_0C;
} HelpEntry;
extern HelpEntry *D_0015F780 MACRO_ADDR;

/* Help_FindIndex: the index of the first D_0015F780 entry with this id,
   or -1. `r = -1` and a `break` give retail's peeled first compare and
   rotated loop; the count goes through the struct so its +0x2C stays a
   displacement. */
int func_001FE4D0(int id) {
    int r = -1;
    int i;

    for (i = 0; i < ((HelpState *)D_001997D0)->count; i++) {
        if (D_0015F780[i].id == id) {
            r = i;
            break;
        }
    }
    return r;
}

extern char D_00199A68[];

/* msg_string(int). The table load sits in the bgezl slot, where the
   MACRO_ADDR access becomes $gp-relative. */
void *func_001FE540(int id) {
    int i = Help_FindIndex(id);
    if (i >= 0) {
        return D_0015F780[i].text;
    }
    return D_00199A68;
}

LINKER_REMNANT("asm/remnants/text", func_001FE580);

extern int D_0015EF1D_i __asm__("D_0015EF1D") MACRO_ADDR;
extern int D_0015EF1C_i __asm__("D_0015EF1C") MACRO_ADDR;
#define D_0015EF1D_b (*(unsigned char *)&D_0015EF1D_i)
#define D_0015EF1C_b (*(unsigned char *)&D_0015EF1C_i)
extern int func_0022EE28(int, int, int);
extern void func_001F7648(void *arg0, int a1, int a2, int a3, int a4, int a5,
                          int a6, int a7, int a8);
extern void func_001F75D0(void *a, long b, void *c, int d);
extern int D_0013E604;

/* Opens the help window: state 1 (opening), the open sound unless both
   fade flags are clear, then the current entry's text is measured in a
   scratch FontSetWindow buffer and the box geometry goes into HelpState's
   pad[] for func_001FE6C0 to draw. The box's y is clamped so its bottom
   stays 12 above the screen's; written as `screenY - 0xC - hHalf5`, CSE
   folds it to retail's screenY - (h/2 + 0x11). The fade flags are bytes
   read through MACRO_ADDR int aliases, which keeps retail's lui+lbu. */
void func_001FE588(void) {
    short win[12];
    int idx;
    char *text;
    int screenY;
    int y0;
    short w, h;
    int hHalf5;

    ((HelpState *)D_001997D0)->state = 1;
    ((HelpState *)D_001997D0)->x04 = 0;
    if (D_0015EF1D_b || D_0015EF1C_b) {
        func_0022EE28(0, 1, 0);
    }

    idx = ((HelpState *)D_001997D0)->pad[6];
    text = D_0015F780[idx].text;
    FontSetWindow((void *)win, 0xF0, 0x1E0, 0x2C, 0x1D4, 0x100, 0x168, 0x10, 7);
    func_001F75D0((void *)win, 0x80FFA888L, text, -1);

    screenY = D_0013E604;
    w = win[6];
    h = win[7];
    hHalf5 = (h >> 1) + 5;
    y0 = screenY - 0x3C;

    ((HelpState *)D_001997D0)->pad[0] = (w >> 1) + 10;
    ((HelpState *)D_001997D0)->pad[1] = hHalf5;
    ((HelpState *)D_001997D0)->pad[2] = 0x100;
    ((HelpState *)D_001997D0)->pad[4] = 8;
    ((HelpState *)D_001997D0)->pad[5] = 8;
    ((HelpState *)D_001997D0)->pad[3] = y0;
    if (screenY - 0xC < y0 + hHalf5) {
        ((HelpState *)D_001997D0)->pad[3] = screenY - 0xC - hHalf5;
    }
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern int func_001F98C0(int);
extern int func_001F9908_r(int *arg0) __asm__("func_001F9908");
extern int func_00216028(int arg0, int arg1);
extern int func_00216960(void);
extern int D_0013CBE0;
extern int D_0013CBE4;
extern int D_0015F6E8 MACRO_ADDR;
extern int D_0015EFA4 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;

typedef struct {
    int state;     /* 0x00 */
    int timer;     /* 0x04 */
    int pad08[6];  /* 0x08 */
    int msg;       /* 0x20 */
    int pending;   /* 0x24 */
    int entry;     /* 0x28 */
    int count;     /* 0x2C */
    int started;   /* 0x30 */
    int delay;     /* 0x34 */
    short queued;  /* 0x38 */
    short shown;   /* 0x3A */
    int wait[1];   /* 0x3C */
} HelpSt;

typedef struct {
    unsigned short count;
    unsigned short best;
    unsigned int flags;
} HelpRecord;

typedef struct {
    unsigned char pad00[0x1C];
    int unk1C;
    unsigned char pad20[0x30];
    int unk50;
    short unk54;
    unsigned char pad56[0x4];
    short unk5A;
} PlayerHud;

extern HelpRecord D_00141A68[];
extern PlayerHud D_001517D0;

#define HS (*(HelpSt *)D_001997D0)
#define PLAY_TIME (func_001F98C0(D_0015EFA4) / 600)

#define RECORD_COUNT()                                   \
    if (D_00141A68[HS.entry].count < 0xFFFF) {           \
        D_00141A68[HS.entry].count++;                    \
    }

#define RECORD_BEST()                                    \
    if (PLAY_TIME > D_00141A68[HS.entry].best) {         \
        D_00141A68[HS.entry].best = PLAY_TIME;           \
    }

#define RECORD_FLAGS() \
    D_00141A68[HS.entry].flags = D_00141A68[HS.entry].flags | (1 << D_0015EE84) | 0x80000000

/* Advances the help popup's state machine: delay, open, hold, close, record stats. Adapted from Lombyte (MIT) for PAL: ui/help/update_help_state.c, update_help_state. */
void func_001FE6C0(void) {
    if (HS.started == 0) {
        if (HS.delay == 0 && (D_0013CBE0 & 0xF000)) {
            HS.delay = 1;
        }
        if (HS.delay != 0) {
            if (++HS.delay >= func_001F98C0(0x78)) {
                HS.started = 1;
            }
        }
    }

    if (D_0015F6E8 != 0 || HS.started == 0) {
        HS.pending = -1;
        HS.state = 0;
        HS.timer = 0;
        return;
    }

    HS.timer++;
    switch (HS.state) {
    case 8:
        if (HS.queued != 0) {
            break;
        }
        if (HS.msg >= 0) {
            if (HS.shown == 0) {
                if (func_001F9908_r(HS.wait) != 0) {
                    func_001FE588();
                    HS.shown++;
                }
                break;
            }
            RECORD_COUNT();
        }
        RECORD_BEST();
        RECORD_FLAGS();
        HS.msg = -1;
        HS.state = 0;
        HS.shown = 0;
        break;

    case 0:
        if (HS.pending >= 0) {
            HS.msg = Help_FindIndex(HS.pending);
            HS.pending = -1;
            if (HS.msg >= 0) {
                func_001FE588();
            }
        }
        break;

    case 1: {
        int id;

        force_help_message(5, 0);
        id = D_0015F780[HS.msg].unk_08;
        if (id >= 0 && D_001517D0.unk50 == 0 && D_001517D0.unk1C == -1) {
            D_001517D0.unk1C = id + 0x7530;
        }
        if (D_0013CBE4 & 0x10) {
            RECORD_COUNT();
            RECORD_BEST();
            RECORD_FLAGS();
            HS.timer = 8 - HS.timer;
            HS.state = 7;
        } else if (HS.timer >= 6) {
            HS.timer = 0;
            HS.state = 2;
        }
        break;
    }

    case 2: {
        int id;

        force_help_message(5, 0);
        if (D_0013CBE4 & 0x10) {
            RECORD_COUNT();
            RECORD_BEST();
            HS.state = 7;
            HS.timer = 0;
            RECORD_FLAGS();
        } else if (HS.timer >= func_001F98C0(0x18)) {
            if (D_001517D0.unk5A == 3
                || (id = D_0015F780[HS.msg].unk_08) == -1
                || id != D_001517D0.unk54 - 0x7530) {
                HS.state = 3;
                HS.timer = 0;
            }
        }
        break;
    }

    case 3:
        force_help_message(5, 0);
        if (D_0013CBE4 & 0x10) {
            RECORD_COUNT();
            RECORD_BEST();
            HS.state = 7;
            HS.timer = 0;
            RECORD_FLAGS();
        } else if (HS.timer >= 8) {
            HS.timer = 0;
            HS.state = 4;
        }
        break;

    case 4: {
        int id;

        force_help_message(5, 0);
        if (D_0013CBE4 & 0x10) {
            RECORD_COUNT();
            RECORD_BEST();
            HS.state = 6;
            RECORD_FLAGS();
            HS.timer = 4 - HS.timer;
        } else if (HS.timer >= 4) {
            id = D_0015F780[HS.msg].unk_08;
            if (id != -1 && id == D_001517D0.unk54 - 0x7530 && D_001517D0.unk5A == 3) {
                func_00216960();
            }
            HS.state = 5;
            HS.timer = 0;
        }
        break;
    }

    case 5: {
        int id;

        force_help_message(5, 0);
        if ((HS.timer >= func_001F98C0(0x1A4)
             && ((id = D_0015F780[HS.msg].unk_08) == -1
                 || id != D_001517D0.unk54 - 0x7530
                 || (D_001517D0.unk50 == 0 && D_001517D0.unk1C == -1)))
            || (D_0013CBE4 & 0x10)) {
            RECORD_COUNT();
            RECORD_BEST();
            HS.state = 6;
            HS.timer = 0;
            RECORD_FLAGS();
        }
        break;
    }

    case 6:
        if (D_0015EF1D_b == 0 || HS.timer >= 4 || (D_0013CBE4 & 0x10)) {
            HS.state = 7;
            HS.timer = 0;
        }
        break;

    case 7: {
        int id;

        force_help_message(5, 0);
        id = D_0015F780[HS.msg].unk_08;
        if (id != -1 && id == D_001517D0.unk54 - 0x7530
            && (unsigned short)D_001517D0.unk5A - 6U >= 2) {
            D_001517D0.unk5A = 5;
        }
        if (HS.timer >= 8) {
            if (HS.queued != 0) {
                HS.state = 8;
            } else {
                HS.state = 0;
                HS.msg = -1;
            }
            HS.timer = 0;
        }
        break;
    }
    }
}

extern void func_001F62C8(int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long, long);
extern long func_001F4868_l(int) __asm__("func_001F4868");

/* Help_DrawPrompt: sets the prompt box's +0x18/+0x1C sizes to 0x20,
   draws its frame (func_001F62C8, a 0x40 box around +0x14/+0x10, 0x60
   as the fifth argument), then the prompt icon (effect texture 4) as a
   0x40 quad in grey whose alpha is 0x7E, or x04 * 21 while opening or
   closing (states 1 and 7), capped at 0x80. Every access goes through
   D_001997D0, as in the rest of this file. The texture handle is the
   draw call's 64-bit last argument, taken as `long` so retail's sd
   stores $v0 directly, ahead of the colour. */
void func_001FF0C8(void) {
    int alpha;

    ((HelpState *)D_001997D0)->pad[4] = 0x20;
    ((HelpState *)D_001997D0)->pad[5] = 0x20;
    DrawUIFrame(((HelpState *)D_001997D0)->pad[3] - 0x20,
                  ((HelpState *)D_001997D0)->pad[3] + 0x20,
                  ((HelpState *)D_001997D0)->pad[2] - 0x20,
                  ((HelpState *)D_001997D0)->pad[2] + 0x20, 0x60);
    if (((HelpState *)D_001997D0)->state == 1 || ((HelpState *)D_001997D0)->state == 7) {
        alpha = ((HelpState *)D_001997D0)->x04 * 0x15;
    } else {
        alpha = 0x7E;
    }
    if (alpha > 0x80) {
        alpha = 0x80;
    }
    DrawTexturedQuad(((HelpState *)D_001997D0)->pad[2] - 0x20,
                  ((HelpState *)D_001997D0)->pad[3] - 0x20, 0x40, 0x40, 0, 0, 0x40, 0x40,
                  (alpha << 24) | 0x808080, func_001F4868_l(4));
}

__asm__(".section .text\n\tnop\n");

/* Help_Draw: nothing unless a state is active, the help window is shown
   (+0x30) and one of the fade flags is set. Then, by state, and only while
   D_0015EF1D is set: the growing frame (1), the prompt (2), the frame
   shrinking into the prompt icon with its fading quad (3), the full
   frame and the entry text, fading in (4) or out (6) through the text
   colour's alpha (4, 5, 6), or the closing frame (7). The fade flag is
   re-read in each case, as retail's per-case andi shows. */
void func_001FF1B0(void) {
    if (((HelpState *)D_001997D0)->state == 0) {
        return;
    }
    if (*(int *)(D_001997D0 + 0x30) == 0) {
        return;
    }
    if (D_0015EF1D_b == 0 && D_0015EF1C_b == 0) {
        return;
    }
    switch (((HelpState *)D_001997D0)->state) {
    case 1:
        if (D_0015EF1D_b) {
            int s = ((HelpState *)D_001997D0)->x04 * 4 + 8;

            ((HelpState *)D_001997D0)->pad[4] = s;
            ((HelpState *)D_001997D0)->pad[5] = s;
            DrawUIFrame(((HelpState *)D_001997D0)->pad[3] - s,
                          ((HelpState *)D_001997D0)->pad[3] + s,
                          ((HelpState *)D_001997D0)->pad[2] - s,
                          ((HelpState *)D_001997D0)->pad[2] + s, 0x60);
        }
        break;
    case 2:
        if (D_0015EF1D_b) {
            Help_DrawPrompt();
        }
        break;
    case 3:
        if (D_0015EF1D_b) {
            int a = (((HelpState *)D_001997D0)->pad[0] - 0x20) * ((HelpState *)D_001997D0)->x04 / 8 + 0x20;
            int b = (((HelpState *)D_001997D0)->pad[1] - 0x20) * ((HelpState *)D_001997D0)->x04 / 8 + 0x20;
            int alpha;

            ((HelpState *)D_001997D0)->pad[4] = a;
            ((HelpState *)D_001997D0)->pad[5] = b;
            DrawUIFrame(((HelpState *)D_001997D0)->pad[3] - b,
                          ((HelpState *)D_001997D0)->pad[3] + b,
                          ((HelpState *)D_001997D0)->pad[2] - a,
                          ((HelpState *)D_001997D0)->pad[2] + a, 0x60);
            alpha = (8 - ((HelpState *)D_001997D0)->x04) * 16;
            if (alpha < 0) {
                alpha = 0;
            }
            DrawTexturedQuad(((HelpState *)D_001997D0)->pad[2] - 0x20,
                          ((HelpState *)D_001997D0)->pad[3] - 0x20, 0x40, 0x40, 0, 0, 0x40, 0x40,
                          (alpha << 24) | 0x808080, func_001F4868_l(4));
        }
        break;
    case 4:
    case 5:
    case 6:
        if (D_0015EF1D_b) {
            short win[12];
            int col = 0x80FFA888;
            char *text;

            ((HelpState *)D_001997D0)->pad[4] = ((HelpState *)D_001997D0)->pad[0];
            ((HelpState *)D_001997D0)->pad[5] = ((HelpState *)D_001997D0)->pad[1];
            DrawUIFrame(((HelpState *)D_001997D0)->pad[3] - ((HelpState *)D_001997D0)->pad[1],
                          ((HelpState *)D_001997D0)->pad[3] + ((HelpState *)D_001997D0)->pad[1],
                          ((HelpState *)D_001997D0)->pad[2] - ((HelpState *)D_001997D0)->pad[0],
                          ((HelpState *)D_001997D0)->pad[2] + ((HelpState *)D_001997D0)->pad[0], 0x60);
            if (((HelpState *)D_001997D0)->state == 4) {
                col = 0xFFA888 | (((HelpState *)D_001997D0)->x04 << 29);
            } else if (((HelpState *)D_001997D0)->state == 6) {
                col = ((4 - ((HelpState *)D_001997D0)->x04) << 29) | 0xFFA888;
            }
            text = D_0015F780[((HelpState *)D_001997D0)->pad[6]].text;
            FontSetWindow((void *)win, 0xF0, 0x1E0, 0x2C, 0x1D4, 0x100,
                          ((HelpState *)D_001997D0)->pad[3], 0x10, 3);
            func_001F75D0((void *)win, col, text, -1);
        }
        break;
    case 7:
        if (D_0015EF1D_b) {
            int b = ((HelpState *)D_001997D0)->pad[4]
                  - (((HelpState *)D_001997D0)->pad[4] - 8) * ((HelpState *)D_001997D0)->x04 / 8;
            int a = ((HelpState *)D_001997D0)->pad[5]
                  - (((HelpState *)D_001997D0)->pad[5] - 8) * ((HelpState *)D_001997D0)->x04 / 8;

            DrawUIFrame(((HelpState *)D_001997D0)->pad[3] - a,
                          ((HelpState *)D_001997D0)->pad[3] + a,
                          ((HelpState *)D_001997D0)->pad[2] - b,
                          ((HelpState *)D_001997D0)->pad[2] + b,
                          (8 - ((HelpState *)D_001997D0)->x04) * 12);
        }
        break;
    }
}

extern short D_00199810[];

/* Looks `key` up in field `which` (0 or 1) of D_00199810's 150
   {short a, short b} pairs. On a match it stores the OTHER field into
   *out (when out is not NULL) and returns the pair's index, else -1.
   `off` walks the b field's byte offset and becomes the a field's
   (idx * 4) when which != 0. idx * 4 is computed before the compare,
   so it lives across the branch and fills its delay slot; the address
   is an int sum so the addu takes the offset first. */
int func_001FF4F8(short key, int which, short *out) {
    char *base = (char *)D_00199810;
    short *p = (short *)(base + which * 2);
    int idx = 0;
    int off = 2;

    do {
        int t = idx * 4;
        if (*p == key) {
            if (which != 0) {
                off = t;
            }
            if (out != 0) {
                *out = *(short *)(off + (int)base);
            }
            return idx;
        }
        idx++;
        off += 4;
        p = (short *)((char *)p + 4);
    } while (idx < 150);
    return -1;
}

__asm__(".section .text\n\tnop\n");

extern int func_001FF4F8(short, int, short *);
extern unsigned char D_00141F08[];
extern int D_0015EF30 MACRO_ADDR;

/* Moves the entry func_001FF4F8 finds for `id` (field a of its table)
   to the end of the byte list D_00141F08, whose length is D_0015EF30:
   an earlier occurrence is removed by shifting the rest down, then the
   value is appended. Unknown ids (-1) change nothing. The search loop
   tests the byte before the bound, as retail does: it reads
   D_00141F08[0] before it first reads the count. */
void func_001FF560(short id) {
    int val = func_001FF4F8(id, 0, 0);
    int i;

    if (val == -1) {
        return;
    }
    for (i = 0; D_00141F08[i] != val && i < D_0015EF30; i++) {
    }
    if (i < D_0015EF30) {
        for (; i < D_0015EF30 - 1; i++) {
            D_00141F08[i] = D_00141F08[i + 1];
        }
        D_00141F08[i] = 0;
        D_0015EF30--;
    }
    D_00141F08[D_0015EF30] = val;
    D_0015EF30++;
}

LINKER_REMNANT("asm/remnants/text", func_001FF660);
