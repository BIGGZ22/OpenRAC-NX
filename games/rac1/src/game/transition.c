#include "common.h"
#include "callees.h"
#include "moby.h"
#include "structs.h"

/*
 * transition.cpp in the original source; text 0x1E9E70-0x1EC038.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/*
 * Close but not yet byte-matching, new evidence for the sq/lq open
 * question below: this function only saves $ra (no $s0-$s7 at all), and
 * retail STILL spills it as `sq` here -- unlike every other function
 * seen so far, where retail consistently uses `sd` for a lone $ra save.
 * This compiler always uses `sd` for $ra regardless. Logic/instructions
 * otherwise identical (return func_0022C7E0(); ... 5 calls in a row,
 * body confirmed correct via objdump before reverting this to
 * INCLUDE_ASM):
 *   func_0022C7E0(); func_0022C188(); func_0022C870();
 *   func_00234C98(0x47, 0x5360B);
 *   func_00234C98(0x4E, 0x1000000 | (D_0015EF88 >> 13));
 * Means the sq/lq choice isn't purely "s-regs vs ra", it's something
 * more granular retail decides per-function (maybe per translation
 * unit, or some other property not yet isolated). See "Open toolchain
 * questions" in docs/DECOMP_PROGRESS.md.
 */
extern void func_0022C7E0(void);
extern void func_0022C188(void);
extern void func_0022C870(void);
extern void func_00234C98(int, long);
extern int D_0015EF88 MACRO_ADDR;

/* func_00234C98 is (int, long) and D_0015EF88 MACRO_ADDR; the older
   sq/sd note is obsolete. */
void Transition_DrawSky(void) {
    SetupSkyGifPaging();
    SkyLevelGeneric___maybe();
    DoSkyGifPaging();
    VU1_addGSregister(0x47, 0x5360B);
    VU1_addGSregister(0x4E, 0x1000000 | (D_0015EF88 >> 13));
}

/* Level init from the level header hdr: loads the frame and fog constants, uploads the VU0
   program, unpacks the shrub and tie records (class lists, rotations, scales, colour
   averages) into the level arena, clears the ent tables and returns the next free address. */

typedef struct { int x0; int x4; unsigned char pad08[0x2C]; int x34; unsigned char pad38[4]; int x3C; } LevelHeaderE9EC8;
typedef struct { float v[3]; float w; } __attribute__((aligned(16))) FRowE9EC8;
typedef struct { float v[3]; int w; } __attribute__((aligned(16))) IRowE9EC8;
typedef struct { FRowE9EC8 m[4]; unsigned char pad40[0x100]; unsigned char x140[0x80]; } ShrubDataE9EC8;
typedef struct { IRowE9EC8 m[3]; FRowE9EC8 m3; } TieDataE9EC8;
typedef struct {
    float v[3]; float scale; void *data; float x14; short x18; unsigned char cls; unsigned char x1B;
    unsigned short x1C; unsigned short x1E;
} ShrubE9EC8;
typedef struct {
    float v[3]; float scale; float radius; unsigned char pad14[3]; unsigned char x17; short x18;
    unsigned char cls; unsigned char x1B; unsigned short x1C; unsigned short x1E;
} TieE9EC8;
typedef struct {
    unsigned char pad00[0x26]; unsigned short count; ShrubE9EC8 *first; unsigned char pad2C[4];
    float x30[3]; float scale; float x40;
} ShrubClassE9EC8;
typedef struct {
    unsigned char pad00[0xC]; float scale; unsigned char pad10[6]; unsigned short count;
    TieE9EC8 *first; float *x1C; float x20;
} TieClassE9EC8;
typedef struct { unsigned char pad00[0x36]; unsigned short x36; unsigned char pad38[2]; short x3A; unsigned char pad3C[4]; } Ent40E9EC8;
typedef struct { unsigned char pad00[0x20]; unsigned char x20; unsigned char pad21[0x15]; short x36; unsigned char pad38[0xC8]; } Ent100E9EC8;
typedef struct { unsigned char pad00[0x23C]; int x23C; int x240; int x244; } DisplayE9EC8;
typedef struct { int x0; unsigned char *x4; unsigned char *x8; int xC; int x10; int x14; } BufE9EC8;
typedef struct { int x0; unsigned char *x4; } S160E9EC8;

