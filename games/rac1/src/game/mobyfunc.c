#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * mobyfunc.cpp in the original source; text 0x20D348-0x20E6B8.
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

/* A moby instance: 0x100 bytes, in one array from D_0016001C to
   D_00160020. state 0xFE is a free slot and 0xFF marks the free tail
   (taking that slot moves the mark to the next one); a freed slot is not
   reused before frame unk38 (DeleteMoby sets it two frames ahead). */
typedef struct {
    char _pad00[0x11];
    unsigned char unk11; /* 0x11: copied to a moby's +0x7C */
    unsigned char unk12; /* 0x12: copied to a moby's +0x7E */
    char _pad13[0x1C - 0x13];
    int frames[1]; /* 0x1C */
} MobySeq;
typedef struct {
    char _pad00[0x48];
    MobySeq *seqs[1]; /* 0x48: animation sequences */
} MobyClass;
typedef struct {
    char _pad00[0x20];
    unsigned char state; /* 0x20 */
    char _pad21[0x24 - 0x21];
    MobyClass *pClass; /* 0x24 */
    char _pad28[0x38 - 0x28];
    unsigned long unk38; /* 0x38 */
    char _pad40[0x50 - 0x40];
    unsigned char frame;     /* 0x50 */
    unsigned char prevFrame; /* 0x51 */
    unsigned char seq;       /* 0x52: 0xFF for none */
    unsigned char prevSeq;   /* 0x53 */
    char _pad54[0x68 - 0x54];
    int frameData;     /* 0x68 */
    int prevFrameData; /* 0x6C */
    char _pad70[0x78 - 0x70];
    char *pvars; /* 0x78: this moby's 0x80 bytes at D_00160028 */
    unsigned char unk7C; /* 0x7C: the sound it wants (func_0020D790) */
    unsigned char unk7D; /* 0x7D: the handle of the one playing, or 0xFF */
    unsigned char unk7E; /* 0x7E */
    char _pad7F[0x100 - 0x7F];
} Moby;
extern char *D_0016001C MACRO_ADDR;
extern char *D_00160020 MACRO_ADDR;
extern char *D_00160028 MACRO_ADDR;
extern int D_0015F6F0 MACRO_ADDR;
extern int D_0015FFFC MACRO_ADDR;
extern char D_001E86F0[];
extern void func_0020D440(void *, int);

/* CreateMoby. The failure message ("... Time: %d, oClass: %d") takes
   oClass as its third argument, which is why retail keeps it in $a2. */
Moby *CreateMoby(int oClass) {
    Moby *m;

    for (m = (Moby *)D_0016001C; m < (Moby *)D_00160020; m++) {
        if (m->state >= 0xFE && (unsigned int)D_0015F6F0 >= m->unk38) {
            if (m->state == 0xFF) {
                m[1].state = 0xFF;
            }
            InitMobyInstance(m, oClass);
            m->pvars = D_00160028 + (m - (Moby *)D_0016001C) * 0x80;
            FastMemSet(m->pvars, 0, 0x80);
            if (D_0015FFFC != 0) {
                D_0015FFFC--;
            }
            return m;
        }
    }
    STUB_printf(D_001E86F0, D_0015F6F0, oClass);
    return 0;
}

extern unsigned char D_001B3E40[] NOT_SDA;
extern int D_001B3900[];
extern void *D_001B3580[];
extern int D_00160018 MACRO_ADDR;
extern void func_0020D6D0_p(void *) __asm__("func_0020D6D0");

typedef struct {
    char _pad00[0x10];
    unsigned char nframes; /* 0x10 */
    signed char unk11;     /* 0x11 */
} MobyISeq;

typedef struct {
    char _pad00[6];
    unsigned char unk06; /* 0x06 */
    char _pad07[5];
    unsigned char unk0C; /* 0x0C */
    char _pad0D;
    unsigned char unk0E; /* 0x0E */
    unsigned char unk0F; /* 0x0F */
    int unk10;           /* 0x10 */
    char _pad14[0x10];
    float unk24;         /* 0x24 */
    char _pad28[0x18];
    int unk40;           /* 0x40 */
    unsigned short unk44; /* 0x44 */
    char _pad46[2];
    MobyISeq *seq;       /* 0x48 */
} MobyIClass;

