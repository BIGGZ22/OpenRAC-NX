#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * tfragfunc.cpp in the original source; text 0x234380-0x2347F0.
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
extern void func_0012F068(void *);
extern void func_002177F0(int);
extern short D_001517D0[];
extern void func_0012EDE0(void *);
extern void func_0012EFE8(void);
extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);
extern int func_0012F030(void);
extern void func_002167C0(short, short, short);
extern void func_002169B8(short, short, short);
extern short D_001517F0 NOT_SDA;
extern char D_0013CA40[];
extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);
extern void func_00217F68(void *);
extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];
extern char D_001D5F70[] NOT_SDA;
extern char D_001D603B[];
extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);
extern int D_00141FA0[];
extern char D_001D0A50[];
extern char D_001D0A88[];
extern int D_001A0418 NOT_SDA;
extern float func_001FA748(float, float);
extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");
extern int func_002267C0(int);
extern void func_00234C98(int, int);
extern void func_00205E70(void);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern short D_001602B0;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;
extern int func_00226EA8(int);
extern int func_00226F68(int);
extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);
typedef struct {
    int key;
    int flags;
} PadBind;
extern PadBind D_001D6448_t[] __asm__("D_001D6448");
extern int func_00227018(int handle);
extern int D_001D6448[];
extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;
extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];
extern char D_00187180_a[] __asm__("D_00187180");
extern char D_00194220[];
extern int D_0013E6BC;
extern void func_002141A8(void *, float, float);
extern void func_001F9BD8_a(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_a(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BF0_a(void *, void *, void *) __asm__("func_001F9BF0");
extern int func_001EFE10_a(void *, void *, int, int, int) __asm__("func_001EFE10");
extern char D_00187180[];
extern void func_001F9BF0(void *, void *, void *);
extern void func_001EFE10(void *, void *, int, int, int);
extern float func_001F9D10(int, void *);
extern void func_0022DA10(void *, float, float, float);
extern void func_001F9EE8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_001FA058_a(float, float) __asm__("func_001FA058");
extern void func_001FA898(float);
extern void func_00120F30(int);
extern int func_0012E060(void *, int);
extern void func_0012EE70(int);
extern void func_0012EF48(int);
extern void func_0012E2E8(void);
extern void func_0011AE20(int arg0);
typedef struct {
    int unk_00;
    int unk_04;
    int unk_08;
    int unk_0C;
} Rec10;
extern Rec10 D_001DD568[];
extern int func_00234350(unsigned int arg0);

/*
 * Retail aligns func_00234380 to 16 bytes, and func_00234350's .s file
 * carried two trailing nops after its endlabel to supply that padding.
 * Those nops sit OUTSIDE the declared function size (0x28), so they were
 * silently lost when this function became C, shifting all 179 later
 * text functions by -8 and giving them spurious 1-byte jal-target diffs.
 * The .s files only ever .align 3 (8 bytes), so the 16-byte alignment
 * has to be restored explicitly here.
 */
__asm__(".align 4");

extern float D_00160FA0[3];
extern int D_00160FB0[3];
extern float D_0018D020;
extern float D_001DEB70[4][4];
extern int func_001FA898_i(float) __asm__("func_001FA898");

/* SetTfragDists(void): the three LOD distances as fixed point (x1024), and
   the matrix mapping distance to the two blend weights. The store order is
   load-bearing: filling the rows first and the two translation terms last
   ([1][3] = b, then [0][3] = a) moves sched1's last use of b, and with it
   the local-alloc priority of b's quantity below ab's -- retail's b in $f21,
   1/(a-b) in $f20, and -0.5 in $f3. */
void SetTfragDists(void) {
    float a, b, c, ab, bc;

    D_00160FB0[0] = func_001FA898_i(D_00160FA0[0] * 1024.0f);
    D_00160FB0[1] = func_001FA898_i(D_00160FA0[1] * 1024.0f);
    D_00160FB0[2] = func_001FA898_i(D_00160FA0[2] * 1024.0f);
    a = D_00160FA0[0] * D_0018D020;
    b = D_00160FA0[1] * D_0018D020;
    c = D_00160FA0[2] * D_0018D020;
    ab = 1.0f / (a - b);
    bc = 1.0f / (b - c);
    FastMemZero16(D_001DEB70, 0x40);
    D_001DEB70[0][0] = ab * 0.5f;
    D_001DEB70[0][1] = -ab;
    D_001DEB70[1][0] = bc * 0.5f;
    D_001DEB70[1][1] = -bc;
    D_001DEB70[2][0] = b * ab * -0.5f;
    D_001DEB70[2][1] = a * ab;
    D_001DEB70[3][0] = c * bc * -0.5f;
    D_001DEB70[3][1] = b * bc;
    D_001DEB70[1][3] = b;
    D_001DEB70[0][3] = a;
}

extern int D_00161000 MACRO_ADDR;
extern int D_00160FBC MACRO_ADDR;
extern int D_00160FC4 MACRO_ADDR;
extern int D_0015EF74 MACRO_ADDR;
extern char D_001E8CE0[];
extern void func_00235EF0(void);
extern int func_00236060(int);
extern void func_00234E80(void);

/* DmaTfragTextures: the tfrag twin of DmaShrubTextures (func_00229C08),
   which also warns "tfrag texture overflow" past 0x400000 bytes and keeps
   the largest size seen in D_00160FC4. */
/* A copy of its twin func_00229C08. */
void DmaTfragTextures(void) {
    int *p = (int *)D_00161000;
    int size;

    D_00161000 += 0x10;
    ((int *)D_00160FBC)[0] = 0x20000000;
    ((int *)D_00160FBC)[1] = D_00161000;
    ((int *)D_00160FBC)[2] = 0;
    ((int *)D_00160FBC)[3] = 0;
    if (D_0018A3B0[4] != 0 && D_0018A3B0[3] != 0) {
        ComputeTfragTextureUsage();
        size = func_00236060(D_0015EF74);
        VU1_texFlush();
        if (size > 0x400000) {
            STUB_printf(D_001E8CE0);
        }
        if (D_00160FC4 < size) {
            D_00160FC4 = size;
        }
    }
    ((int *)D_00161000)[0] = 0x20000000;
    ((int *)D_00161000)[1] = D_00160FBC + 0x10;
    ((int *)D_00161000)[2] = 0;
    ((int *)D_00161000)[3] = 0;
    D_00161000 += 0x10;
    p[0] = 0x20000000;
    p[1] = D_00161000;
    p[2] = 0;
    p[3] = 0;
}

typedef struct {
    short a;
    short b;
} TexRemap;

typedef struct {
    char *items;
    int count;
} TexChunk;

extern TexChunk D_001E1200[];
extern TexRemap D_001E0F00[];

/* Walk the null-terminated chunk list at D_001E1200; for every 0x50-byte
   record in each chunk, look up the remap entry indexed by the byte at
   +0x23 and patch its two nonzero halves into the low 14 bits of the
   words at +0x00 and +0x30. */
/* PatchTfragGifs(void) */
void PatchTfragGifs(void) {
    TexChunk *c;
    char *r;
    TexRemap *e;
    int i;
    int n;

    c = D_001E1200;
    while (c->items != 0) {
        n = c->count;
        r = c->items;
        for (i = 0; i < n; i++) {
            e = &D_001E0F00[*(unsigned char *)(r + 0x23)];
            if (e->a != 0) {
                *(int *)r = (*(int *)r & 0xFFFFC000) | e->a;
            }
            if (e->b != 0) {
                *(int *)(r + 0x30) = (*(int *)(r + 0x30) & 0xFFFFC000) | e->b;
            }
            r += 0x50;
        }
        c++;
    }
}

extern int D_0015EF78 MACRO_ADDR;
extern char D_00160F70[];
extern char D_00160F80[];
extern char D_001E1600[];
extern void func_001F2560(void *, int); /* empty profiling marker */
extern void func_001F2558(void *, int); /* empty profiling marker */
extern void func_001FA190(void *);
extern void func_00234BA0(int, void *, int);
extern void func_002352C8(void);
extern void func_002344D8(void);

/* DrawTfrag, in DrawShrubs' shape. */
void DrawTfrag(void) {
    float m[4][4];
    int p = D_00161000;

    D_00160FBC = p;
    D_0015EF74 = D_0015EF78;
    p += 0x10;
    D_00161000 = p;
    ProfileMarker_Begin(D_00160F70, 1);
    func_001FA190(m);
    FastVecScale(m[3], D_00187180, -1024.0f);
    m[3][3] = 1.0f;
    sce_vu0_mul_matrix(m, D_00187180 - 0x100, m);
    write_vif_unpack_packet(5, m, 4);
    write_vif_unpack_packet(0x14D, m, 4);
    if (D_0018A3B0[4] != 0) {
        func_00118D80(0);
        TfragProc();
    }
    ProfileMarker_Begin(D_00160F80, 2);
    DmaTfragTextures();
    if (D_0018A3B0[4] != 0) {
        write_dma_channel(D_001E1600, 0x3000, 0x40);
    }
    ProfileMarker_End(D_00160F80, 2);
}

/* The last function in the object: retail pads to 16 bytes after it
   (two nops in its .s), and vuchain.o starts there. */
__asm__(".align 4");