extern int func_001F99B0();
extern void func_001F99D8(void *, int);
extern void func_001F9A00(void *, void *, int);
extern void func_001E9730(char *);
extern void func_002157C0(int);
extern int func_001F2930();
extern void func_001FB448(int, int, int);
extern int func_002347F0();
extern int func_002362B0();
extern int func_0022B8F8();
extern int func_00238688();
extern float func_001F9CB8(void *);
extern float func_001F9B90(float, float);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9C48(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_001FA898(float);
extern float func_001FA888(int);
extern void func_001F3B90(void);
extern void func_00217EC0(void);
extern void func_001F6598(void);

extern int D_0015EE80 MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern int D_0016044C MACRO_ADDR;
extern unsigned char *D_001941D8;
extern unsigned char D_0013F450[];
extern unsigned char D_00187040[];
extern unsigned char D_001AADC0[];
extern BufE9EC8 D_00194200;
extern unsigned char D_001942C0[];
extern unsigned char D_00194280[];
extern unsigned char D_001B7A60[];
extern unsigned char D_001C7A60[];
extern DisplayE9EC8 D_0018CE00;
extern int D_0015F584 MACRO_ADDR;
extern int D_0015F585 MACRO_ADDR;
extern int D_0015F586 MACRO_ADDR;
extern float D_0015F588 MACRO_ADDR;
extern float D_0015F58C MACRO_ADDR;
extern float D_0015F590 MACRO_ADDR;
extern float D_0015F594 MACRO_ADDR;
extern float D_00160FC0 MACRO_ADDR;
extern float D_00161070 MACRO_ADDR;
extern float D_001604E4 MACRO_ADDR;
extern int D_00160030 MACRO_ADDR;
extern int D_001601BC MACRO_ADDR;
extern unsigned char D_00100AE0[];
extern unsigned char D_0019C2C0[];
extern unsigned char D_0019C4C0[];
extern unsigned char D_0019BEC0[];
extern char D_001E79D8[];
extern int D_00160F90 MACRO_ADDR;
extern Ent40E9EC8 *D_00160F8C MACRO_ADDR;
extern int D_0016105C MACRO_ADDR;
extern ShrubE9EC8 *D_00161050 MACRO_ADDR;
extern ShrubE9EC8 *D_00161054 MACRO_ADDR;
extern ShrubDataE9EC8 *D_00161058 MACRO_ADDR;
extern unsigned char D_001E1D00[];
extern ShrubClassE9EC8 *D_001E1A00[];
extern unsigned char D_001D8440[];
extern TieClassE9EC8 *D_001D82C0[];
extern int D_001604D0 MACRO_ADDR;
extern TieE9EC8 *D_001604D4 MACRO_ADDR;
extern TieE9EC8 *D_001604D8 MACRO_ADDR;
extern TieDataE9EC8 *D_001604DC MACRO_ADDR;
extern unsigned char *D_001604E0 MACRO_ADDR;
extern S160E9EC8 D_00160BB0;
extern Ent100E9EC8 *D_00160018 MACRO_ADDR;
extern Ent100E9EC8 *D_0016001C MACRO_ADDR;
extern Ent100E9EC8 *D_00160020 MACRO_ADDR;
extern unsigned char *D_00160028 MACRO_ADDR;
extern unsigned char *D_001601AC MACRO_ADDR;
extern int D_001601B0 MACRO_ADDR;
extern int D_001601B4 MACRO_ADDR;
extern int D_001601B8 MACRO_ADDR;
extern unsigned char D_001CDB00[];
extern unsigned char D_0018C418[];
extern int D_0015F6F0 MACRO_ADDR;
extern float D_0015F53C MACRO_ADDR;

/* Level-load parser: builds the runtime object tables from the level header and returns the end pointer. */
unsigned char *Level_ParseObjectTables(LevelHeaderE9EC8 *hdr)
{
    unsigned char *p;
    unsigned char *c;
    unsigned char *lc;
    unsigned char *sc;
    unsigned char *tc;
    unsigned short *sp;
    unsigned short *spa;
    unsigned short *spb;
    int n;
    int i;
    int i0;
    int i1;
    int i2;
    int i3;
    int i5;
    int i6;
    int j;
    int k;
    int id;
    int cls;
    int last;
    int lastTie;
    int size0;
    int size1;
    int size2;
    int size3;
    int size4;
    int r;
    int g;
    int bl;
    int x;
    int y;
    float s;
    float a;
    float b;
    float len0;
    float len1;
    float len2;
    float rad;
    ShrubE9EC8 *sh;
    ShrubDataE9EC8 *sd;
    TieE9EC8 *tie;
    TieDataE9EC8 *td;
    Ent100E9EC8 *e;
    ShrubE9EC8 *o;
    FRowE9EC8 *r1;
    FRowE9EC8 *r2;
    FRowE9EC8 *r3;
    IRowE9EC8 *t1;
    IRowE9EC8 *t2;
    FRowE9EC8 *t3;

    p = D_001941D8;
    ((void (*)(void *, int, int))FastMemSet)(D_0013F450, 0, 0x2310);
    ((void (*)(void *, int, int))FastMemSet)(gCamera, 0, 0x3A0);
    ((void (*)(void *, int, int))FastMemSet)(D_001AADC0, 0, 0x180);
    if (D_0015EE80 != 0) {
        if (D_0015EE60 == 1.0f) {
            SetVideoModeTiming(1);
        }
    } else if (D_0015EE60 != 1.0f) {
        SetVideoModeTiming(0);
    }
    *(unsigned char *)&D_0016044C = *(unsigned char *)&D_0015EE80;
    D_00194200.x4 = D_001942C0;
    D_00194200.x8 = D_00194280;
    D_00194200.xC = 0;
    D_00194200.x10 = 0;
    D_00194200.x14 = 0;
    FastMemZero16(D_001B7A60, 0x10000);
    FastMemZero16(D_001C7A60, 0x60);

    c = (unsigned char *)hdr + hdr->x0;
    D_0018CE00.x23C = *(int *)c;
    c += 4;
    D_0018CE00.x240 = *(int *)c;
    c += 4;
    D_0018CE00.x244 = *(int *)c;
    c += 4;
    *(unsigned char *)&D_0015F584 = *c;
    c += 4;
    *(unsigned char *)&D_0015F585 = *c;
    c += 4;
    *(unsigned char *)&D_0015F586 = *c;
    c += 4;
    D_0015F588 = *(float *)c;
    D_0015F58C = *(float *)(c + 4);
    D_0015F590 = *(float *)(c + 8);
    D_0015F594 = *(float *)(c + 12);
    ((void (*)(void))UpdateFog)();
    D_00160FC0 = 512000.0f;
    D_00161070 = 720.0f;
    D_001604E4 = 500.0f;
    D_00160030 = 500;
    D_001601BC = 0x1F4000;
    SetBackgroundColor(D_0018CE00.x23C, D_0018CE00.x240, D_0018CE00.x244);
    ((void (*)(void *))VU0_loadMicroProgram)(D_00100AE0);
    FastMemZero16(D_0019C2C0, 0x100);
    FastMemZero16(D_0019C4C0, 0x180);
    FastMemZero16(D_0019BEC0, 0x400);

    lc = (unsigned char *)hdr + hdr->x4;
    n = *(int *)lc;
    lc += 0x10;
    if (n >= 12) {
        STUB_printf(D_001E79D8);
        n = 12;
    }
    if (n != 0) {
        func_001F9A00(D_0019BEC0, lc, n << 6);
    }

    for (i0 = 0; i0 < D_00160F90; i0++) {
        D_00160F8C[i0].x36 = 0xFFFF;
    }
    i1 = 0;
    while (i1 < D_00160F90) {
        sp = (unsigned short *)0x70003000;
        for (j = 0; j < 0x3FF && i1 < D_00160F90; j++, i1++) {
            *sp++ = (unsigned short)i1;
        }
        *sp = 0xFFFF;
        ((void (*)(void *))LightTfrags)((void *)0x70003000);
    }

    sc = (unsigned char *)hdr + hdr->x34;
    D_0016105C = *(int *)sc;
    sc += 0x10;
    D_00161050 = (ShrubE9EC8 *)p;
    size0 = D_0016105C << 5;
    p += size0;
    if (D_0016105C != 0) {
        ((void (*)(void *, int, int))FastMemSet)(D_00161050, 0, size0);
    }
    size1 = D_0016105C * 0x1C0;
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    D_00161058 = (ShrubDataE9EC8 *)p;
    p += size1;
    if (D_0016105C != 0) {
        ((void (*)(void *, int, int))FastMemSet)(D_00161058, 0, size1);
    }
    last = -1;
    D_00161054 = &D_00161050[D_0016105C];
    for (i2 = 0; i2 < D_0016105C; i2++) {
        sh = &D_00161050[i2];
        sd = &D_00161058[i2];
        sh->cls = D_001E1D00[*(int *)sc];
        if (sh->cls != last) {
            D_001E1A00[sh->cls]->first = sh;
            D_001E1A00[sh->cls]->count = 0;
            last = sh->cls;
        }
        D_001E1A00[sh->cls]->count++;
        sh->data = sd;
        sh->x14 = (float)*(int *)(sc + 4);
        sh->x1E = 0xFFFF;
        sh->x1B = 0;
        sh->x1C = 0;
        id = *(int *)(sc + 0xC);
        qcopy(sd, sc + 0x10);
        r1 = &sd->m[1];
        qcopy(r1, sc + 0x20);
        r2 = &sd->m[2];
        qcopy(r2, sc + 0x30);
        r3 = &sd->m[3];
        qcopy(r3, sc + 0x40);
        sh->x18 = (short)id;
        sc += 0x50;
        sd->m[3].w = D_001E1A00[sh->cls]->x40;
        func_001F9EC0(sh, D_001E1A00[sh->cls]->x30, sd);
        s = FastVecLength(sd);
        len1 = FastVecLength(r1);
        s = func_001F9B90(s, len1);
        len2 = FastVecLength(r2);
        s = func_001F9B90(s, len2);
        sh->scale = D_001E1A00[sh->cls]->scale * s;
        func_001F9C48(sh, sh, sd->m[3].w);
        FastVecAdd(sh, sh, r3);
        D_00161058[i2].m[0].w = 1.0f / FastVecLength(&D_00161058[i2].m[0]);
        D_00161058[i2].m[1].w = 1.0f / FastVecLength(&D_00161058[i2].m[1]);
        D_00161058[i2].m[2].w = 1.0f / FastVecLength(&D_00161058[i2].m[2]);
        func_001F9A00(sd->x140, sc, 0x80);
        sc += 0x80;
        sh->x1C = *(unsigned short *)sc;
        sc += 0x10;
    }
    spa = (unsigned short *)0x70000000;
    for (i3 = 0; i3 < D_0016105C; i3++) {
        *spa++ = (unsigned short)i3;
    }
    *spa = 0xFFFF;
    ((void (*)(void *))LightTies)((void *)0x70000000);

    tc = (unsigned char *)hdr + hdr->x3C;
    D_001604D0 = *(int *)tc;
    tc += 0x10;
    D_001604D4 = (TieE9EC8 *)p;
    size2 = D_001604D0 << 5;
    p += size2;
    if (D_001604D0 != 0) {
        ((void (*)(void *, int, int))FastMemSet)(D_001604D4, 0, size2);
    }
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    D_001604DC = (TieDataE9EC8 *)p;
    size3 = D_001604D0 << 6;
    p += size3;
    if (D_001604D0 != 0) {
        ((void (*)(void *, int, int))FastMemSet)(D_001604DC, 0, size3);
    }
    D_001604E0 = p;
    size4 = D_001604D0 * 0x60;
    p += size4;
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    if (D_001604D0 != 0) {
        ((void (*)(void *, int, int))FastMemSet)(D_001604E0, 0, size4);
    }
    lastTie = -1;
    D_00160BB0.x4 = D_001604E0;
    D_001604D8 = D_001604D4 + D_001604D0;

    for (i = 0; i < D_001604D0; i++) {
        tie = &D_001604D4[i];
        td = &D_001604DC[i];
        cls = D_001D8440[*(int *)tc];
        tie->cls = (unsigned char)cls;
        if (cls != lastTie) {
            lastTie = cls;
            D_001D82C0[cls]->first = tie;
            D_001D82C0[cls]->count = 0;
        }
        D_001D82C0[cls]->count++;
        tie->x18 = (short)i;
        tie->radius = *(float *)(tc + 4);
        if (tie->radius < 16.0f) {
            tie->radius = 16.0f;
        }
        tie->x1B = 0;
        tie->x1E = 0xFFFF;
        if (D_001D82C0[cls]->x1C != 0) {
            tie->x17 = (unsigned char)truncate_float_to_s32(*D_001D82C0[cls]->x1C);
            rad = IntToFloat(tie->x17) + 24.0f;
            if (tie->radius < rad) {
                tie->radius = rad;
            }
        }
        qcopy(td, tc + 0x10);
        t1 = &td->m[1];
        qcopy(t1, tc + 0x20);
        t2 = &td->m[2];
        qcopy(t2, tc + 0x30);
        t3 = &td->m3;
        qcopy(t3, tc + 0x40);
        tc += 0x50;
        td->m3.w = D_001D82C0[cls]->x20;
        len0 = FastVecLength(td);
        len1 = FastVecLength(t1);
        a = (len0 + len1) * 0.5f;
        b = FastVecLength(t2);
        x = truncate_float_to_s32(a * 4096.0f);
        y = truncate_float_to_s32(b * 4096.0f);
        if (x > 0x10000) {
            x = 0x10000;
        }
        if (y > 0x10000) {
            y = 0x10000;
        }
        td->m[2].w = (int)((unsigned int)x | ((unsigned int)y << 16));
        r = *(int *)tc;
        tc += 4;
        g = *(int *)tc;
        tc += 4;
        bl = *(int *)tc;
        tc += 8;
        g = (int)(((unsigned int)g << 8) | 0x80000000U);
        td->m[0].w = (int)(((unsigned int)bl << 16) | (unsigned int)g | (unsigned int)r);
        tie->x1C = *(unsigned short *)tc;
        tc += 0x10;
        func_001F9EC0(tie, D_001D82C0[cls], td);
        s = FastVecLength(td);
        len1 = FastVecLength(t1);
        s = func_001F9B90(s, len1);
        len2 = FastVecLength(t2);
        s = func_001F9B90(s, len2);
        tie->scale = D_001D82C0[cls]->scale * s;
        func_001F9C48(tie, tie, td->m3.w);
        FastVecAdd(tie, tie, t3);
    }
    spb = (unsigned short *)0x70000000;
    for (i5 = 0; i5 < D_001604D0; i5++) {
        *spb++ = (unsigned short)i5;
    }
    *spb = 0xFFFF;
    ((void (*)(void *))LightShrubs)((void *)0x70000000);

    for (i6 = 0; i6 < D_001604D0; i6++) {
        unsigned char *cp;
        int cr;
        int cg;
        int cb;

        cp = D_001604E0 + i6 * 0x60;
        cr = 0;
        cg = 0;
        cb = 0;
        for (k = 0; k < 24; k++) {
            cr += cp[0];
            cg += cp[1];
            cb += cp[2];
            cp += 4;
        }
        cr /= 24;
        cg /= 24;
        cb /= 24;
        *(int *)((unsigned char *)&D_001604DC[i6] + 0x1C) = (cb << 16) | (cg << 8) | cr;
    }

    D_00160018 = (Ent100E9EC8 *)p;
    ((void (*)(void *, int, int))FastMemSet)(p, 0, 0x4000);
    p += 0x4000;
    D_0016001C = D_00160018;
    *((unsigned char *)D_00160018 + 0x20) = 0xFF;
    D_00160028 = p;
    p += 0x2000;
    D_001601AC = p;
    D_00160020 = (Ent100E9EC8 *)((unsigned char *)D_00160018 + 0x3F00);
    p += 0x20000;
    D_001601B4 = -1;
    D_001601B0 = 0;
    D_001601B8 = 0;
    FastMemZero16(D_001CDB00, 0x200);
    for (e = D_00160018; e != D_00160020; e++) {
        e->x36 = 0x7F80;
    }
    for (o = D_00161050; o != D_00161054; o++) {
        o->x18 = 0x7F80;
    }
    for (i0 = 0; i0 < D_00160F90; i0++) {
        D_00160F8C[i0].x3A = 0x7F80;
    }
    {
        char *st = (char *)gDebugFreeCam;

        *(int *)(st + 0x1C) = 0;
    }
    ResetDrawGlobals();
    pad_resetState();
    Font_EnableColorCodes();
    D_0015F6F0 = 0;
    gFade = 1.0f;
    return p;
}

struct DiscFile_EABE8 {
    s32 sector;
    s32 size;
};

struct DiscTable_EABE8 {
    u8 pad_0[0x14E8];
    struct DiscFile_EABE8 unk14E8;         /* 0x14E8: the transition wad */
    u8 pad_14F0[0x38];
    struct DiscFile_EABE8 help_text;       /* 0x1528 */
};

extern struct DiscTable_EABE8 D_00137C80_EABE8 __asm__("D_00137C80");

typedef struct 
{
  s32 x0;
  s32 data;
  s32 x8;
  s32 xC;
  s32 x10;
  s32 x14;
  s32 n18;
  s32 x1C;
  s32 n20;
  s32 x24;
  s32 n28;
  s32 x2C;
  s32 n30;
  s32 x34;
  s32 n38;
  s32 x3C;
  s32 n40;
  s32 x44;
  s32 n48;
  s32 x4C;
  s32 x50;
  s32 x54;
  s32 x58;
  s32 x5C;
  s32 x60;
  s32 x64;
  s32 x68;
  s32 x6C;
  s32 x70;
  s32 x74;
  s32 x78;
  s32 x7C;
  s32 x80;
  s32 x84;
} WadHeader_EABE8;
typedef struct 
{
  s32 offset;
  u16 x4;
  u16 pad6;
  s32 pad8[2];
} WadTex_EABE8;
typedef struct 
{
  s32 offset;
  s32 x4;
  s32 pad8[2];
  u8 x10[0x10];
} WadClass20_EABE8;
typedef struct 
{
  s32 offset;
  s32 x4;
  s32 pad8[2];
  u8 x10[0x10];
  u8 x20[0x10];
} WadClass30_EABE8;
typedef struct 
{
  s32 offset;
  s32 size;
} WadSound_EABE8;
typedef struct 
{
  u8 pad0[0x14];
  u8 *hdr;
  s32 x18;
  s32 x1C;
} LoadState_EABE8;
typedef struct 
{
  u8 pad0[0x58];
  s32 x58;
  s32 x5C;
  s32 x60[0x46];
} SoundBanks_EABE8;
typedef struct 
{
  s32 offset;
  s32 pad[3];
} TextEntry_EABE8;
typedef struct 
{
  u8 pad0[0x2C];
  s32 count;
} HelpState_EABE8;
extern s32 D_0015EF74_EABE8 __asm__("D_0015EF74") MACRO_ADDR;
extern s32 D_0015EF78_EABE8 __asm__("D_0015EF78") MACRO_ADDR;
extern s32 D_0015EF8C_EABE8 __asm__("D_0015EF8C") MACRO_ADDR;
extern s64 D_0015F048_EABE8 __asm__("D_0015F048") MACRO_ADDR;
extern s32 D_0015F058_EABE8 __asm__("D_0015F058") MACRO_ADDR;
extern s32 D_0015F060_EABE8 __asm__("D_0015F060") MACRO_ADDR;
extern s32 D_0015F064_EABE8 __asm__("D_0015F064") MACRO_ADDR;
extern s32 D_0015F560_EABE8 __asm__("D_0015F560") MACRO_ADDR;
extern TextEntry_EABE8 *D_0015F780_EABE8 __asm__("D_0015F780") MACRO_ADDR;
extern s32 D_00160000_EABE8 __asm__("D_00160000") MACRO_ADDR;
extern s32 D_00160008_EABE8 __asm__("D_00160008") MACRO_ADDR;
extern s32 D_001604CC_EABE8 __asm__("D_001604CC") MACRO_ADDR;
extern s32 D_001604EC_EABE8 __asm__("D_001604EC") MACRO_ADDR;
extern s32 D_00160F94_EABE8 __asm__("D_00160F94") MACRO_ADDR;
extern s32 D_0016100C_EABE8 __asm__("D_0016100C") MACRO_ADDR;
extern s32 D_0016104C_EABE8 __asm__("D_0016104C") MACRO_ADDR;
extern s32 D_00161064_EABE8 __asm__("D_00161064") MACRO_ADDR;
extern u8 D_001862E0_EABE8[] __asm__("D_001862E0");
extern u8 D_00186410_EABE8[] __asm__("D_00186410");
extern SoundBanks_EABE8 D_0018CC20_EABE8 __asm__("D_0018CC20");
extern LoadState_EABE8 D_001941C0_EABE8 __asm__("D_001941C0");
extern u8 D_00194280_EABE8[] __asm__("D_00194280");
extern HelpState_EABE8 D_001997D0_EABE8 __asm__("D_001997D0");
extern u64 D_0019E7C0_EABE8[] __asm__("D_0019E7C0");
extern MobyClass *D_001B3580_EABE8[] __asm__("D_001B3580");
extern u8 D_001B3E40_EABE8[] __asm__("D_001B3E40");
extern s32 D_001B5D00_EABE8[] __asm__("D_001B5D00");
extern s32 D_001B6500_EABE8[] __asm__("D_001B6500");
extern u8 D_001B6C00_EABE8[] __asm__("D_001B6C00");
extern s32 D_001D8840_EABE8[] __asm__("D_001D8840");
extern s32 D_001E0C00_EABE8[] __asm__("D_001E0C00");
extern s32 D_001E2900_EABE8[] __asm__("D_001E2900");
extern void func_002348B8_EABE8(void) __asm__("func_002348B8");
extern void func_001F99B0_EABE8(void *, s32, s32) __asm__("func_001F99B0");
extern void func_00118D80_EABE8(s32) __asm__("func_00118D80");
extern void func_001EB300_EABE8(s32) __asm__("func_001EB300");
extern void func_00120858_EABE8(s32, s32) __asm__("func_00120858");
extern s32 func_001E9EC8_EABE8(u8 *) __asm__("func_001E9EC8");
extern void func_001F3008_EABE8(void) __asm__("func_001F3008");
extern void func_001F3140_EABE8(void) __asm__("func_001F3140");
extern char func_001F9968_EABE8(s32) __asm__("func_001F9968");
extern void func_00201E10_EABE8(void) __asm__("func_00201E10");
extern void func_00202F00_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00202F00");
extern void func_00203038_EABE8(u8 *, s32) __asm__("func_00203038");
extern void func_00203118_EABE8(u8 *) __asm__("func_00203118");
extern void func_00203958_EABE8(u8 *, s32, u8 *) __asm__("func_00203958");
extern void func_00203E78_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00203E78");
extern void func_00203F68_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00203F68");
extern void func_00204340_EABE8(u8 *, u8 *, u8 *, u8 *, s32) __asm__("func_00204340");
extern void func_00204918_EABE8(u8 *, u8 *) __asm__("func_00204918");
extern void func_00205220_EABE8(s32) __asm__("func_00205220");
extern s32 func_0020C468_EABE8(u8 *, u8 *) __asm__("func_0020C468");
extern void func_00217628_EABE8(u8 *, s32, s32) __asm__("func_00217628");
extern unsigned int func_002176C8_EABE8(s32, s32, s32) __asm__("func_002176C8");
extern void func_00217748_EABE8(s32) __asm__("func_00217748");
extern void func_002348E8_EABE8(void) __asm__("func_002348E8");
extern s32 func_00122630_EABE8(void *, s16, s16, s16, s16, s16, s16, s16) __asm__("func_00122630");
extern s32 func_00122958_EABE8(void *, u8 *) __asm__("func_00122958");

/* Loads the space-transition wad to 0x1400000 and unpacks it at the level arena: textures and
   their tables, the three kinds of moby classes, sound and help text tables, the loading image;
   then resets the sequence state and relocates the eight help text tables.
   Adapted from Lombyte (MIT) for PAL: src/gameplay/state/transition_load_wad.c, transition_load_wad. */
void LoadSpaceTransitionWad(void)
{
  u8 li[0x60];
  WadHeader_EABE8 *hdr;
  u8 *data;
  u8 *base;
  s32 size;
  s32 i;
  s32 j;
  s32 k;
  s32 cnt;
  s32 v;
  s32 *out;
  int new_var;
  WadTex_EABE8 *tex;
  s32 new_var4;
  WadClass20_EABE8 *c20;
  u8 *new_var3;
  WadClass30_EABE8 *c30;
  WadSound_EABE8 *snd;
  TextEntry_EABE8 *te;
  WadTex_EABE8 *new_var2;
  s32 k_800 = 0x800;
  s32 adj;
  s64 t;
  s64 u;
  D_0015F058_EABE8 = 0;
  i = 0;
  func_002348B8_EABE8();
  func_00201E10_EABE8();
  D_0016100C_EABE8 = 0x100000;
  D_0015EF8C_EABE8 = 0x2C0000;
  D_0015EF78_EABE8 = 0x2C0000;
  D_0015EF74_EABE8 = 0x2C0000;
  func_001F99B0_EABE8(D_00194280_EABE8, 0x87654321, 0x10);
  func_001F99B0_EABE8(D_001B3E40_EABE8, -1, k_800);
  func_001F99B0_EABE8(D_001B6C00_EABE8, -1, 0xE00);
  func_001F99B0_EABE8(D_001B6500_EABE8, 0, 0xE0);
  func_001F3008_EABE8();
  func_001F3140_EABE8();
  func_002348E8_EABE8();
  func_00217628_EABE8((u8 *)0x1400000, D_00137C80_EABE8.unk14E8.sector, D_00137C80_EABE8.unk14E8.size);
  func_00217748_EABE8(1);
  func_00118D80_EABE8(0);
  size = func_0020C468_EABE8((u8 *)0x1400000, D_001941C0_EABE8.hdr);
  func_00118D80_EABE8(0);
  hdr = (WadHeader_EABE8 *) D_001941C0_EABE8.hdr;
  func_00203958_EABE8(((u8 *) hdr) + hdr->x0, hdr->x8, ((u8 *) hdr) + hdr->xC);
  t = ((s64) ((D_0015EF8C_EABE8 + hdr->x70) >> 8)) | 0x1D308000;
  u = (((s64) ((D_0015EF8C_EABE8 + hdr->x74) >> 8)) << 37) | (((s64) 0xB800) << 19);
  data = ((u8 *) hdr) + hdr->data;
  base = data + hdr->x60;
  D_0019E7C0_EABE8[0] = (t | u) | (((s64) (-1)) << 63);
  D_0019E7C0_EABE8[1] = 0xFFA0000000E0;
  D_0019E7C0_EABE8[2] = 0x0040000400004000;
  tex = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x34);
  if ((D_00160F94_EABE8 = hdr->n30) > 0)
  {
    do
    {
      D_001E0C00_EABE8[i] = (((s32) base) + tex[i].offset) + (func_001F9968_EABE8(tex[i].x4) << 28);
      i++;
    }
    while (i < D_00160F94_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x3C));
  if ((D_00160008_EABE8 = hdr->n38) > 0)
  {
    do
    {
      D_001B5D00_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_00160008_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x44));
  if ((D_00161064_EABE8 = hdr->n40) > 0)
  {
    do
    {
      D_001E2900_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_00161064_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x4C));
  if ((D_001604EC_EABE8 = hdr->n48) > 0)
  {
    do
    {
      D_001D8840_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_001604EC_EABE8);
  }
  func_00204918_EABE8(data + hdr->x10, ((u8 *) hdr) + hdr->x34);
  func_00203118_EABE8(data + hdr->x14);
  c20 = (WadClass20_EABE8 *) (((u8 *) hdr) + hdr->x1C);
  D_00160000_EABE8 = 0;
  D_0016104C_EABE8 = 0;
  D_001604CC_EABE8 = 0;
  for (k = 0; k < hdr->n18; k++)
  {
    func_00203E78_EABE8((c20->offset != 0) ? (data + c20->offset) : (0), ((u8 *) hdr) + hdr->x3C, c20->x10, c20->x4);
    c20++;
  }

  c20 = (WadClass20_EABE8 *) (((u8 *) hdr) + hdr->x24);
  for (k = 0; k < hdr->n20; k++)
  {
    func_00203F68_EABE8(data + c20->offset, ((u8 *) hdr) + hdr->x44, c20->x10, c20->x4);
    c20++;
  }

  c30 = (WadClass30_EABE8 *) (((u8 *) hdr) + hdr->x2C);
  for (k = 0; k < hdr->n28; k++)
  {
    func_00204340_EABE8(data + c30->offset, ((u8 *) hdr) + hdr->x4C, c30->x10, c30->x20, c30->x4);
    c30++;
  }

  D_0015F560_EABE8 = (s32) (data + hdr->x68);
  func_00203038_EABE8(((u8 *) hdr) + hdr->x5C, hdr->x58);
  new_var3 = data + hdr->x64;
  new_var4 = hdr->x54;
  func_00202F00_EABE8(((u8 *) hdr) + (new_var = hdr->x6C), new_var3, ((u8 *) hdr) + new_var4, hdr->x50);
  func_00122630_EABE8(li, (D_0015EF74_EABE8 << 8) >> 16, 4, 0, 0, 0, 0x100, 0x80);
  func_00118D80_EABE8(0);
  func_00122958_EABE8(li, data + hdr->x84);
  func_00120858_EABE8(0, 0);
  D_001941C0_EABE8.x18 = ((s32) D_001941C0_EABE8.hdr) + size;
  v = D_0015EF74_EABE8;
  D_0015F048_EABE8 = ((v >> 8) | 0x20010000) | (((s64) 0xB800) << 19);
  D_0015EF74_EABE8 = v + 0x20000;
  D_0015EF78_EABE8 = v + 0x20000;
  D_001941C0_EABE8.x1C = func_001E9EC8_EABE8(data + hdr->x7C);
  func_001F99B0_EABE8(&D_0018CC20_EABE8, 0, 0x1C0);
  func_001F99B0_EABE8(D_00186410_EABE8, 0, 0x40);
  D_0018CC20_EABE8.x58 = D_001941C0_EABE8.x1C;
  D_0018CC20_EABE8.x5C = (D_001941C0_EABE8.x1C += 0x40000);
  D_001941C0_EABE8.x1C += 0x40000;
  snd = (WadSound_EABE8 *) (data + hdr->x80);
  cnt = 0;
  if (snd->size != 0)
  {
    out = D_0018CC20_EABE8.x60;
    do
    {
      cnt++;
      *out = ((s32) (data + hdr->x80)) + (snd->offset + k_800);
      snd++;
      out++;
    }
    while ((cnt < 0x46) && (snd->size != 0));
  }
  func_00205220_EABE8(0);
  D_0015F060_EABE8 = D_001941C0_EABE8.x1C;
  func_002176C8_EABE8(D_001941C0_EABE8.x1C, D_00137C80_EABE8.help_text.sector, D_00137C80_EABE8.help_text.size);
  D_0015F064_EABE8 = D_0015F060_EABE8;
  D_001941C0_EABE8.x1C = D_0015F060_EABE8 + (D_00137C80_EABE8.help_text.size << 11);
  for (k = 0; k < 8; k++)
  {
    func_001EB300_EABE8(k);
    j = 0;
    if (D_001997D0_EABE8.count > 0)
    {
      te = D_0015F780_EABE8;
      adj = ((s32) te) - 8;
      {
        do
        {
          te[j].offset += adj;
          j++;
        }
        while (j < D_001997D0_EABE8.count);
      }
    }
  }

  func_001EB300_EABE8(0);
  cnt = (new_var = D_001B3E40_EABE8[0x472]);
  if (cnt >= 0)
  {
    D_001B3580_EABE8[cnt]->sound_defs = (s32)D_001862E0_EABE8;
    (*(u8 *)&D_001B3580_EABE8[cnt]->pad_D) = 5;
  }
}