typedef struct {
    char _pad00[0x21];
    unsigned char unk21;   /* 0x21 */
    unsigned char oClass;  /* 0x22 */
    unsigned char unk23;   /* 0x23 */
    MobyIClass *pClass;    /* 0x24 */
    char _pad28[4];
    float unk2C;           /* 0x2C */
    char _pad30[4];
    unsigned short flags;  /* 0x34 */
    unsigned short unk36;  /* 0x36 */
    unsigned long unk38;   /* 0x38 */
    char _pad40[0x18];
    float unk58;           /* 0x58 */
    float unk5C;           /* 0x5C */
    char _pad60[0x11];
    unsigned char unk71;   /* 0x71 */
    unsigned char unk72;   /* 0x72 */
    unsigned char unk73;   /* 0x73 */
    int unk74;             /* 0x74 */
    char _pad78[4];
    unsigned char unk7C;   /* 0x7C */
    unsigned char unk7D;   /* 0x7D */
    unsigned char unk7E;   /* 0x7E */
    unsigned char unk7F;   /* 0x7F */
    char _pad80[4];
    int unk84;             /* 0x84 */
    int unk88;             /* 0x88 */
    char _pad8C[4];
    int unk90;             /* 0x90 */
    int unk94;             /* 0x94 */
    char _pad98[8];
    unsigned char unkA0;   /* 0xA0 */
    unsigned char unkA1;   /* 0xA1 */
    unsigned char unkA2;   /* 0xA2 */
    unsigned char unkA3;   /* 0xA3 */
    unsigned char unkA4;   /* 0xA4 */
    char _padA5;
    short unkA6;           /* 0xA6 */
    int unkA8;             /* 0xA8 */
    int unkAC;             /* 0xAC */
    char _padB0[0xD];
    unsigned char unkBD;   /* 0xBD */
    char _padBE[0x42];
} MobyI;

/* InitMobyInstance: clears the 0x100-byte moby, fills its defaults (class
   byte from D_001B3E40[oClass], colours, its slot index from the moby
   array base D_00160018), flags a class with no D_001B3900 entry, then
   copies the class record D_001B3580[class] (or marks the moby dead when
   there is none) and applies the animation-sequence rules after
   func_0020D6D0. The moby is a real struct so its non-byte stores are "in
   struct" and D_00160018's load can move above them; the class load comes
   first, and the default stores are ordered so that sched1's
   register-pressure tie-break (stores that free a register go first)
   reproduces retail's store order. */
void InitMobyInstance(void *arg0, int oClass) {
    MobyI *m = (MobyI *)arg0;
    unsigned char c;
    int idx;
    MobyIClass *pClass;

    FastMemSet(m, 0, 0x100);
    c = D_001B3E40[oClass];
    m->unk23 = 0x80;
    m->oClass = c;
    m->unkA4 = 0xFF;
    m->unk21 = 0xFF;
    m->unk71 = 0xFF;
    m->unk72 = 0xFF;
    m->unkA6 = oClass;
    m->unk38 = 0x40404000000000L;
    m->unk36 = 0x7F80;
    idx = ((char *)m - (char *)D_00160018) >> 8;
    m->unkA8 = idx << 16;
    m->unkAC = idx;
    m->unk7E = 0;
    m->unk7C = 0xFF;
    m->unkA0 = 0x7F;
    m->unkA2 = 0x80;
    m->unk7D = 0xFF;
    m->unkA1 = 0x7F;
    m->unkA3 = 0x80;
    m->unk74 = D_001B3900[m->oClass];
    if (m->unk74 == 0) {
        m->flags |= 2;
    }
    pClass = (MobyIClass *)D_001B3580[m->oClass];
    if (pClass != 0) {
        MobyIClass *p;

        m->pClass = pClass;
        m->unk72 = pClass->unk0E;
        m->flags |= pClass->unk44;
        m->unk94 = pClass->unk10;
        m->unk2C = pClass->unk24;
        m->unk58 = 1.0f;
        m->unk5C = 1.0f;
        if (pClass->unk40 != 0) {
            m->flags |= 0x10;
            m->unk90 = pClass->unk40;
        }
        if (m->pClass->unk0F != 0) {
            m->unk7F = 0x18;
            m->flags |= 0x400;
            m->unk84 = 0;
            m->unk88 = 0;
            m->unkBD = 0;
        }
        if (m->pClass->unk06 != 0) {
            m->unk73 = 0x18;
        }
        if (m->pClass->seq == 0) {
            return;
        }
        func_0020D6D0_p(m);
        if (m->pClass->seq->nframes >= 2) {
            m->flags &= 0xFFFD;
        }
        p = m->pClass;
        if (p->unk0C == 1) {
            if (p->seq->nframes < 2) {
                m->unk58 = 0.0f;
                if (p->seq->unk11 < 0) {
                    m->flags |= 0x40;
                }
            }
        }
        return;
    }
    m->pClass = 0;
    m->flags |= 5;
    m->unk94 = 0;
}

