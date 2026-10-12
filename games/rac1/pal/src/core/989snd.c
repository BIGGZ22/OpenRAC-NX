#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * 989snd.c (0x12DB68-0x12F308): 989 Studios' sound library. The
 * "/usr/local/989snd/ee/989snd.c" path string is used from 0x12DBE0 on.
 * Name and extent from the NTSC split, shifted +0x140.
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
extern void *D_00154A40 NOT_SDA;
extern int D_00155080[];
extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;
extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;
extern void func_0011AA90(int, int, int, int, int, int, int);
extern void func_0011AA00(void);
extern int D_0012FD08 NOT_SDA;
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;
extern char D_00157E80[];
extern int D_0012FD9C;
extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;
extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;
extern void func_001153FC(void *, int, int);
extern int D_0012FD94;
extern int D_0012FDAC;
extern char D_00158140[];
extern int D_00158180;
extern int D_001581C0;
extern char D_00158528[];
extern int D_0012FDB4;
extern int func_0011CE70(int arg0, int arg1, int arg2, void *arg3);
extern void func_00118EC0(void);
extern void func_0011D4E0(void);
extern void func_00118EB0(void);
extern int D_00130420;
extern int D_00130424;
extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);
extern int func_00120F30(int);
extern void *D_00159840;
extern int D_001313E0;
extern int D_001313E8;
extern int D_001313EC;
extern int D_001313F0;
extern int D_001313E4;
extern int D_001313FC;
extern int func_0011B4C8();
extern int func_00120D28(int);
extern void func_00118C90(int);
extern char D_00132590[];
extern int D_00131440;
extern void func_0011A6C8();
extern int func_0011B6B8(void *);
extern char D_00153000[];
extern char D_00132E08[];
extern int D_001313D0;
extern int func_00121040(int);
extern int D_001325C0;
extern char D_00132E40[];
extern void func_00123650(void *);
extern char D_001534E0[];
extern int D_00132E70[];
extern int func_001238B0(int, int, int, int);
extern int D_00132EA8;
extern int *D_00159B28;
extern int *D_00159B2C;
extern int *D_00159B30;
extern char D_0015B108[];
extern int D_0015B180;
extern void func_00124B60(void *);
extern char D_00153658[];
extern int func_00124920(int);
typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;
extern Ent330 D_0015B640[];
extern void func_00119288(void *a, void *b);
extern void func_001286E8(int, int);
extern int D_00132F70[];
extern void func_0012BCC8(int);
extern void func_00128590(void *);
extern void func_00128968(void *, int);
extern int func_00128860(void *, int);
extern int func_00128A58(void *, int);
extern void func_00129180(void *);
extern char D_00153A80[];
extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");
extern void func_0012AAC8(void *, int);
extern void func_0012C468(void *, void *);
extern char D_00153B38[];
extern void func_0012C0A0(void *);
extern void func_0012BF40(void *);
extern void func_0012C278(void *);
extern void func_00129E30(void *, int, int, int);
extern char D_00153BB8[];
extern char D_00153BD8[];
extern void func_00116248(void *);
extern void func_0012C420(void *);
extern char D_00153C48[];
extern char D_00153C78[];
extern char D_00153C90[];
extern char D_00153CC8[];
extern int func_0012CE48(void *);
extern void func_0012D2A0(void);
extern signed char D_001331D8[];
extern int func_0012D4E0(int);
extern int func_0012D4B0(int);
extern void func_0011DDA0(int);

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012DB68); /* snd_StartSoundSystem */

typedef void (*SndCallback)(int, long);
struct SndCommand {
    SndCallback fn;
    int pad4;
    long arg;
};
/* All SDA ($gp); read through casts like the rest of this file. */
extern short D_0015ED80;   /* pending RPC record */
extern short D_0015ED90;   /* stop callback */
extern short D_0015ED94;   /* stop requested */
extern short D_0015ED98;
extern short D_0015ED9C;   /* abort */
extern short D_0015EDA0;   /* int *count[2] */
extern short D_0015EDB0;   /* struct SndCommand *cmd[2] */
extern short D_0015EDB8;   /* int *reply[2] */
extern short D_0015EDC0;   /* current buffer */
extern short D_0015EDC4;
extern short D_0015EDC8;   /* CD read pending */
extern short D_0015EDD0;   /* CD read callback */
extern short D_0015EDE0;   /* abort callback */
extern short D_0015EDE8;   /* its argument (64-bit) */
/* volatile: retail's compiler keeps its 64-bit accesses out of delay
   slots, so they stay lui-based rather than going through $gp. */