extern int D_0015F064 MACRO_ADDR;
extern int D_0015F060 MACRO_ADDR;
extern int D_001997FC;
/* gp-relative, no retail symbol: gp 0x166D00 - 0x7580 = 0x15F780
   (cursor into the table walked below). */
extern short D_0015F780;

/* Points the D_0015F780 cursor at entry arg0 of the table D_0015F064
   indexes into D_0015F060, past its 8-byte header, and keeps the
   header's first word in D_001997FC. Both table pointers are MACRO_ADDR
   (one register each), and the index goes first in the addition. */
void Help_LoadMsgs(int arg0) {
    int *entry = (int *)(arg0 * 4 + D_0015F064);
    int off = *entry;
    char *p = (char *)(D_0015F060 + off);

    D_001997FC = *(int *)p;
    p += 8;
    *(char **)&D_0015F780 = p;
}

extern char D_0018CC20[];
extern float D_0018CEB0;
extern char D_00187180[];
extern void func_001F3140(void);

/* Transition_UpdateMovieCamera: the current keyframe (0x20 bytes: position,
   a flag byte at +0xC, then X/Y/Z angles) sets the camera position and
   its orientation matrix (negated first two rows); returns the flag. */
unsigned char Transition_UpdateMovieCamera(void) {
    char *t = D_0018CC20;
    char *key = *(char **)(t + 0x54) + *(int *)(t + 0x38) * 32;
    unsigned char flag = key[0xC];
    float *ang = (float *)(key + 0x10);
    char *pos;
    char *cam;
    float m[16];

    D_0018CEB0 = 0.63f;
    UpdateViewContext();
    pos = D_00187180;
    qcopy(pos, key);
    func_00125358(m);
    func_001254A0(m, m, *(float *)(key + 0x10));
    func_00125548(m, m, ang[1]);
    func_001253F8(m, m, ang[2]);
    cam = pos - 0x140;
    *(float *)(cam + 0x350) = -m[8];
    *(float *)(cam + 0x360) = -m[0];
    *(float *)(cam + 0x370) = m[4];
    *(float *)(cam + 0x354) = -m[9];
    *(float *)(cam + 0x364) = -m[1];
    *(float *)(cam + 0x374) = m[5];
    *(float *)(cam + 0x358) = -m[10];
    *(float *)(cam + 0x368) = -m[2];
    *(float *)(cam + 0x378) = m[6];
    return flag;
}