typedef struct {
    char _pad00[0x20];
    unsigned char state; /* 0x20 */
    char _pad21[0x38 - 0x21];
    long unk38; /* 0x38 */
} MobyDel;
extern void func_0020EA70(void *, int);

/* DeleteMoby */
void DeleteMoby(MobyDel *m) {
    if ((char *)m < D_0016001C) {
        m->state = 0xFD;
    } else {
        m->state = 0xFE;
    }
    m->unk38 = D_0015F6F0 + 2;
    func_0020EA70(m, 0x80807F7F);
}

extern unsigned char D_001AAF40[];

/* Refresh a moby's animation frame pointers from its class's sequence
   table: seq 0xFF (none) points frameData into D_001AAF40 instead.
   Reaching the table as mc->seqs[i] (an array member behind a
   class-pointer local) is load-bearing: CSE shares mc + 0x48 across the
   three uses, the adds come out base-first, and local-alloc ties the
   last one to that base, which puts it in $a0 and overwrites the
   compare's copy of m->seq. With the value gone, reload_cse cannot turn
   the three index loads into `andi`s of the compare register. */
void update_moby_animation_state(Moby *m) {
    MobyClass *mc;
    if (m->seq != 0xFF) {
        mc = m->pClass;
        m->frameData = mc->seqs[m->seq]->frames[m->frame];
        m->unk7E = mc->seqs[m->seq]->unk12;
        m->unk7C = mc->seqs[m->seq]->unk11;
    } else {
        m->unk7C = m->seq;
        m->unk7E = 0;
        m->frameData = (int)&D_001AAF40[m->frame << 11];
    }
    m->prevFrameData = m->pClass->seqs[m->prevSeq]->frames[m->prevFrame];
}

extern void func_0022EAB0(int);
extern int func_0022ED80(int, int, int);

/* Drop this moby's sound handle (byte 0x7D) when its sound slot no longer
   belongs to it or plays another sound than the wanted one (byte 0x7C);
   with no handle, start the wanted one. Re-reading s[0x7D] inside the
   branch is what gives retail's two registers: the compare keeps the
   first load, the copy (live across blocks) becomes CSE's canonical
   register for the index and the call. */
void MobySyncVoice(unsigned char *s) {
    if (s[0x7D] != 0xFF) {
        int id = s[0x7D];
        char *e = D_0013E650 + id * 0x70;
        if (*(unsigned char **)(e + 0x88) != s) {
            s[0x7D] = 0xFF;
        } else if (*(short *)(e + 0x7E) != s[0x7C]) {
            sound_KillChannel(id);
            s[0x7D] = 0xFF;
        }
    } else if (s[0x7C] != 0xFF) {
        s[0x7D] = func_0022ED80(s[0x7C], 4, (int)s);
    }
}

LINKER_REMNANT("asm/remnants/text", func_0020D828);

extern float func_001FA888(int);

/* Animation-frame value of a moby (the short at +4 of its frame data,
   1/16 units): the current frame's (the previous one's when no sequence
   is set), plus the blend factor at +0x54 while a new sequence or frame
   is starting, else blended between the current and previous frames by
   that factor. Written `frame > prevFrame` so the two bytes load in
   retail's order; prevFrameData is read before the first call. */