extern volatile long D_0015EDD8_l __asm__("D_0015EDD8") MACRO_ADDR;
extern unsigned int D_0015EE00 MACRO_ADDR;
extern int D_00133204[];
extern int func_0012DFB0(void);
extern void func_0012EB18(void);
extern int func_0012EF48(int);

#define SDA_I(x) (*(int *)&(x))
#define SND_COUNT ((int **)&D_0015EDA0)
#define SND_CMDS ((struct SndCommand **)&D_0015EDB0)
#define SND_REPLY ((int **)&D_0015EDB8)

/* snd_FlushSoundCommands: once the pending RPC completes, run the
   callbacks queued with the other command buffer (or the abort callback);
   finish a CD read (FlushCache, then its callback with the reply word);
   send the next batch when idle; and poll a stop request. Returns
   whether an RPC or a CD read is still in flight. Adapted from Lombyte
   (MIT) for PAL. */
int func_0012DDC0(void) {
    int i;
    int idx;
    SndCallback fn;
    long arg;

    if (SDA_I(D_0015ED80) != 0 && func_0012DFB0() != 0) {
        if (SDA_I(D_0015ED9C) != 0) {
            if ((SndCallback)SDA_I(D_0015EDE0) != 0) {
                ((SndCallback)SDA_I(D_0015EDE0))(D_00133204[0], *(long *)&D_0015EDE8);
            }
            SDA_I(D_0015EDE0) = 0;
            SDA_I(D_0015ED9C) = 0;
        } else {
            idx = SDA_I(D_0015EDC0) != 1;
            for (i = 0; i < *SND_COUNT[idx]; i++) {
                if (SND_CMDS[idx][i].fn != 0) {
                    SND_CMDS[idx][i].fn(SND_REPLY[idx][i + 1], SND_CMDS[idx][i].arg);
                }
            }
        }
    }
    if (SDA_I(D_0015EDC8) != 0) {
        func_00118D80(0);
        if (D_0015EE00 != 0xFFFFFFFF) {
            if ((SndCallback)SDA_I(D_0015EDD0) != 0) {
                arg = D_0015EDD8_l;
                fn = (SndCallback)SDA_I(D_0015EDD0);
                SDA_I(D_0015EDD0) = 0;
                D_0015EDD8_l = 0;
                fn(D_0015EE00, arg);
            }
            D_0015EE00 = 0;
            SDA_I(D_0015EDC8) = 0;
        }
    }
    if (SDA_I(D_0015ED80) == 0) {
        if (*SND_COUNT[SDA_I(D_0015EDC0)] != 0 && SDA_I(D_0015EDC4) == 0) {
            func_0012EB18();
        }
    }
    if (SDA_I(D_0015ED94) != 0) {
        func_0012EF48(1);
        if (SDA_I(D_0015ED98) != 0) {
            SDA_I(D_0015ED94) = 0;
            SDA_I(D_0015ED98) = 0;
            if ((void (*)(int))SDA_I(D_0015ED90) != 0) {
                ((void (*)(int))SDA_I(D_0015ED90))(1);
            }
        }
    }
    return SDA_I(D_0015ED80) != 0 || SDA_I(D_0015EDC8) != 0;
}

#undef SDA_I
#undef SND_COUNT
#undef SND_CMDS
#undef SND_REPLY

LINKER_REMNANT("asm/remnants/core_text", func_0012DFA0);

extern void func_00118D80(int arg0);
extern int D_0015ECC0;
extern int func_00116078(void *arg0);
extern char D_00153D98[];
extern int func_0011B6B8(void *arg0);
extern short D_0015ED80;
extern short D_0015ED84;

/* snd_GotReturns: true once the pending IOP reply record's sentinel word
 * and its per-slot echo both read back -1, after taking the returns
 * semaphore (D_0015ECC0). D_0015ED80 is a pointer to that record, not a
 * plain flag; D_0015ED84 is the slot index within it. The slot address
 * is left as one inlined expression (no separate idx/slot locals) --
 * that is what gets retail's addu operand order and epilogue merge. */