typedef struct {
    u8 pad0[0x10];
    Vec4A position;
    u8 pad20[0x30];
    u8 current_frame;
    u8 next_frame;
    u8 pad52[2];
    f32 frame_fraction;
    u8 pad58[0x19];
    u8 cached_frame;
    u8 pad72[0xD];
    u8 update_enabled;
    u8 pad80[0x26];
    s16 class_id;
} RenderSequenceActor_EB458;

typedef struct {
    u8 pad0[0x78];
    Vec4A *animation_positions;
} RenderSequenceSidecar_EB458;

typedef struct {
    u8 pad0[0x34];
    s32 time;
    s32 frame;
    s32 sequence_frame;
    s16 end_time;
    u8 pad42[2];
    s16 count;
    u8 pad46[0x132];
    RenderSequenceActor_EB458 *actors[1];
} RenderSequenceState_EB458;

extern RenderSequenceState_EB458 D_0018CC20_EB458 __asm__("D_0018CC20");
extern f32 D_0015F53C_EB458 __asm__("D_0015F53C") MACRO_ADDR;
extern s32 D_0015F050_EB458 __asm__("D_0015F050") MACRO_ADDR;
extern s32 D_0015F054_EB458 __asm__("D_0015F054") MACRO_ADDR;
extern s32 D_0015F6E8_EB458 __asm__("D_0015F6E8") MACRO_ADDR;
extern s32 D_0013CBE4_EB458[] __asm__("D_0013CBE4");
extern void func_00219E60_EB458(void) __asm__("func_00219E60");
extern void func_001E9790_EB458(RenderSequenceActor_EB458 *) __asm__("func_001E9790");
extern void func_001E97A8_EB458(void) __asm__("func_001E97A8");
extern void func_001E97B0_EB458(void) __asm__("func_001E97B0");
extern void func_001EB338_EB458(void) __asm__("func_001EB338");
extern s32 func_001F98C0_EB458(s32) __asm__("func_001F98C0");
extern void func_001F9BD8_EB458(Vec4A *, Vec4A *, Vec4A *) __asm__("func_001F9BD8");
extern void func_001F9C30_EB458(Vec4A *, Vec4A *, f32) __asm__("func_001F9C30");
extern f32 func_001F9F90_EB458(f32) __asm__("func_001F9F90");
extern f32 func_001FA888_EB458(s32) __asm__("func_001FA888");
extern void func_001FD3E8_EB458(void) __asm__("func_001FD3E8");
extern void func_00205220_EB458(s32) __asm__("func_00205220");
extern void func_0020D6D0_EB458(RenderSequenceActor_EB458 *) __asm__("func_0020D6D0");
extern void func_0020ED48_EB458(RenderSequenceActor_EB458 *) __asm__("func_0020ED48");
extern void func_0021A1A0_EB458(void) __asm__("func_0021A1A0");
extern void func_0022DD68_EB458(void) __asm__("func_0022DD68");