float MobyGetFrameValue(Moby *m) {
    int fd;

    if (m->seq != 0xFF) {
        fd = m->frameData;
    } else {
        fd = m->prevFrameData;
    }
    if (*(float *)((char *)m + 0x54) == 0.0f) {
        return IntToFloat(*(short *)(fd + 4)) * 0.0625f;
    }
    if (m->seq != m->prevSeq || m->frame > m->prevFrame) {
        return IntToFloat(*(short *)(fd + 4)) * 0.0625f + *(float *)((char *)m + 0x54);
    } else {
        int prev = m->prevFrameData;
        float a = IntToFloat(*(short *)(fd + 4)) * (1.0f - *(float *)((char *)m + 0x54));
        return (a + IntToFloat(*(short *)(prev + 4)) * *(float *)((char *)m + 0x54)) * 0.0625f;
    }
}
__asm__(".section .text\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_0020D928);

/* Attach a fresh node to arg0's list at +0x64, seeded with 1.0f scales. */
/* AttachManipulator */
void AttachManipulator(char *arg0, int arg1, unsigned char *arg2) {
    unsigned char *e;

    if (arg2[1] != 0) {
        return;
    }
    arg2[0] = (char)arg1;
    arg2[1] = 1;
    *(float *)(arg2 + 0x1C) = 1.0f;
    *(float *)(arg2 + 0x20) = 1.0f;
    *(float *)(arg2 + 0x24) = 1.0f;
    *(float *)(arg2 + 0x28) = 1.0f;

    e = *(unsigned char **)(*(char **)(*(char **)(arg0 + 0x24) + 0x1C) + arg2[0] * 4 + 4);
    *(int *)(arg2 + 4) = e[*e + 4] * 0x40 + 0x70000000;

    *(int *)(arg2 + 8) = *(int *)(arg0 + 0x64);
    *(int *)(arg0 + 0x64) = (int)arg2;
}

/*
 * DetachManipulator: unlink `node` from the list at arg0+0x64 (next
 * pointer at +8), then clear it with func_001F99B0(node, 0, 0x40).
 * The head is read twice, for the test and again for `cur`: the copy
 * that makes lands in the bnel's slot and leaves retail's two nops.
 */
void DetachManipulator(void *arg0, void *arg1) {
    char *base = (char *)arg0;
    char *node = (char *)arg1;
    char *cur;

    if (node == 0) {
        return;
    }
    if (*(char **)(base + 0x64) == node) {
        *(char **)(base + 0x64) = *(char **)(node + 8);
    } else {
        cur = *(char **)(base + 0x64);
        while (*(char **)(cur + 8) != 0 && *(char **)(cur + 8) != node) {
            cur = *(char **)(cur + 8);
        }
        if (*(char **)(cur + 8) == node) {
            *(char **)(cur + 8) = *(char **)(node + 8);
        }
    }
    FastMemSet(node, 0, 0x40);
}

extern int D_001B2F40[];

int AnimSlot_Register(int v) {
    int i = 0;
    int *p = D_001B2F40;
    do {
        if (*p == 0 || *p == v) {
            *p = v;
            return i;
        }
        i++;
        p++;
    } while (i < 0x10);
    return -1;
}

void AnimSlot_ReleaseStale(void) {
    int sentinel = 0xFF;
    char **p = (char **)D_001B2F40;
    int i = 0xF;
    do {
        char *e = *p;
        i--;
        if (e != 0) {
            if ((*(unsigned char *)(e + 0x20) & 0x80) != 0 ||
                *(unsigned char *)(e + 0x52) != sentinel) {
                *p = 0;
            }
        }
        p++;
    } while (i >= 0);
}

/* func_001FA460 is declared above with a single argument, for the
   func_00215328 site; this one passes a source as well. */
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);

/* Scale the single vertex block at arg2+0x30 by the object's 0x2C field
   in 1/1024 units, then run it through the two per-object transforms at
   arg0+0xC0 and arg0+0x10. 0x3A800000 is exactly 2^-10. */