int func_0012DFB0(void) {
    int *p;

    func_00118D80(0);
    if (*(int *)&D_0015ED80 == 0) {
        return 1;
    }
    if (func_0011B6B8(&D_0015ECC0) != 0) {
        return 0;
    }
    p = *(int **)&D_0015ED80;
    if ((unsigned int)p[0] != 0xFFFFFFFFU) {
        func_00116078(D_00153D98);
        return 0;
    }
    if ((unsigned int)*(int *)((*(int *)&D_0015ED84) * 4 + (char *)p + 4) !=
        (unsigned int)p[0]) {
        func_00116078(D_00153D98);
        return 0;
    }
    *(int *)&D_0015ED80 = 0;
    return 1;
}


/* snd_PrepareReturnBuffer */
void func_0012E038(void *arg0, int arg1) {
    int *p = (int *)arg0;
    *(int *)&D_0015ED84 = arg1;
    *(int *)&D_0015ED80 = (int)arg0;
    p[arg1 + 1] = 0;
    p[0] = 0;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E058);

extern short D_0015EDC8;
extern short D_0015ED88;
extern int D_00137C40_a[] __asm__("D_00137C40");
extern int D_0015ECE8;
extern char D_00153EA8[];
extern char D_00153ED8[];
extern char D_00153EF8[];
extern char D_00153E20[];
extern int func_0012DDC0(void);

/* IOP command 3 with two words, waiting for the reply word in
   D_0015EE00; 0 (and a message) when a stream is pending, the CD is
   busy or the RPC fails (error 0x106 in D_0015ED88). */
unsigned int func_0012E060(int arg0, int arg1) {
    *(int *)&D_0015ED88 = 0;
    if (*(int *)&D_0015EDC8 != 0) {
        func_00116078(D_00153EA8);
        return 0;
    }
    if (func_0012EF48(1) == 1) {
        func_00116078(D_00153ED8);
        return 0;
    }
    D_00137C40_a[1] = arg1;
    D_0015EE00 = 0xFFFFFFFF;
    D_00137C40_a[0] = arg0;
    while (func_0011B6B8(&D_0015ECE8) != 0) {
        func_00116078(D_00153E20);
        func_0012DDC0();
        func_00118D80(0);
    }
    if (func_0011B4C8(&D_0015ECE8, 3, 1, D_00137C40_a, 8, &D_0015EE00, 4, 0, 0) < 0) {
        func_00116078(D_00153EF8);
        *(int *)&D_0015ED88 = 0x106;
        return 0;
    }
    if (D_0015EE00 == 0xFFFFFFFF) {
        do {
            func_00118D80(0);
        } while (D_0015EE00 == 0xFFFFFFFF);
    }
    return D_0015EE00;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E1B8);

extern short D_0015EDD0;
extern long D_0015EDD8 MACRO_ADDR;
extern int D_00137C40;
extern char D_00153FC0[];
extern char D_00153FF8[];

/* Start an IOP stream (command 0x57) for arg0, unless one is already
   pending (D_0015EDC8) or the CD is busy (func_0012EF48); the reply word
   lands in D_0015EE00. */