/* One frame of the space-flight sequence between levels: steps the sequence clock and its
   scene chunks, the movie camera, each actor's animation frame and interpolated position,
   then the intro overlay fades and the mode's own update (pause menu, freeze) and sound.
   Adapted from Lombyte (MIT) for PAL: src/textbin/fun_001eb0a8.c, update_gameplay_frame. */
void UpdateSpaceFlightSequence(void) {
    Vec4A first_position;
    Vec4A next_position;
    RenderSequenceActor_EB458 *actor;
    Vec4A *animation_positions;
    s32 actor_index;
    f32 fade;

    func_001E97B0_EB458();
    fade = D_0015F53C_EB458 - 0.0625f;
    D_0018CC20_EB458.frame++;
    D_0018CC20_EB458.time++;
    D_0015F53C_EB458 = fade;
    if (fade < 0.0f) {
        D_0015F53C_EB458 = 0.0f;
    }
    if (D_0018CC20_EB458.time >= D_0018CC20_EB458.end_time) {
        D_0018CC20_EB458.time = 0;
        D_0018CC20_EB458.sequence_frame = 0;
        func_00205220_EB458(0);
    } else if (D_0018CC20_EB458.frame >= 0x60) {
        func_00205220_EB458(++D_0018CC20_EB458.sequence_frame);
    }
    func_001EB338_EB458();
    for (actor_index = 0; actor_index < D_0018CC20_EB458.count; actor_index++) {
        u8 frame_index;
        actor = D_0018CC20_EB458.actors[actor_index];
        frame_index = D_0018CC20_EB458.frame >> 1;
        actor->current_frame = D_0018CC20_EB458.frame >> 1;
        actor->next_frame = frame_index + 1;
        func_0020D6D0_EB458(actor);
        actor->frame_fraction = func_001FA888_EB458(D_0018CC20_EB458.frame & 1) * 0.5f;
        animation_positions = ((RenderSequenceSidecar_EB458 *)actor)->animation_positions;
        func_001F9C30_EB458(&first_position, &animation_positions[actor->current_frame],
                            1.0f - actor->frame_fraction);
        func_001F9C30_EB458(&next_position, &animation_positions[actor->next_frame],
                            actor->frame_fraction);
        func_001F9BD8_EB458(&actor->position, &first_position, &next_position);
        actor->cached_frame = 0xFF;
        func_0020ED48_EB458(actor);
        actor->update_enabled = 0;
        if (actor->class_id == 0) {
            func_001E9790_EB458(actor);
        }
    }
    func_001E97A8_EB458();
    if (D_0015F6E8_EB458 == 0) {
        D_0015F058_EABE8++;
        if (func_001F98C0_EB458(0x1E) < D_0015F058_EABE8) {
            if (++D_0015F050_EB458 > 0x40) {
                D_0015F050_EB458 = 0x40;
            }
        }
        if (func_001F98C0_EB458(0x78) < D_0015F058_EABE8) {
            D_0015F054_EB458 =
                (s32)(func_001F9F90_EB458((D_0015F058_EABE8 - func_001F98C0_EB458(0x78)) % 60 * 0.10471976f +
                                          -3.1415927f) *
                      32.0f) +
                0x60;
        }
        if (D_0013CBE4_EB458[0] & 0x840) {
            func_00219E60_EB458();
        }
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 3) {
        D_0015F058_EABE8 = func_001F98C0_EB458(0x3C);
        if ((D_0015F050_EB458 -= 0x10) < 0) {
            D_0015F050_EB458 = 0;
        }
        if ((D_0015F054_EB458 -= 0x10) < 0) {
            D_0015F054_EB458 = 0;
        }
        func_0021A1A0_EB458();
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 4) {
        func_001FD3E8_EB458();
        func_0022DD68_EB458();
    }
}