void MobyTransformScaledVertex(char *arg0, int arg1, char *arg2) {
    char buf[0x40];
    int n;
    char *v;
    float s;

    s = *(float *)(arg0 + 0x2C) * 0.0009765625f;
    n = arg1;
    func_002116A0(arg0, 1, &n, arg2);
    v = arg2 + 0x30;
    FastVecScale(v, v, s);
    func_001FA460_2(buf, arg0 + 0xC0);
    sce_vu0_mul_matrix(arg2, buf, arg2);
    FastVecAdd(v, v, arg0 + 0x10);
}

/* The many-vertex form of the same thing: arg1 blocks of 0x10 bytes
   starting at arg3, each scaled and transformed in place. */
void MobyTransformScaledVertexBatch(char *arg0, int arg1, void *arg2, char *arg3) {
    char *v = arg3;
    int n = arg1;
    float s;

    s = *(float *)(arg0 + 0x2C) * 0.0009765625f;
    func_00211548(arg0, arg1, arg2, arg3);
    if (n > 0) {
        do {
            FastVecScale(v, v, s);
            n--;
            func_001F9EC0(v, v, arg0 + 0xC0);
            FastVecAdd(v, v, arg0 + 0x10);
            v += 0x10;
        } while (n != 0);
    }
}

LINKER_REMNANT("asm/remnants/text", func_0020DC38);

extern int D_0016000C MACRO_ADDR;
extern int D_00161000 MACRO_ADDR;
extern int D_0015EF74 MACRO_ADDR;
extern void func_00212258(int);
extern void func_00234E80(void);

/* DmaMobyTextures: splices the texture uploads into the DMA chain with
   "next" tags (0x20000000). D_00161000 is the packet write pointer and
   D_0016000C the tag slot DrawMobysSetup reserved. Every access goes
   back to the globals, because each store through them could alias. */
void DmaMobyTextures(void) {
    int *p = (int *)D_00161000;

    D_00161000 += 0x10;
    ((int *)D_0016000C)[0] = 0x20000000;
    ((int *)D_0016000C)[1] = D_00161000;
    ((int *)D_0016000C)[2] = 0;
    ((int *)D_0016000C)[3] = 0;
    if (D_0018A3B0[10] != 0 && D_0018A3B0[9] != 0) {
        func_00212258(D_0015EF74);
        VU1_texFlush();
    }
    ((int *)D_00161000)[0] = 0x20000000;
    ((int *)D_00161000)[1] = D_0016000C + 0x10;
    ((int *)D_00161000)[2] = 0;
    ((int *)D_00161000)[3] = 0;
    D_00161000 += 0x10;
    p[0] = 0x20000000;
    p[1] = D_00161000;
    p[2] = 0;
    p[3] = 0;
}

extern int D_001B6880[];
extern short D_001B6100[];

/* A class's joint records (the list at class+0x20): the joint's tag
   bytes, 0xFF-terminated, then the address of its first 0x40-byte GIF
   entry, whose sign bit marks the last joint. */
typedef struct {
    unsigned char tags[0xC];
    int entry;
} MobyJoint;

/* PatchMobyGifs: for each class in the negative-terminated D_001B6880
   list, walk its joints; each tag looks up two 14-bit values in
   D_001B6100 and ORs the non-zero ones into the low bits of the +0x30 /
   +0x40 words of successive GIF entries. The outer pointer is advanced
   through a `next` named at the top of the body (retail computes p + 1
   there), and the joint loop tests and breaks before its increment: in
   a do/while the increment is an expression GCSE hoists to the loop top. */
void PatchMobyGifs(void) {
    int *p;
    int *next;

    for (p = D_001B6880; *p >= 0; p = next) {
        MobyJoint *joint;

        next = p + 1;
        joint = *(MobyJoint **)(*(char **)((char *)D_001B3580 + *p * 4) + 0x20);
        for (;;) {
            unsigned char *t = joint->tags;
            char *entry = (char *)(joint->entry & 0x7FFFFFFF);

            while (*t != 0xFF) {
                short *lut = D_001B6100 + *t * 2;
                short v;
                v = lut[0];
                if (v != 0) {
                    *(int *)(entry + 0x30) = (*(int *)(entry + 0x30) & 0xFFFFC000) | v;
                }
                v = lut[1];
                t++;
                if (v != 0) {
                    *(int *)(entry + 0x40) = (*(int *)(entry + 0x40) & 0xFFFFC000) | v;
                }
                entry += 0x40;
            }
            if (joint->entry < 0) {
                break;
            }
            joint++;
        }
    }
}

extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);

void UpdateLightDirection(void) {
    float buf[4];
    float a;

    func_0020E360((void *)D_001414D0, buf);
    a = func_001FA058(buf[0], buf[1]);
    D_001CAE00[0] = FastCos(a) * 0.14f;
    D_001CAE00[1] = FastSin(a) * 0.14f;
    D_001CAE00[2] = -0.99f;
}

extern int D_0016003C MACRO_ADDR;
extern int D_00160040 MACRO_ADDR;
extern char D_0015FFC0[];
extern char D_001C8A00[];
extern void func_00228A58(void);
extern void func_00228860(void *);
extern void func_001F2558(void *, int);

/* The same splice as DmaMobyTextures, around func_00228A58/func_00228860,
   at the tag slot D_00160040; with D_0016003C clear the slot becomes a
   plain "cnt" tag (0x10000000) instead. The write pointer goes through
   a temporary that is advanced in place (t += 0x10): that breaks CSE's
   p == t equivalence, which is what keeps retail's `daddu $16,$2,$0`
   copy. `p = D_00161000; D_00161000 += 0x10;` is 4 bytes short. */
void SpliceShadowDma(void) {
    int *p;
    int t;

    if (D_0016003C == 0) {
        ((int *)D_00160040)[0] = 0x10000000;
        ((int *)D_00160040)[1] = 0;
        ((int *)D_00160040)[2] = 0;
        ((int *)D_00160040)[3] = 0;
        return;
    }
    t = D_00161000;
    p = (int *)t;
    t += 0x10;
    D_00161000 = t;
    ((int *)D_00160040)[0] = 0x20000000;
    ((int *)D_00160040)[1] = D_00161000;
    ((int *)D_00160040)[2] = 0;
    ((int *)D_00160040)[3] = 0;
    func_00228A58();
    SubmitGraphicsSetupStream(D_001C8A00);
    ((int *)D_00161000)[0] = 0x20000000;
    ((int *)D_00161000)[1] = D_00160040 + 0x10;
    ((int *)D_00161000)[2] = 0;
    ((int *)D_00161000)[3] = 0;
    D_00161000 += 0x10;
    p[0] = 0x20000000;
    p[1] = D_00161000;
    p[2] = 0;
    p[3] = 0;
    func_001F2558(D_0015FFC0, 8);
}

extern void func_00118D80(int);
extern char D_00165600[];
extern int D_0015F718 MACRO_ADDR;
extern int D_0015F71C MACRO_ADDR;

/* ProcessMobyAnimData(void). Both globals are MACRO_ADDR: D_0015F718
   loads with retail's one-register lui $4 / lw $4, and D_0015F71C's
   load, scheduled into the jal delay slot, becomes $gp-relative. */
void ProcessMobyAnimData(void) {
    func_00118D80(0);
    FastMemCopy((void *)0x70003800, D_00165600, 0x800);
    MobyAnimProc(D_0015F718, D_0015F71C);
}


/* InitMobyClassDists(void) */
void InitMobyClassDists(void) {
    FastMemSet((void *)0x70003A00, (void *)0x40000000, 0x380);
}

extern char D_001B3200[];

/* StashMobyClassDists(void) */
void StashMobyClassDists(void) {
    FastMemCopy(D_001B3200, (void *)0x70003A00, 0x380);
}

/* RestoreMobyClassDists(void) */
void RestoreMobyClassDists(void) {
    FastMemCopy((void *)0x70003A00, D_001B3200, 0x380);
}