void func_0012E1C8(int arg0, int arg1, long arg2) {
    *(int *)&D_0015ED88 = 0;
    if (*(int *)&D_0015EDC8 != 0) {
        func_00116078(D_00153FC0);
        return;
    }
    if (func_0012EF48(1) == 1) {
        func_00116078(D_00153FF8);
        return;
    }
    D_00137C40 = arg0;
    D_0015EE00 = 0xFFFFFFFF;
    *(int *)&D_0015EDD0 = arg1;
    D_0015EDD8 = arg2;
    while (func_0011B6B8(&D_0015ECE8) != 0) {
        func_00116078(D_00153E20);
        func_0012DDC0();
        func_00118D80(0);
    }
    *(int *)&D_0015EDC8 = 1;
    func_0011B4C8(&D_0015ECE8, 0x57, 1, &D_00137C40, 4, &D_0015EE00, 4, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E2D8);

extern int func_0012E820(int, int, void *, int, int);

/* Command 8 with no data. */
void func_0012E2E8(void) {
    func_0012E820(8, 0, 0, 0, 0);
}

extern int func_0012E820(int, int, void *, int, int);

/* snd_UnloadBank */
void func_0012E318(int arg0) {
    int local = arg0;
    func_0012E820(6, 4, &local, 0, 0);
}

/* snd_SetMasterVolume */
void func_0012E348(int arg0, int arg1) {
    int local[2];
    local[0] = arg0;
    local[1] = arg1;
    func_0012E820(9, 8, local, 0, 0);
}

/* snd_SetPlaybackMode */
void func_0012E380(int arg0) {
    int local = arg0;
    func_0012E820(0xB, 4, &local, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E3B0);

/* Command 0xD with two ints. */
void func_0012E3C0(int arg0, int arg1) {
    int local[2];
    local[0] = arg0;
    local[1] = arg1;
    func_0012E820(0xD, 8, local, 0, 0);
}

/* snd_SetGroupVoiceRange */
void func_0012E3F8(int arg0, int arg1, int arg2) {
    int local[3];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    func_0012E820(0x4E, 0xC, local, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E438);

/* Command 0x11 with six ints; the last two arguments are passed through. */
void func_0012E448(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
                   int arg6, int arg7) {
    int local[6];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    local[4] = arg4;
    local[5] = arg5;
    func_0012E820(0x11, 0x18, local, arg6, arg7);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E490);

/* Command 0x15 with one int. */
void func_0012E4A8(int arg0) {
    int local = arg0;
    func_0012E820(0x15, 4, &local, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E4D8);

/* Command 0x18 with no data. */
void func_0012E4F8(void) {
    func_0012E820(0x18, 0, 0, 0, 0);
}

/* snd_PauseAllSoundsInGroup */
void func_0012E528(int arg0) {
    int local = arg0;
    func_0012E820(0x16, 4, &local, 0, 0);
}

/* snd_ContinueAllSoundsInGroup */
void func_0012E558(int arg0) {
    int local = arg0;
    func_0012E820(0x17, 4, &local, 0, 0);
}

/* snd_SoundIsStillPlaying_CB */
void func_0012E588(int arg0, int arg1, int arg2) {
    int local = arg0;
    func_0012E820(0x19, 4, &local, arg1, arg2);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E5B8);

/* Command 0x21 with six ints; the last two arguments are passed through. */
void func_0012E600(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
                   int arg6, int arg7) {
    int local[6];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    local[4] = arg4;
    local[5] = arg5;
    func_0012E820(0x21, 0x18, local, arg6, arg7);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012E648);

extern unsigned char D_00133240_c[] __asm__("D_00133240");
extern int D_00133200_w[] __asm__("D_00133200");
extern int func_0012E038_s(int *, int) __asm__("func_0012E038");

/* snd_SendIOPCommandAndWait: copy the command data into the send buffer,
   flush pending commands, wait for the RPC server, call command cmd and
   wait for its reply; the reply's second word is the result. Adapted
   from Lombyte (MIT) for PAL. */
int func_0012E688(int cmd, int size, void *arg) {
    unsigned char *data = arg;
    int i;
    int result;

    for (i = 0; i < size; i++) {
        D_00133240_c[i] = data[i];
    }
    goto check;
    do {
        func_0012DDC0();
        func_00118D80(0);
check:;
    } while (*(int *)&D_0015ED80 != 0);
    func_0012E038_s(D_00133200_w, 1);
    while (func_0011B6B8(&D_0015ECC0) != 0) {
        func_00116078(D_00153E20);
        func_0012DDC0();
        func_00118D80(0);
    }
    if (size != 0) {
        func_0011B4C8(&D_0015ECC0, cmd, 1, D_00133240_c, size, D_00133200_w, 0xC, 0, 0);
    } else {
        func_0011B4C8(&D_0015ECC0, cmd, 1, 0, 0, D_00133200_w, 0xC, 0, 0);
    }
    while (func_0012DFB0() == 0) {
    }
    result = D_00133200_w[1];
    if (*((int **)&D_0015EDA0)[*(int *)&D_0015EDC0] != 0 && *(int *)&D_0015EDC4 == 0) {
        func_0012EB18();
    }
    return result;
}

typedef struct {
    int func;
    int pad;
    long data;
} SndCb_2E820;

extern short D_0015EDC0;
extern short D_0015EDA0;
extern short D_0015EDA8;
extern short D_0015EDB0;
extern char D_00154110[];
extern char D_00154170[];
extern void func_0012EAE0(void);
extern int func_00116078_p(const char *, ...) __asm__("func_00116078");

/* snd_SendIOPCommandNoWait: sends a bare command at once when nothing is queued or pending; otherwise waits
   for room in the current batch, appends the command and its data, and records the callback for its reply.
   Adapted from Lombyte (MIT) for PAL: src/audio/rpc/snd_send_iop_command_no_wait.c, snd_send_iop_command_no_wait. */
void func_0012E820_r(int cmd, int size, unsigned char *data, int cb, long cb_data) __asm__("func_0012E820");
/* snd_SendIOPCommandNoWait */
void func_0012E820_r(int cmd, int size, unsigned char *data, int cb, long cb_data) {
    int need;
    int tries;
    int held;
    int i;
    int slot;
    unsigned char *p;

    held = 0;
    if (*(int *)&D_0015EDC4 == 0 && *(int *)&D_0015ED80 == 0 && size == 0 && cb == 0) {
        func_0012E038_s(D_00133200_w, 1);
        while (func_0011B6B8(&D_0015ECC0) != 0) {
            func_00116078_p(D_00153E20);
            func_0012DDC0();
            func_00118D80(0);
        }
        func_0011B4C8(&D_0015ECC0, cmd, 1, 0, 0, D_00133200_w, 0xC, 0, 0);
        return;
    }
    need = size + 4;
    tries = 0;
    if (need % 4 != 0) {
        need += 4 - need % 4;
    }
    while (*((int **)&D_0015EDA0)[*(int *)&D_0015EDC0] == 0x100 ||
           ((int *)&D_0015EDA8)[*(int *)&D_0015EDC0] < need) {
        if (*(int *)&D_0015EDC4 != 0) {
            *(int *)&D_0015EDC4 = 0;
            held = 1;
        }
        func_0012DDC0();
        if (tries == 1) {
            func_00116078_p(D_00154110, *(int *)&D_0015EDC0, *((int **)&D_0015EDA0)[*(int *)&D_0015EDC0]);
        }
        tries++;
    }
    if (tries != 0) {
        func_00116078_p(D_00154170, tries);
    }
    if (held != 0) {
        *(int *)&D_0015EDC4 = 1;
    }
    p = (unsigned char *)((int **)&D_0015EDA0)[*(int *)&D_0015EDC0] -
        (((int *)&D_0015EDA8)[*(int *)&D_0015EDC0] - 0x1000);
    *(short *)p = cmd;
    p += 2;
    *(short *)p = size;
    p += 2;
    for (i = 0; i < size; i++) {
        p[i] = data[i];
    }
    ((int *)&D_0015EDA8)[*(int *)&D_0015EDC0] -= need;
    slot = *(int *)&D_0015EDC0;
    ((SndCb_2E820 **)&D_0015EDB0)[slot][*((int **)&D_0015EDA0)[slot]].func = cb;
    ((SndCb_2E820 **)&D_0015EDB0)[slot][*((int **)&D_0015EDA0)[slot]].data = cb_data;
    func_0012EAE0();
}

/* gp-relative, no retail symbol: gp 0x166D00 - 0x7F40 = 0x15EDC0
   (message-queue index) and gp 0x166D00 - 0x7F60 = 0x15EDA0 (array of
   pointers to the per-queue message-count int). */

/* snd_PostMessage */
void func_0012EAE0(void) {
    int idx = *(int *)&D_0015EDC0;
    int **base = (int **)&D_0015EDA0;
    int *msg = base[idx];

    *msg = *msg + 1;
    func_0012DDC0();
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012EB18); /* snd_SendCurrentBatch */

/* gp-relative: declared small so -G2 places it in the small-data area,
   accessed as the word it really is. gp 0x166D00 - 0x7F3C = 0x15EDC4. */

/* snd_UnkFunction_0012eaf0 */
void func_0012EC30(void) {
    *(int *)&D_0015EDC4 = 1;
}


/*
 * Close, not exact (2/32), same size. Retail saves/restores $ra with
 * sq/lq here; v1.36 (correctly for core_text overall) emits sd/ld, so
 * the two spill instructions differ and nothing else does. This is one
 * of the ~14 core_text functions on the sq side of that split -- the
 * still-open half of the sq/lq question, not a source-shape problem.
 */
/* snd_UnkFunction_0012eb00 */
void func_0012EC40(void) {
    *(int *)&D_0015EDC4 = 0;
    func_0012DDC0();
}

/* gp-relative, no retail symbol: gp 0x166D00 - 0x7F74 = 0x15ED8C
   (streaming-enabled flag, same as func_0012F030/func_0012F068's) and
   gp 0x166D00 - 0x7F38 = 0x15EDC8 (drain-commands-first flag). */
extern short D_0015ED8C;
extern short D_0015EDBC;   /* the word at +0xC, SDA ($gp -0x7F38) */
extern int func_0012EF48(int);              /* snd_StreamSafeCdSync */
extern int func_0012E688(int, int, void *); /* snd_SendIOPCommandAndWait */

/* snd_InitVAGStreamingEx: no-op (returns 0) if VAG streaming is already
 * enabled; otherwise drains any pending sound commands, syncs the CD
 * stream, and sends the "init streaming" IOP command with the four
 * arguments packed into a header, storing (and returning) its result as
 * the streaming-enabled flag. */
int func_0012EC60(int arg0, int arg1, int arg2, int arg3) {
    int hdr[4];
    int r;

    if (*(int *)&D_0015ED8C == 1) {
        return 0;
    }
    if (*(int *)((char *)&D_0015EDBC + 0xC) != 0) {
        while (func_0012DDC0() != 0) {
        }
    }
    func_0012EF48(0);
    hdr[0] = arg0;
    hdr[1] = arg1;
    hdr[2] = arg2;
    hdr[3] = arg3;
    r = func_0012E688(0x2A, 0x10, hdr);
    *(int *)&D_0015ED8C = r;
    return r;
}

/* snd_StopAllStreams */
void func_0012ED10(void) {
    func_0012E820(0x34, 0, 0, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012ED40);

extern int func_0012E820_l(int, int, void *, int, long) __asm__("func_0012E820");

/* Command 0x2C: seven words, the middle two each packing two 16-bit
   values; the last two arguments are passed through. */
void func_0012ED48(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
                   int arg6, int arg7, int arg8, int arg9, long arg10) {
    int local[7];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = (arg4 << 16) | (arg2 & 0xFFFF);
    local[3] = (arg5 << 16) | (arg3 & 0xFFFF);
    local[4] = arg6;
    local[5] = arg7;
    local[6] = arg8;
    func_0012E820_l(0x2C, 0x1C, local, arg9, arg10);
}

/* snd_PauseVAGStream */
void func_0012EDB0(int arg0) {
    int local = arg0;
    func_0012E820(0x2D, 4, &local, 0, 0);
}

/* snd_ContinueVAGStream */
void func_0012EDE0(int arg0) {
    int local = arg0;
    func_0012E820(0x2E, 4, &local, 0, 0);
}

/* snd_GetVAGStreamTimeRemaining_CB */
void func_0012EE10(int arg0, int arg1, int arg2) {
    int local = arg0;
    func_0012E820(0x32, 4, &local, arg1, arg2);
}

/* snd_IsVAGStreamBuffered_CB */
void func_0012EE40(int arg0, int arg1, int arg2) {
    int local = arg0;
    func_0012E820(0x4F, 4, &local, arg1, arg2);
}

extern int func_0012E688(int, int, void *);

/* snd_StreamSafeCheckCDIdle */
void func_0012EE70(int arg0) {
    int local = arg0;
    func_0012E688(0x36, 4, &local);
}

/* gp-relative, no retail symbol: gp 0x166D00 - 0x7F74 = 0x15ED8C
   (streaming-enabled flag, see func_0012F030/func_0012F068). */
extern short D_0015ED94;

typedef struct {
    volatile unsigned int count;
    int unk04[3];
    volatile int error;
} CdSafeState;
extern CdSafeState D_00137C00;

extern int func_00121750(int, int, int);

/* snd_StreamSafeCdRead */
int func_0012EE98(int arg0, int arg1, int arg2) {
    int local[3];

    if (*(int *)&D_0015ED8C == 0) {
        return func_00121750(arg0, arg1, arg2);
    }

    if (func_0012EF48(1) == 1) {
        return 0;
    }

    local[0] = arg0;
    D_00137C00.count = 1;
    D_00137C00.error = 0;
    local[1] = arg1;
    local[2] = arg2;
    func_0012E820(0x38, 12, local, 0, 0);
    *(int *)&D_0015ED94 = 1;
    *(int *)&D_0015ED98 = 0;
    return 1;
}

extern void func_00118D80(int);

/* snd_StreamSafeCdSync */
int func_0012EF48(int arg0) {
    if (*(int *)&D_0015ED8C == 0) {
        return func_00120F30(arg0);
    }

    func_00118D80(0);
    *(int *)&D_0015ED98 = D_00137C00.count == 0;
    if (*(int *)&D_0015ED98 == 1) {
        return 0;
    }

    if (arg0 == 1) {
        return 1;
    }

    while (*(int *)&D_0015ED98 == 0) {
        func_0012DDC0();
        func_00118D80(0);
        *(int *)&D_0015ED98 = D_00137C00.count == 0;
    }

    return 0;
}

extern int func_001219C8(void);

/* snd_StreamSafeCdBreak */
int func_0012EFE8(void) {
    if (*(int *)&D_0015ED8C == 0) {
        return func_001219C8();
    }
    func_0012E820(0x37, 0, 0, 0, 0);
    return 1;
}

extern int func_00121930(void);

/* snd_StreamSafeCdGetError: returns the last CD error while VAG streaming
 * owns the drive (the cached value at D_00137C00.error), otherwise defers
 * to sceCdGetError. The field access (not a byte-offset cast) plus
 * volatile is what makes retail's separate lui+addiu base and its
 * duplicated epilogue restore come out of this compiler. */
int func_0012F030(void) {
    if (*(int *)&D_0015ED8C == 0) {
        return func_00121930();
    }
    return D_00137C00.error;
}

/* gp-relative, no retail symbol: gp 0x166D00 - 0x7F74 = 0x15ED8C
   (streaming-enabled flag, same as func_0012F030's) and
   gp 0x166D00 - 0x7F70 = 0x15ED90 (cached callback value). */
extern short D_0015ED90;
extern int func_00120978(void);

/* snd_StreamSafeCdCallback */
int func_0012F068(int arg0) {
    int old;

    if (*(int *)&D_0015ED8C == 0) {
        return func_00120978();
    }
    old = *(int *)&D_0015ED90;
    *(int *)&D_0015ED90 = arg0;
    return old;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012F098);

/* Command 0x50 with five ints. */
void func_0012F0A8(int arg0, int arg1, int arg2, int arg3, int arg4) {
    int local[5];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    local[4] = arg4;
    func_0012E820(0x50, 0x14, local, 0, 0);
}

/* snd_PreAllocReverbWorkArea */
void func_0012F0E8(int arg0, int arg1) {
    int local[2];
    local[0] = arg0;
    local[1] = arg1;
    func_0012E820(0x51, 8, local, 0, 0);
}

/* snd_AutoReverb */
void func_0012F120(int arg0, int arg1, int arg2, int arg3) {
    int local[4];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    func_0012E820(0x10, 0x10, local, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012F160);

/* Command 0x3B with six ints, waiting for the reply. */
int func_0012F1A8(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    int local[6];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    local[4] = arg4;
    local[5] = arg5;
    return func_0012E688(0x3B, 0x18, local);
}

/* snd_ResetMovieSound */
void func_0012F1E8(void) {
    func_0012E688(0x3D, 0, 0);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012F210);

/* Command 0x3C with no data, waiting for the reply. */
int func_0012F220(void) {
    return func_0012E688(0x3C, 0, 0);
}

/* snd_StartMovieSound */
void func_0012F248(int arg0, int arg1, int arg2, int arg3, int arg4) {
    int local[5];
    local[0] = arg0;
    local[1] = arg1;
    local[2] = arg2;
    local[3] = arg3;
    local[4] = arg4;
    func_0012E688(0x3E, 0x14, local);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012F280);

/* Command 0x5A with two ints, waiting for the reply. */
int func_0012F288(int arg0, int arg1) {
    int local[2];
    local[0] = arg0;
    local[1] = arg1;
    return func_0012E688(0x5A, 8, local);
}

/* snd_GetMovieNAX */
void func_0012F2B8(void) {
    func_0012E688(0x5B, 0, 0);
}

/* snd_GetDopplerPitchMod */
int func_0012F2E0(int arg0) {
    return (arg0 * 0x5F4) / 0x2E5;
}