struct M2c_D_0016045C
{
  u8 pad_0[0x4];
  s16 unk4;
  u8 pad_6[0x2];
};
extern u8 D_00100AE0[];
extern s32 D_0013E604[];
extern short D_0015EE88;
extern short D_0015F048;
extern s32 D_0015F050 MACRO_ADDR;
extern s32 D_0015F054 MACRO_ADDR;
extern f32 D_0015F53C MACRO_ADDR;
extern s32 D_0015F564 MACRO_ADDR;
extern s32 D_0015F6E8 MACRO_ADDR;
extern s32 D_0015F704 MACRO_ADDR;
extern struct M2c_D_0016045C * D_0016055C MACRO_ADDR;
extern s32 D_0018A3E8[];
extern u8 D_001940C0[];
extern u8 D_001D9240[];
extern u8 D_001E1600[];
extern u8 D_001E3500[];
extern void func_00235290();
extern void func_001F99B0(void *, s32, s32);
extern s32 func_00118D80();
extern void func_001E9E70_EB7C0() __asm__("func_001E9E70");
extern s32 func_001EB338_EB7C0() __asm__("func_001EB338");
extern s32 func_001F2608();
extern s32 func_001F2930();
extern void func_001F3C10();
extern s32 func_001F4630();
extern s32 func_001F4748();
extern u64 func_001F4868();
extern void func_001F4A00();
extern s32 func_001F55C0();
extern void func_001F5800();
extern s32 func_001FA898(f32);
extern s32 func_001FB530();
extern s32 func_001FB848();
extern s32 func_001FBE80();
extern void func_0020DAB0();
extern s32 func_0020DD48();
extern void func_0020E2B0();
extern void func_00218B10();
extern void func_0021A610();
extern s32 func_00229D48();
extern void func_00229E50();
extern s32 func_0022B8F8();
extern s32 func_00234620();
extern void func_002346C0();
extern s32 func_002347F0();
extern s32 func_00234AC8();
extern s32 func_00234C98_EB7C0() __asm__("func_00234C98");
extern void func_00234F40();
extern s32 func_002362B0();
extern s32 func_00236A98();
extern void func_00236BE0();
extern s32 func_00238688();
void func_001EB7C0(s32 *arg0);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/gameplay/state/transition_default_draw.c, transition_default_draw. */
void Transition_DefaultDraw(s32 *arg0)
{
    s32 n;

    if (D_0016055C == 0 || D_0016055C->unk4 != 0) {
        framebuf_appendLargeSetup();
    }
    FastMemSet(D_001940C0, -1, 0x80);
    func_001EB338_EB7C0();
    BuildViewMatrices();
    AnimSlot_ReleaseStale();
    ResetGsRegisters();
    D_0015F704 = -1;
    if (D_0016055C != 0) {
        func_001E9E70_EB7C0();
    }
    DrawTfrag();
    Vif1ChainCmd(0x02010000);
    DrawTies_1();
    Vif1ChainCmd(0x02020000);
    DrawShrubs();
    Vif1ChainCmd(0x02040000);
    if (D_0015F6E8 == 3) {
        Menu_DrawPanels();
    } else {
        DrawMobys();
    }
    Vif1ChainCmd(0x02080000);
    SetupGifPaging(0);
    Dma_AppendRefDirect3();
    if (D_0015F564 != 0) {
        ExecuteDrawCallbacks();
    }
    Dma_AppendRefDirect3();
    if (D_0018A3E8[0] != 0) {
        func_00234C98_EB7C0(8, 5);
        Dma_AppendRefDirect3();
        func_00118D80(0);
        PartProc();
        D_0015F704 = 8;
    }
    AA_BlurPass();
    ResetGsRegisters();
    if (D_0015F050 != 0) {
        DrawTexturedQuad(0xEC, 0x10, 0x100, 0x80, 0, 0, 0x100, 0x80,
                      (long)(D_0015F050 << 24 | 0x808080), (*(s64 *)&D_0015F048));
    }
    if (D_0015F054 != 0) {
        n = (*(s32 *)&D_0015EE88) - 1;
        if (n < 0) {
            n = 0;
        }
        DrawTexturedQuad(0xA0, D_0013E604[0] - 0x50, 0xC0, 0x60, 0, 0, 0x100, 0x80,
                      (long)(D_0015F054 << 24 | 0x808080), GetEffectTex(n + 4));
    }
    DoGifPaging();
    if (gFade > 0.0f) {
        if (gFade > 1.0f) {
            gFade = 1.0f;
        }
        emit_rgba_draw_packet(0, 0, 0, truncate_float_to_s32(gFade * 128.0f));
    }
    VU0_loadMicroProgram(D_00100AE0);
    func_00118D80(0);
    if (D_0015F6E8 == 4) {
        func_001FBE80();
    }
    VU1_syncChain(2);
    LightTfrags(D_001E1600);
    PatchTfragGifs();
    VU1_syncChain(4);
    LightTies(D_001E3500);
    PatchTieGifs();
    VU1_syncChain(8);
    LightShrubs(D_001D9240);
    PatchShrubGifs();
    VU1_syncChain(0x10);
    PatchMobyGifs();
    UpdateFog();
}