extern void func_00234B48(void *, int);
extern void func_002347F0(void *);
extern void func_00234C98(int, long);
extern void func_001F2560(void *, int);
extern unsigned short D_0010FA90 NOT_SDA;
extern char D_0010FAA0[];
extern int D_0015F704 MACRO_ADDR;
extern char D_00100080[];
extern int D_0015EF78 MACRO_ADDR;
extern char D_0015FFD0[];
extern int D_00160014 MACRO_ADDR;
extern int D_00161008 MACRO_ADDR;

/* DrawMobysSetup(void). What an older note here called a dead
   `&D_0015FFD0` and an unexplained `li $5,1` are the arguments of the
   profiling marker func_001F2560. The packet pointer goes through a
   local advanced in place, as in DrawShrubs. func_00234C98's second
   parameter being `long` is what orders its arguments like retail. */
void DrawMobysSetup(void) {
    int p;

    VU1_addDataRef(D_0010FAA0, D_0010FA90);
    D_0015F704 = 6;
    VU0_loadMicroProgram(D_00100080);
    VU1_addGSregister(0x47, 0x5360B);
    p = D_00161000;
    D_0016000C = p;
    D_0015EF74 = D_0015EF78;
    p += 0x10;
    D_00161000 = p;
    func_001F2560(D_0015FFD0, 1);
    D_00160040 = 0;
    D_00161008 = D_0015F71C - 0x10000;
    D_00160014 = D_0015F718;
}

extern int func_00212658(int, int, int, int);

/* DrawMobyList */
void DrawMobyList(int arg0, int arg1) {
    VU1_addGSregister(0x47, 0x5360B);
    func_00118D80(0);
    RestoreMobyClassDists();
    D_00160014 = func_00212658(arg0, D_00160014, arg1, 0);
    StashMobyClassDists();
    D_00160014 -= 0x10;
}

extern void func_0020DC40(void);
extern void func_0020DFF8(void);
extern void func_00212508(void);
extern char D_0015FFE0[];
extern char D_0015FFF0[];
extern int D_00160038 MACRO_ADDR;

/* DrawMobysCleanUp. The empty profiling markers take (void *, int);
   DmaMobyTextures and func_00212508 take nothing. With the arguments
   right, D_0015FFF0 is a plain array, not MACRO_ADDR. */
void DrawMobysCleanUp(void) {
    func_001F2560(D_0015FFE0, 3);
    DmaMobyTextures();
    func_001F2558(D_0015FFE0, 5);
    if (D_0018A3B0[10] != 0) {
        ProcessMobyAnimData();
        if (D_00160038 != 0) {
            func_00212508();
        }
    }
    func_001F2558(D_0015FFF0, 3);
    if (D_0018A3B0[10] != 0 && D_00160040 != 0) {
        SpliceShadowDma();
    }
}

extern int D_0018A3D8;
extern char D_001E8730[];
extern void func_0020E0C8(void);
extern void func_0020E200(void);

/* DrawMobys */
void DrawMobys(void) {
    DrawMobysSetup();
    if (D_0018A3D8 != 0) {
        InitMobyClassDists();
        D_00160014 = func_00212658(D_00160018, D_00160014, -1, 1);
        if (D_00161000 > D_00161008) {
            STUB_printf(D_001E8730);
        }
    }
    DrawMobysCleanUp();
}

LINKER_REMNANT("asm/remnants/text", func_0020E330);

void func_0020E340(u64 *command_words, u64 upper_field, u64 middle_field, u64 low_field, u64 tail_field);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/commands/pack_render_command_fields.c, PackRenderCommandFields. */
void func_0020E340(u64 *command_words, u64 upper_field,
                             u64 middle_field, u64 low_field, u64 tail_field) {
    int command_word_index;
    if (middle_field) {
        upper_field <<= 32;
        low_field <<= 8;
        tail_field <<= 16;
    } else {
        upper_field <<= 32;
        low_field <<= 8;
        tail_field <<= 16;
    }
    {
        register u64 packed_command;
        packed_command = upper_field | middle_field;
        packed_command |= low_field;
        packed_command |= tail_field;
        command_word_index = 7;
        command_words[command_word_index] = packed_command;
    }
}

ASM_FUNC("asm/handwritten/text", func_0020E360);

ASM_FUNC("asm/handwritten/text", func_0020E3D0);