extern char D_0013E650[];
extern int D_0015F694;

extern int D_0015F694_m __asm__("D_0015F694") MACRO_ADDR;

/* The older register residual was the split load of D_0015F694; read
   through a MACRO_ADDR alias it is retail's one-register load. */
int IsStateEntryActive(int arg0, int arg1) {
    if (arg0 >= 0) {
        char *p = D_0013E650 + arg0 * 0x70;
        if (*(short *)(p + 0x7E) == arg1 + D_0015F694_m) {
            unsigned char s = p[0x74];
            if (s == 1 || s == 2) {
                return 1;
            }
        }
    }
    return 0;
}

typedef struct {
    s32 off;
    s32 size;
} WadChunk_BB48;

typedef struct {
    char pad0[0x20];
    WadChunk_BB48 chunk[6];
} WadHeader_BB48;

typedef struct {
    s32 sector;
    s32 size;
} DiscFile_BB48;

typedef struct {
    char pad0[0x14F8];
    DiscFile_BB48 wad_chunks[6];
} DiscTable_BB48;

typedef struct {
    s32 start;
    s32 end;
    s32 sound;
    s32 voice;
} SoundCue_BB48;

typedef struct {
    char pad0[0x34];
    s32 time;
    s32 pad38;
    s32 f3C;
} TransferState_BB48;

extern s32 current_level_BB48 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 saved_EEE8_BB48 __asm__("D_0015EEE8") MACRO_ADDR;
extern WadHeader_BB48 *volatile wad_buf_BB48[] __asm__("D_001941C8");
extern DiscTable_BB48 disc_table_BB48 __asm__("D_00137C80");
extern WadHeader_BB48 *level_root_BB48 __asm__("D_0015EF4C") MACRO_ADDR;
extern s32 space_load_pending_BB48 __asm__("D_0015F690") MACRO_ADDR;
extern s32 space_load_in_progress_BB48 __asm__("D_0015F6FC") MACRO_ADDR;
extern s32 dma_index_BB48 __asm__("D_0015EE88") MACRO_ADDR;
extern SoundCue_BB48 sound_cues_BB48[] __asm__("D_001863B0");
extern TransferState_BB48 transfer_BB48 __asm__("D_0018CC20");
extern s32 game_mode_BB48 __asm__("D_0015F6E8") MACRO_ADDR;
extern f32 sequence_BB48 __asm__("D_0015F53C") MACRO_ADDR;
extern s32 intro_alpha_BB48 __asm__("D_0015F050") MACRO_ADDR;
extern s32 language_alpha_BB48 __asm__("D_0015F054") MACRO_ADDR;
extern s32 intro_timer_BB48 __asm__("D_0015F058") MACRO_ADDR;
extern u8 requested_video_mode_BB48 __asm__("D_0016044C") MACRO_ADDR;
extern s32 pal_mode_BB48 __asm__("D_0015EE80") MACRO_ADDR;
extern s32 tex_start_BB48 __asm__("D_0015EF78") MACRO_ADDR;
extern s32 tex_cursor_BB48 __asm__("D_0015EF74") MACRO_ADDR;
extern volatile s32 tex_base_BB48 __asm__("D_0015EF8C") MACRO_ADDR;
extern s32 frame_counter_BB48 __asm__("D_0015F538") MACRO_ADDR;

extern void load_and_init_BB48(void) __asm__("func_00209DC0");
extern void transition_load_wad_BB48(void) __asm__("func_001EABE8");
extern s32 load_BB48(u8 *dst, s32 sector, s32 size) __asm__("func_002176C8");
extern void init_resource_BB48(void) __asm__("func_001FFA90");
extern void init_sif_rpc_BB48(void) __asm__("func_00233FF8");
extern void init_stash_BB48(void) __asm__("func_00234018");
extern void init_hud_BB48(void) __asm__("func_001FF6B8");
extern void load_hud_banks_BB48(void) __asm__("func_002032D0");
extern s32 sync_v_BB48(s32) __asm__("func_00122598");
extern s32 scale_frames_BB48(s32) __asm__("func_001F98C0");
extern s32 snd_reset_BB48(void) __asm__("func_0012EC40");
extern s32 snd_flush_BB48(void) __asm__("func_0012DDC0");
extern void func_0012EC30_BB48(void) __asm__("func_0012EC30");
extern void vu1_send_chain_BB48(void) __asm__("func_002349B8");
extern void swap_chain_BB48(void) __asm__("func_00234948");
extern void put_draw_small_BB48(void) __asm__("func_001FB598");
extern void append_palette_BB48(void) __asm__("func_001FB8A8");
extern void put_draw_large_BB48(void) __asm__("func_001FB498");
extern s32 memcard_update_BB48(void) __asm__("func_00209E68");
extern s32 run_state_handler_BB48(void) __asm__("func_00209070");
extern void update_pad_BB48(void) __asm__("func_00218908");
extern void queue_dma_BB48(s32) __asm__("func_001EB300");
extern s32 is_active_BB48(s32, s32) __asm__("func_001EBAF0");
extern s32 alloc_voice_BB48(s32, s32, s32) __asm__("func_0022EEB8");
extern void release_voice_BB48(s32) __asm__("func_0022EAB0");
extern void update_gameplay_BB48(void) __asm__("func_001EB458");
extern void default_draw_BB48(void) __asm__("func_001EB7C0");
extern void start_level_BB48(s32) __asm__("func_001E9808");
extern void parse_space_scene_BB48(s32) __asm__("func_00205220");
extern void vu1_init_chain_BB48(void) __asm__("func_002348E8");
extern void fade_to_black_BB48(s32) __asm__("func_001F4E08");
extern s32 reset_vif1_gif_BB48(void) __asm__("func_001207B8");
extern void reset_graph_BB48(s16, s16, s16, s16) __asm__("func_00121B78");
extern void set_pal_mode_BB48(void) __asm__("func_001F3890");
extern void vu1_sync_chain_BB48(s32) __asm__("func_00234AC8");

/* Adapted from Lombyte (MIT): src/gameplay/state/transition_do_transition.c, transition_do_transition. */
void func_001EBB48(void) {
    WadHeader_BB48 *hdr;
    u8 *p;
    s32 n;
    s32 level;
    s32 wait;
    s32 i;
    s32 old;
    s32 saved;
    s32 first_size;
    u32 alignment_mask = 0xFFFFFFF0;
    DiscTable_BB48 *tbl;

    current_level_BB48 = 0;
    *(volatile u32 *)0x10000010 = 0x83;
    *(volatile u32 *)0x10000000 = 0;
    saved = saved_EEE8_BB48;
    load_and_init_BB48();
    saved_EEE8_BB48 = saved;
    transition_load_wad_BB48();
    tbl = &disc_table_BB48;
    p = (u8 *)wad_buf_BB48[0];
    hdr = (WadHeader_BB48 *)p;
    p = (u8 *)hdr + 0x60;
    level_root_BB48 = hdr;
    first_size = load_BB48(p, ((volatile DiscFile_BB48 *)&tbl->wad_chunks[1])->sector, ((volatile DiscFile_BB48 *)&tbl->wad_chunks[1])->size);
    hdr->chunk[1].size = first_size;
    hdr->chunk[1].off = p - (u8 *)hdr;
    p += ((u32)first_size + 0xF) & alignment_mask;
    hdr->chunk[2].size = load_BB48(p, tbl->wad_chunks[2].sector, tbl->wad_chunks[2].size);
    hdr->chunk[2].off = p - (u8 *)hdr;
    p += ((u32)hdr->chunk[2].size + 0xF) & alignment_mask;
    hdr->chunk[3].size = load_BB48(p, tbl->wad_chunks[3].sector, tbl->wad_chunks[3].size);
    hdr->chunk[3].off = p - (u8 *)hdr;
    p += ((u32)hdr->chunk[3].size + 0xF) & alignment_mask;
    hdr->chunk[4].size = load_BB48(p, tbl->wad_chunks[4].sector, tbl->wad_chunks[4].size);
    hdr->chunk[4].off = p - (u8 *)hdr;
    p += ((u32)hdr->chunk[4].size + 0xF) & alignment_mask;
    hdr->chunk[5].size = load_BB48(p, tbl->wad_chunks[5].sector, tbl->wad_chunks[5].size);
    hdr->chunk[5].off = p - (u8 *)hdr;
    p += ((u32)hdr->chunk[5].size + 0xF) & alignment_mask;
    hdr->chunk[0].size = load_BB48(p, tbl->wad_chunks[0].sector, tbl->wad_chunks[0].size);
    hdr->chunk[0].off = p - (u8 *)hdr;
    init_resource_BB48();
    init_sif_rpc_BB48();
    init_stash_BB48();
    init_hud_BB48();
    load_hud_banks_BB48();
    for (n = *(volatile u32 *)0x10000000 / 0x109; n < scale_frames_BB48(0xB4); n++) {
        sync_v_BB48(0);
    }
    snd_reset_BB48();
    while (snd_flush_BB48() != 0) {
    }
    func_0012EC30_BB48();
    level = 1;
    wait = 0;
    while (space_load_pending_BB48 == 0) {
        vu1_send_chain_BB48();
        swap_chain_BB48();
        put_draw_small_BB48();
        append_palette_BB48();
        put_draw_large_BB48();
        memcard_update_BB48();
        run_state_handler_BB48();
        update_pad_BB48();
        queue_dma_BB48(dma_index_BB48);
        space_load_in_progress_BB48 = 0;
        for (i = 0; sound_cues_BB48[i].start != -1; i++) {
            if (sound_cues_BB48[i].end == -1) {
                if (transfer_BB48.time < sound_cues_BB48[i].start) {
                    sound_cues_BB48[i].voice = -1;
                    continue;
                }
                if (sound_cues_BB48[i].voice != -1) {
                    continue;
                }
            } else if (transfer_BB48.time < sound_cues_BB48[i].start ||
                       sound_cues_BB48[i].end < transfer_BB48.time) {
                goto stop;
            } else if (is_active_BB48(sound_cues_BB48[i].voice, sound_cues_BB48[i].sound) != 0) {
                continue;
            }
            sound_cues_BB48[i].voice = alloc_voice_BB48(sound_cues_BB48[i].sound, 0, 0);
            continue;
        stop:
            if (is_active_BB48(sound_cues_BB48[i].voice, sound_cues_BB48[i].sound) != 0) {
                release_voice_BB48(sound_cues_BB48[i].voice);
            }
            sound_cues_BB48[i].voice = -1;
        }
        update_gameplay_BB48();
        default_draw_BB48();
        if (game_mode_BB48 != 0) {
            wait = 0;
        } else if (++wait >= scale_frames_BB48(0x5DC)) {
            start_level_BB48(level);
            wait = 0;
            level = (level + 1) % 4;
            if (level == 0) {
                level++;
            }
            sequence_BB48 = 1.0f;
            transfer_BB48.time = 0;
            transfer_BB48.f3C = 0;
            parse_space_scene_BB48(0);
            intro_alpha_BB48 = 0;
            language_alpha_BB48 = 0;
            intro_timer_BB48 = 0;
            vu1_init_chain_BB48();
            continue;
        }
        if (requested_video_mode_BB48 != pal_mode_BB48) {
            pal_mode_BB48 = pal_mode_BB48 == 0;
            fade_to_black_BB48(4);
            reset_vif1_gif_BB48();
            reset_graph_BB48(0, 1, pal_mode_BB48 != 0 ? 3 : 2, 0);
            old = tex_start_BB48;
            sequence_BB48 = 1.0f;
            set_pal_mode_BB48();
            tex_start_BB48 = old;
        }
        vu1_sync_chain_BB48(1);
        sync_v_BB48(0);
        frame_counter_BB48++;
    }
    if (pal_mode_BB48 == 0) {
        tex_base_BB48 = 0x280000;
    }
    tex_cursor_BB48 = tex_start_BB48 = tex_base_BB48;
}

/* Not a standalone function: single `addiu $sp,$sp,0x30`, no `jr $31` --
   fallthrough fragment, same category as func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC030);
