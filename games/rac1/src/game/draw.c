#include "common.h"
#include "vector.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * draw.cpp in the original source; text 0x1F0F30-0x1F7C60.
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

/* Resets the texture DMA state words to 1. */
void draw_resetTextureDmaState(void) {
    int *p = D_0018A3B0;
    int val = 1;
    int i = 0x13;
    p = (int *)((char *)p + 0x4C);
    for (; i >= 0; i--, p--) {
        *p = val;
    }
}

LINKER_REMNANT("asm/remnants/text", func_001F0F70);

typedef struct {
    int x;
    int y;
    int color;
    char *str;
} DrawTextRec;
extern DrawTextRec D_0018AC00[];
extern short D_0015F100;
extern short D_0015F104;
extern char D_0015F108[];
extern int func_00116248();

/* Queues one text item: D_0018AC00[n] = {x, y, colour, pool position},
   then sprintf(pool, "%s", str) (D_0015F108 is "%s") advances the
   D_0015F100 string pool past the copy. Indexing the table at every
   store gives retail's two addu forms; a `DrawTextRec *` local folds
   them into one register and comes out 12 bytes short. */
void Text_Queue(int x, int y, int color, char *str) {
    int n = *(int *)&D_0015F104;

    D_0018AC00[n].x = x;
    D_0018AC00[n].y = y;
    D_0018AC00[n].color = color;
    D_0018AC00[n].str = *(char **)&D_0015F100;
    *(int *)&D_0015F104 = n + 1;
    *(char **)&D_0015F100 += func_00116248(*(char **)&D_0015F100, D_0015F108, str) + 1;
}

LINKER_REMNANT("asm/remnants/text", func_001F0FF0);

extern int D_00189EC0[];

/* Draws `str` centred on x: sums the per-character widths in D_00189EC0
   (indexed by char - 0x20, anything past the table using entry 0x20),
   moves x left by half the total and queues the text with
   func_001F0F78. Returns the adjusted x. `idx = ch` followed by the
   out-of-range override gives retail's sltiu 0x60 + movn; the reverse
   (default first, then `if (ch < 0x60)`) becomes sltu + movz. */
int Text_QueueCentered(int x, int y, int color, char *str) {
    int w = 0;
    unsigned char *p = (unsigned char *)str;

    while (*p != 0) {
        unsigned char ch = *p++ - 0x20;
        int idx = ch;
        if (ch >= 0x60) {
            idx = 0x20;
        }
        w += D_00189EC0[idx];
    }
    x -= w >> 1;
    Text_Queue(x, y, color, str);
    return x;
}

__asm__(".section .text\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_001F1088);

LINKER_REMNANT("asm/remnants/text", func_001F10A0);

INCLUDE_ASM("asm/nonmatchings/text", func_001F10C8);

LINKER_REMNANT("asm/remnants/text", func_001F2410);

extern void func_001FA190(void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9EE8(void *, void *, void *);
extern float D_0018D010;

typedef struct {
    char pad0[0x40];
    float viewMtx[4][4];   /* +0x40 */
    char pad1[0xC0];
    float focus[3];        /* +0x140 */
} CameraBlock;
/* The camera block as a struct: all four accesses then share one base
   register with field offsets, as in retail. */
extern CameraBlock D_00187040_cam __asm__("D_00187040");

typedef struct {
    float x, y, z;
} Vec3f;

/* Projects camera-space point a1 through the camera matrix, divides by
   depth (D_0018D010 / w) and scales to x16 screen units. func_002346C0
   in tfragfunc.c builds the same matrix. */
void projectWorldPoint(Vec3f *a0, float *a1) {
    float m[4][4];
    float m2[4][4];
    float v[4];
    float out[4];
    float invw;

    func_001FA190(m);
    m[3][0] = -D_00187040_cam.focus[0] * 1024.0f;
    m[3][1] = -D_00187040_cam.focus[1] * 1024.0f;
    m[3][2] = -D_00187040_cam.focus[2] * 1024.0f;
    sce_vu0_mul_matrix(m2, D_00187040_cam.viewMtx, m);

    FastVecScale(v, a1, 1024.0f);
    v[3] = 1.0f;
    func_001F9EE8(out, v, m2);

    invw = D_0018D010 / out[3];
    a0->z = out[2] * 0.0009765625f;
    out[0] = out[0] * invw + 2048.0f;
    a0->x = out[0] * 16.0f;
    out[1] = out[1] * invw + 2048.0f;
    a0->y = out[1] * 16.0f;
}

LINKER_REMNANT("asm/remnants/text", func_001F2550);

/* Empty stub (returns immediately). */
void ProfileMarker_End(void) {
}

/* Empty stub (returns immediately). */
void ProfileMarker_Begin(void) {
}

extern int D_0015F6FC_m __asm__("D_0015F6FC") MACRO_ADDR;
extern int D_0015EE80 MACRO_ADDR;
extern int D_0015EF78 MACRO_ADDR;
extern int *D_00161000 MACRO_ADDR;

extern void func_002350A8(void);
extern int func_00123308(int);
extern void func_0020C268(void);
extern void func_00121B78(int, int, int, int);
extern void func_001207B8(void);
extern void func_002348E8(void);
extern void func_001F3890(void);
extern void func_00235018(void);
extern volatile int D_00160FE0_v __asm__("D_00160FE0") MACRO_ADDR;

/* Render setup: raises D_0015F6FC, clears D_00160FE0, runs the setup calls,
   picks func_00121B78's mode from D_0015EE80 (retail's movz), then runs
   func_001F3890 with D_0015EF78 saved around it and D_00161000 cleared.
   The D_00160FE0 flag is volatile: retail keeps its store out of
   func_002350A8's delay slot, where the plain MACRO_ADDR store would go.
   The two stores that do sit in delay slots (D_00161000, D_0015EF78) are
   MACRO_ADDR and come out $gp-relative there. */
void ResetVideoPipeline(void) {
    int flag;
    int pal;

    D_0015F6FC_m = 1;
    D_00160FE0_v = 0;
    DMAC_VIF1_Disable();
    func_00123308(1);
    InitDma();
    flag = D_0015EE80;
    func_00121B78(0, 1, flag ? 3 : 2, 0);
    func_001207B8();
    VU1_initChain();
    pal = D_0015EF78;
    D_00161000 = 0;
    SetPalMode();
    D_0015EF78 = pal;
    VU1_initChain();
    DMAC_VIF1_Enable();
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern void func_001F9C48(void *, void *, float);
extern int D_0018C42C;
extern char D_00187390[];
extern char D_0018CEC0[];
extern char D_0018D080[];

typedef struct {
    float f[0x50];
} SceneG;
extern SceneG D_00187040_g __asm__("D_00187040");

/* Builds the view matrices in the camera block from the rotation set up in D_00187390 or the three rotation angles, then copies the result to D_0018D080. Adapted from Lombyte (MIT) for PAL: src/rendering/view/fun_001f2260.c, FUN_001f2260. */
void BuildViewMatrices(void) {
    float m[4][4];
    SceneG *g;
    char *x;
    float *xf;
    float a;
    float b;
    float c;

    if (D_0018C42C == 0) {
        qcopy(m[0], D_00187390);
        qcopy(m[1], D_00187390 + 0x10);
        qcopy(m[2], D_00187390 + 0x20);
    } else {
        func_00125358(m[0]);
        func_001254A0(m[0], m[0], D_00187040_g.f[0x150 / 4]);
        func_00125548(m[0], m[0], D_00187040_g.f[0x154 / 4]);
        func_001253F8(m[0], m[0], D_00187040_g.f[0x158 / 4]);
    }
    g = &D_00187040_g;
    x = D_0018CEC0;
    xf = (float *)(x - 0xC0);
    g->f[0] = -m[1][0];
    g->f[0x3C / 4] = 1.0f;
    *(int *)&g->f[0x30 / 4] = 0;
    g->f[0x10 / 4] = -m[1][1];
    g->f[0x20 / 4] = -m[1][2];
    g->f[4 / 4] = -m[2][0];
    g->f[0x14 / 4] = -m[2][1];
    g->f[0x24 / 4] = -m[2][2];
    g->f[8 / 4] = m[0][0];
    g->f[0x18 / 4] = m[0][1];
    g->f[0x28 / 4] = m[0][2];
    *(int *)&g->f[0x34 / 4] = 0;
    *(int *)&g->f[0x38 / 4] = 0;
    *(int *)&g->f[0xC / 4] = 0;
    *(int *)&g->f[0x1C / 4] = 0;
    *(int *)&g->f[0x2C / 4] = 0;
    sce_vu0_mul_matrix(&g->f[0x40 / 4], x, g);
    sce_vu0_mul_matrix(&g->f[0x80 / 4], x + 0x40, g);
    a = xf[0x1A0 / 4];
    b = xf[0x1A4 / 4];
    c = xf[0x1A8 / 4];
    g->f[0x80 / 4] += g->f[0x8C / 4] * a;
    g->f[0x84 / 4] += g->f[0x8C / 4] * b;
    g->f[0x88 / 4] += g->f[0x8C / 4] * c;
    g->f[0x90 / 4] += g->f[0x9C / 4] * a;
    g->f[0x94 / 4] += g->f[0x9C / 4] * b;
    g->f[0x98 / 4] += g->f[0x9C / 4] * c;
    g->f[0xA0 / 4] += g->f[0xAC / 4] * a;
    g->f[0xA4 / 4] += g->f[0xAC / 4] * b;
    g->f[0xA8 / 4] += g->f[0xAC / 4] * c;
    g->f[0xB0 / 4] += g->f[0xBC / 4] * a;
    g->f[0xB4 / 4] += g->f[0xBC / 4] * b;
    g->f[0xB8 / 4] += g->f[0xBC / 4] * c;
    sce_vu0_mul_matrix(&g->f[0xC0 / 4], x + 0x80, g);
    func_001F9C48(&g->f[0x100 / 4], x + 0x80, xf[0x1C0 / 4]);
    func_001F9C48(&g->f[0x110 / 4], x + 0x90, xf[0x1C0 / 4]);
    qcopy(&g->f[0x120 / 4], x + 0xA0);
    qcopy(&g->f[0x130 / 4], x + 0xB0);
    sce_vu0_mul_matrix(&g->f[0x100 / 4], &g->f[0x100 / 4], g);
    qcopy(D_0018D080, g);
    qcopy(D_0018D080 + 0x10, &g->f[0x10 / 4]);
    qcopy(D_0018D080 + 0x20, &g->f[0x20 / 4]);
    FastVecScale(D_0018D080 + 0x30, &g->f[0x140 / 4], 1024.0f);
    *(float *)(D_0018D080 + 0x3C) = 1024.0f;
}

/* A fog preset: an RGB byte triple and four floats. */
typedef struct {
    unsigned char r, g, b, pad;
    float f[4];
} FogPreset;
extern int D_001873D4;
extern FogPreset D_001611C4 MACRO_ADDR;
extern FogPreset D_0015F584 MACRO_ADDR;
extern int D_0018CE00[];
extern int D_001601BC MACRO_ADDR;
extern int D_0015F598 MACRO_ADDR;
extern void func_001F3140(void);

/* UpdateFog(int): copies the fog preset (the fixed D_001611C4 when
   D_001873D4 is set, else the current D_0015F584) into the draw context
   at D_0018CE00+0x218, sets the mode word D_001601BC (0x40000 or
   0x1F4000), runs UpdateViewContext (func_001F3140) and clears
   D_0015F598. The presets and the two words it writes are MACRO_ADDR:
   retail reads each field with the one-register macro, and the mode
   word's store fits the branch delay slot ($gp-relative there). */
void UpdateFog(int arg0) {
    if (D_001873D4 != 0) {
        D_0018CE00[0x8C] = D_001611C4.r;
        D_0018CE00[0x8D] = D_001611C4.g;
        D_0018CE00[0x8E] = D_001611C4.b;
        *(float *)&D_0018CE00[0x86] = D_001611C4.f[0];
        *(float *)&D_0018CE00[0x87] = D_001611C4.f[1];
        *(float *)&D_0018CE00[0x8A] = D_001611C4.f[2];
        *(float *)&D_0018CE00[0x8B] = D_001611C4.f[3];
        D_001601BC = 0x40000;
    } else {
        D_0018CE00[0x8C] = D_0015F584.r;
        D_0018CE00[0x8D] = D_0015F584.g;
        D_0018CE00[0x8E] = D_0015F584.b;
        *(float *)&D_0018CE00[0x86] = D_0015F584.f[0];
        *(float *)&D_0018CE00[0x87] = D_0015F584.f[1];
        *(float *)&D_0018CE00[0x8A] = D_0015F584.f[2];
        *(float *)&D_0018CE00[0x8B] = D_0015F584.f[3];
        D_001601BC = 0x1F4000;
    }
    UpdateViewContext();
    D_0015F598 = 0;
}
__asm__(".section .text\n\tnop\n");

extern char *D_0015F720 MACRO_ADDR;

/* ParseOcclGrid(x, y, z): walks the three-level occlusion grid at
   D_0015F720. Each level is {u16 start, u16 count, u16 entry[count]};
   the coordinate minus start must fall in [0, count), and its entry is
   the next level's offset in words from the grid (levels 1 and 2, 0 =
   empty) or, at the last level, a 128-byte cell index from the root
   (grid + grid[0]), 0xFFFF = empty. Returns the cell or 0. Each level's
   coordinate goes in its own local: that keeps y and x in their argument
   registers and the level pointer in $a3. The last level's bounds share
   one `if`, which is what leaves retail's shared failure return after
   level 2 and a separate one for the 0xFFFF test. */
int ParseOcclGrid(int x, int y, int z) {
    char *grid = D_0015F720;
    char *base = grid + *(int *)grid;
    unsigned short *l = (unsigned short *)(grid + 4);
    int cz, cy, cx;

    cz = z - l[0];
    if (cz < 0) return 0;
    if (cz >= l[1]) return 0;
    if (l[cz + 2] == 0) return 0;
    l = (unsigned short *)(grid + l[cz + 2] * 4);
    cy = y - l[0];
    if (cy < 0) return 0;
    if (cy >= l[1]) return 0;
    if (l[cy + 2] == 0) return 0;
    l = (unsigned short *)(grid + l[cy + 2] * 4);
    cx = x - l[0];
    if (cx < 0 || cx >= l[1]) return 0;
    if (l[cx + 2] == 0xFFFF) return 0;
    return (int)(base + l[cx + 2] * 128);
}

__asm__(".section .text\n\tnop\n");

extern int func_001F2A38(int, int, int);

/* GetOcclGridFromPair(int, int, int, int, int, int, float) */
int GetOcclGridFromPair(int a0, int a1, int a2, int a3, int a4, int a5, float t) {
    int r;
    int b0;
    int b1;
    int b2;

    if (t < 0.5f) {
        r = ParseOcclGrid(a0, a1, a2);
        if (r != 0) {
            return r;
        }
        b0 = a3;
        b1 = a4;
        b2 = a5;
    } else {
        r = ParseOcclGrid(a3, a4, a5);
        if (r != 0) {
            return r;
        }
        b0 = a0;
        b1 = a1;
        b2 = a2;
    }
    return ParseOcclGrid(b0, b1, b2);
}

extern int func_001FA898_r(float) __asm__("func_001FA898");
extern float func_001FA888(int);
extern int func_001F2B10(int, int, int, int, int, int, float);
extern void func_001F9AC0(void *, void *, void *, int);
extern void func_001F99B0();
extern char D_001940C0[];
extern char D_00194140[];
extern int D_0015F72C MACRO_ADDR;
extern int D_0015F728 MACRO_ADDR;
extern char *D_0015F730 MACRO_ADDR;
extern float *D_0015F724 MACRO_ADDR;

/* Build the occlusion visibility bitmap for the camera's grid cell.
   Adapted from Lombyte (MIT) for PAL: src/rendering/build_occlusion_visibility.c, build_occlusion_visibility. */
void BuildOcclVisibility(void) {
    float scale = 0.25f;
    int x, y, z;
    char *vis;
    char *gx, *gy, *gz;
    float *p;
    int bx, by, bz;

    x = func_001FA898_r(D_00187040_cam.focus[0] * scale);
    y = func_001FA898_r(D_00187040_cam.focus[1] * scale);
    z = func_001FA898_r(D_00187040_cam.focus[2] * scale);
    vis = (char *)ParseOcclGrid(x, y, z);
    if (vis != 0) {
        D_0015F72C = 0;
        FastMemCopy(D_001940C0, vis, 0x80);
        D_0015F730 = vis;
    } else {
        D_0015F72C = 1;
        if (D_0015F728 == 0) {
            gx = (char *)GetOcclGridFromPair(x - 1, y, z, x + 1, y, z,
                                       D_00187040_cam.focus[0] * scale - IntToFloat(x));
            gy = (char *)GetOcclGridFromPair(x, y - 1, z, x, y + 1, z,
                                       D_00187040_cam.focus[1] * scale - IntToFloat(y));
            gz = (char *)GetOcclGridFromPair(x, y, z - 1, x, y, z + 1,
                                       D_00187040_cam.focus[2] * scale - IntToFloat(z));
            if (gx != 0 || gy != 0 || gz != 0) {
                FastMemZero16(D_00194140, 0x80);
                if (gx != 0) {
                    FastMemOr16(D_00194140, D_00194140, gx, 0x80);
                }
                if (gy != 0) {
                    FastMemOr16(D_00194140, D_00194140, gy, 0x80);
                }
                if (gz != 0) {
                    FastMemOr16(D_00194140, D_00194140, gz, 0x80);
                }
                vis = D_00194140;
                D_0015F730 = vis;
                FastMemCopy(D_001940C0, vis, 0x80);
            }
        }
        if (vis == 0) {
            switch (D_0015F728) {
            case 0:
                if (D_0018C42C == 0 && D_0015F730 != 0) {
                    FastMemCopy(D_001940C0, D_0015F730, 0x80);
                } else {
                    FastMemSet(D_001940C0, -1, 0x80);
                }
                break;
            case 1:
                FastMemSet(D_001940C0, -1, 0x80);
                break;
            case 2:
                p = D_0015F724;
                if (p != 0) {
                    bx = 0.0f < D_00187040_cam.focus[0] - p[0];
                    by = 0.0f < D_00187040_cam.focus[1] - p[1];
                    bz = 0.0f < D_00187040_cam.focus[2] - p[2];
                    FastMemCopy(D_001940C0, (char *)p + ((bz + by * 2 + bx * 4) * 0x80 + 0x10), 0x80);
                } else if (D_0018C42C == 0 && D_0015F730 != 0) {
                    FastMemCopy(D_001940C0, D_0015F730, 0x80);
                } else {
                    FastMemSet(D_001940C0, -1, 0x80);
                }
                break;
            }
        }
    }
    {
        unsigned char *vb = (unsigned char *)D_001940C0;
        vb[0x7F] |= 0x80;
    }
}

/* Unprototyped deliberately: two call sites need incompatible arg1
   types (-1 and a pointer) and both callers are byte-exact, so
   neither may be edited. Codegen is identical either way -- int and
   pointer are both 32-bit in the same arg register. */
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;

/* UpdateOcclusion(void) */
void UpdateOcclusion(void) {
    int state = D_0018C434;
    if (state == 0) {
        FastMemSet(D_001940C0, -1, 0x80);
    } else if (state == 2) {
        BuildOcclVisibility();
    }
}

extern short D_00151880[];
extern int D_0013E600[];
extern float func_001FA888(int);

/* InitViewContext: the same set-up as SetScreenSize (func_001F3760) for
   the display's size, D_00151880[0xA8]/[0xA9], with fixed extras. The
   GS viewport record D_0013E600 gets width, height, their halves and the
   four <<4 edges around the 0x800 centre. The draw context D_0018CE00
   gets 32, 745472 and 0.63 at +0xA0/+0xA4/+0xB0, the half extents as
   floats (func_001FA888 is int to float) at +0x200/+0x204, four times
   each at +0x208/+0x20C, and 0, 524288, 255, 0 at +0x218/+0x21C/+0x228/
   +0x22C. The size is read into `short` locals (lhu, then sll/sra) through
   a base pointer that stays in $s1, and the height is read again from
   memory for the second conversion. */
void InitViewContext(void) {
    short *res = D_00151880;
    float *ctx = (float *)D_0018CE00;
    short w = res[0xA8];
    short h = res[0xA9];
    int hw = w >> 1;
    int hh = h >> 1;
    float fh;

    D_0013E600[0] = w;
    D_0013E600[1] = h;
    D_0013E600[2] = hw;
    D_0013E600[3] = hh;
    D_0013E600[4] = (0x800 - hw) << 4;
    D_0013E600[5] = (0x800 - hh) << 4;
    D_0013E600[6] = (hw + 0x800) << 4;
    D_0013E600[7] = (hh + 0x800) << 4;
    ctx[0x28] = 32.0f;
    ctx[0x29] = 745472.0f;
    ctx[0x2C] = 0.63f;
    ctx[0x80] = IntToFloat(w) * 0.5f;
    fh = IntToFloat(res[0xA9]) * 0.5f;
    ctx[0x81] = fh;
    ctx[0x82] = ctx[0x80] * 4.0f;
    ctx[0x83] = fh * 4.0f;
    ctx[0x86] = 0.0f;
    ctx[0x87] = 524288.0f;
    ctx[0x8A] = 255.0f;
    ctx[0x8B] = 0.0f;
}

__asm__(".section .text\n\tnop\n");

typedef u32 VecQuadP __attribute__((mode(TI), aligned(16)));
typedef union { VecQuadP q; f32 f[4]; s32 i[4]; } Vec4P;
struct ViewP {
    s32 fog_color; s32 unk4; s32 unk8; s32 unkC;
    Vec4P regs[5]; Vec4P gif[4];
    f32 near_clip; f32 far_clip; f32 aspect_x; f32 aspect_y;
    Vec4P fov; f32 proj[16]; f32 proj2[16]; f32 proj3[16];
    Vec4P inverse_screen_scale, screen_scale, screen_bias, inverse_projection_scale, viewport_aspect, aspect_scale, clip_scale, clip_distances;
    f32 half_width, half_height, scr_x, scr_y, fog_mul, fog_add, fog_near_dist, fog_far_dist, fog_slope, fog_base, fog_near_int, fog_far_int;
    s32 fog_r, fog_g, fog_b;
};
#define view_context_p (*(struct ViewP *)&D_0018CE00)
extern s32 D_0015EE80 MACRO_ADDR;
#define pal_mode_p D_0015EE80
struct FogRegs2P {
    u8 pad0[0x28];
    s32 mul;
    s32 max;
    s32 far_int;
    s32 near_int;
};
struct ClipP {
    Vec4P constants[3];
};

extern Vec4P D_0018D0C0[4];
extern f32 D_00160820[4];
extern f32 D_00160C30 MACRO_ADDR;
extern f32 D_001E0250[];
extern f32 D_001E6E70[];
extern struct FogRegs2P D_001DED30;
#define shrub_vu1_constants D_001DED30
extern struct FogRegs2P D_001DED80;
extern Vec4P D_001DEDC0[2];
extern Vec4P D_001DEA90[3];
extern struct {
    u8 pad0[0x10];
    s32 mul;
    s32 far_int;
    s32 near_int;
    u8 pad1c[0x44];
    Vec4P constants[3];
    u8 pad90[0x10];
    s32 x;
    s32 y;
} D_001DEAC0;
#define tfrag_vu_const_block D_001DEAC0

extern f32 func_001FA058(f32, f32);
extern f32 func_001F9F90(f32);
#define fast_cos func_001F9F90
extern void func_001F9A00(void *, void *, s32);
extern void func_00234380(void);
#define set_tfrag_dists func_00234380

/* Rebuilds the view state from the field of view, screen size and fog
   endpoints: the clip constants at D_0018D0C0, the fog multiplier and the
   fog packet registers, the projection matrices and the screen scale and
   bias vectors that the renderers copy into their packets. */
void UpdateViewContext(void) {
    struct ViewP *view;
    struct ViewP *packet_view;
    f32 depth_scale;
    f32 depth_bias;
    f32 horizontal_clip_scale;
    f32 vertical_clip_scale;
    f32 fog_distance_range;
    f32 fog_intensity_range;
    f32 fog_distance_slope;
    f32 near_clip;
    f32 projection_near_clip;
    f32 far_clip;
    f32 fog_multiplier;
    f32 projection_fog_multiplier;
    f32 horizontal_fov;
    f32 vertical_fov;
    f32 viewport_width;
    f32 viewport_height;
    s32 near_bits;
    s32 far_bits;
    s32 mul_bits;

    depth_scale = -8388080.0f;
    depth_bias = 8388112.0f;
    if (pal_mode_p != 0) {
        view_context_p.fov.f[1] = view_context_p.fov.f[0] * 0.756f;
    } else {
        view_context_p.fov.f[1] = view_context_p.fov.f[0] * 0.775f;
    }
    view = &view_context_p;
    view->aspect_x = view->scr_x / view->half_width;
    view->aspect_y = view->scr_y / view->half_height;
    view->fov.f[2] = 1.0f / fast_cos(func_001FA058(1.0f, view->fov.f[0]));
    view->fov.f[3] = 1.0f / fast_cos(func_001FA058(1.0f, view->fov.f[1]));
    horizontal_clip_scale = 1.0f / fast_cos(func_001FA058(1.0f, view->fov.f[0] * view->aspect_x));
    vertical_clip_scale = 1.0f / fast_cos(func_001FA058(1.0f, view->fov.f[1] * view->aspect_y));
    horizontal_clip_scale /= view->fov.f[2];
    vertical_clip_scale /= view->fov.f[3];
    view->aspect_scale.f[2] = 1.0f / view->aspect_x;
    view->aspect_scale.f[3] = 1.0f / view->aspect_y;
    view->clip_scale.f[0] = view->fov.f[2];
    view->clip_scale.f[1] = view->fov.f[3];
    view->clip_distances.f[0] = view->far_clip;
    view->aspect_scale.f[0] = view->aspect_x;
    view->aspect_scale.f[1] = view->aspect_y;
    near_clip = view->near_clip;
    view->clip_distances.f[1] = near_clip;
    view->clip_scale.f[2] = (horizontal_clip_scale + vertical_clip_scale) * 0.5f;
    qcopy(&D_0018D0C0[0], &view->fov);
    qcopy(&D_0018D0C0[1], &view->aspect_scale);
    qcopy(&D_0018D0C0[2], &view->clip_scale);
    qcopy(&D_0018D0C0[3], &view->clip_distances);

    /* Retail lw instructions carry the fog endpoint float bits into packet fields. */
    far_bits = *(s32 *)&view->fog_far_int;
    near_bits = *(s32 *)&view->fog_near_int;
    fog_distance_range = view->fog_far_dist - view->fog_near_dist;
    fog_intensity_range = view->fog_far_int - view->fog_near_int;
    fog_multiplier = fog_intensity_range * near_clip / fog_distance_range;
    fog_distance_slope = fog_intensity_range / (fog_distance_range / 1024.0f);
    view->fog_add =
        (view->fog_near_int * view->fog_far_dist - view->fog_far_int * view->fog_near_dist) /
        fog_distance_range;
    view->fog_slope = fog_distance_slope;
    view->fog_base = view->fog_near_int - view->fog_near_dist / 1024.0f * fog_distance_slope;
    view->fog_mul = fog_multiplier;
    D_00160C30 = fog_multiplier;
    D_001E0250[0] = fog_multiplier;
    D_001E6E70[0] = fog_multiplier;
    mul_bits = *(s32 *)&view->fog_mul;
    tfrag_vu_const_block.mul = mul_bits;
    tfrag_vu_const_block.far_int = far_bits;
    tfrag_vu_const_block.near_int = near_bits;
    shrub_vu1_constants.mul = mul_bits;
    D_001DED80.mul = mul_bits;
    shrub_vu1_constants.far_int = far_bits;
    shrub_vu1_constants.near_int = near_bits;
    D_001DED80.far_int = far_bits;
    D_001DED80.near_int = near_bits;
    shrub_vu1_constants.max = 0x437EFFFF;
    D_001DED80.max = 0x437EFFFF;
    set_tfrag_dists();

    projection_near_clip = view->near_clip;
    far_clip = view->far_clip;
    horizontal_fov = view->fov.f[0];
    vertical_fov = view->fov.f[1];
    viewport_width = view->half_width;
    viewport_height = view->half_height;
    projection_fog_multiplier = view->fog_mul;
    D_00160820[0] = 0.5f / 210000.0f;
    D_00160820[1] = -0.5f / 210000.0f;
    D_00160820[2] = 0.5f;
    D_00160820[3] = viewport_width / (horizontal_fov * 210000.0f);
    view->proj[0] = viewport_width / (horizontal_fov * projection_near_clip);
    view->proj[1] = 0;
    view->proj[2] = 0;
    view->proj[3] = 0;
    view->proj[4] = 0;
    view->proj[5] = viewport_height / (vertical_fov * projection_near_clip);
    view->proj[6] = 0;
    view->proj[7] = 0;
    view->proj[8] = 0;
    view->proj[9] = 0;
    view->proj[10] = (far_clip + projection_near_clip) /
                     (projection_near_clip * (far_clip - projection_near_clip)) * depth_scale;
    view->proj[11] = 1.0f / projection_near_clip * projection_fog_multiplier;
    view->proj[12] = 0;
    view->proj[13] = 0;
    view->proj[14] = -2.0f * projection_near_clip * far_clip /
                     (projection_near_clip * (far_clip - projection_near_clip)) * depth_scale;
    view->proj[15] = 0;
    func_001F9A00(view->proj2, view->proj, 0x40);
    view->proj2[11] = 1.0f / view->near_clip;
    func_001F9A00(view->proj3, view->proj2, 0x40);
    view->proj3[0] /= view->scr_x;
    view->proj3[5] /= view->scr_y;
    view->proj3[10] /= depth_scale;
    view->proj3[14] /= depth_scale;
    view->inverse_screen_scale.f[0] = 1.0f / view->scr_x;
    view->inverse_screen_scale.f[1] = 1.0f / view->scr_y;
    view->inverse_screen_scale.f[2] = 1.0f / depth_scale;
    view->inverse_screen_scale.f[3] = 1.0f / view->fog_mul;
    view->screen_scale.f[0] = view->scr_x;
    view->screen_scale.f[1] = view->scr_y;
    view->screen_scale.f[2] = depth_scale;
    view->screen_scale.f[3] = view->fog_mul;
    view->inverse_projection_scale.f[0] = view->scr_x / view->proj[0];
    view->inverse_projection_scale.f[1] = view->scr_y / view->proj[5];
    view->inverse_projection_scale.f[2] = depth_scale / view->proj[10];
    view->inverse_projection_scale.f[3] = view->fog_mul;
    view->screen_bias.f[0] = 2048.0f;
    view->screen_bias.f[1] = 2048.0f;
    view->screen_bias.f[2] = depth_bias;
    view->screen_bias.f[3] = view->fog_add;
    view->viewport_aspect.f[0] = view->scr_x / view->half_width;
    view->viewport_aspect.f[1] = view->scr_y / view->half_height;
    view->viewport_aspect.f[2] = 1.0f;
    view->viewport_aspect.f[3] = 1.0f;
    view->fog_color = *(s32 *)&view->fog_far_int;
    view->unk4 = 0x103E4000;
    view->unk8 = 0xE;
    view->unkC = 0;
    qzero(&view->regs[0]);

    packet_view = &view_context_p;
    packet_view->gif[0].i[0] = *(s32 *)&packet_view->fog_mul;
    packet_view->gif[0].i[1] = 0x303E4000;
    packet_view->gif[0].i[2] = 0x412;
    packet_view->gif[0].i[3] = 0x303EC000;
    packet_view->gif[1].i[0] = *(s32 *)&packet_view->fog_mul;
    packet_view->gif[1].i[1] = 0x302E4000;
    packet_view->gif[1].i[2] = 0x412;
    packet_view->gif[1].i[3] = 0x302EC000;
    packet_view->gif[2].i[0] = *(s32 *)&packet_view->fog_mul;
    packet_view->gif[2].i[1] = 0x20364000;
    packet_view->gif[2].i[2] = 0x41;
    packet_view->gif[2].i[3] = 0;
    packet_view->gif[3].i[0] = *(s32 *)&packet_view->fog_mul;
    packet_view->gif[3].i[1] = 0x20264000;
    packet_view->gif[3].i[2] = 0x41;
    packet_view->gif[3].i[3] = 0;
    qcopy(&packet_view->regs[0], &packet_view->gif[0]);
    qcopy(&packet_view->regs[1], &packet_view->viewport_aspect);
    qcopy(&packet_view->regs[2], &packet_view->inverse_screen_scale);
    qcopy(&packet_view->regs[3], &packet_view->screen_scale);
    qcopy(&packet_view->regs[4], &packet_view->screen_bias);
    qcopy(&D_001DEA90[0], &packet_view->screen_bias);
    qcopy(&D_001DEA90[1], &packet_view->screen_scale);
    qcopy(&D_001DEA90[2], &packet_view->viewport_aspect);
    qcopy(&tfrag_vu_const_block.constants[0], &packet_view->screen_bias);
    qcopy(&tfrag_vu_const_block.constants[1], &packet_view->inverse_screen_scale);
    qcopy(&tfrag_vu_const_block.constants[2], &packet_view->screen_scale);
    tfrag_vu_const_block.x = packet_view->viewport_aspect.i[0];
    tfrag_vu_const_block.y = packet_view->viewport_aspect.i[1];
    qcopy(&D_001DEDC0[0], &packet_view->screen_scale);
    qcopy(&D_001DEDC0[1], &packet_view->screen_bias);
}

#undef view_context_p
#undef pal_mode_p
#undef shrub_vu1_constants
#undef tfrag_vu_const_block
#undef fast_cos
#undef set_tfrag_dists

extern int D_0013E600[];
extern float func_001FA888(int);

/* Sets the screen size: the GS viewport record at D_0013E600 gets width,
   height, their halves and the four <<4 edges around the 0x800 centre;
   the draw context at D_0018CE00 gets a2 at +0xB0, 32 and 524288 at
   +0xA0/+0xA4, the half extents as floats (func_001FA888 is int to
   float) at +0x200/+0x204, four times each at +0x208/+0x20C, and a3..a6
   at +0x218/+0x21C/+0x228/+0x22C; then UpdateViewContext (func_001F3140).
   The context is reached through one local pointer, which keeps its
   full address in $s0 as retail does (indexing the global directly
   folds +0xB0 into the base and spends $a0 on the %hi). */
void SetScreenSize(int w, int h, float a2, float a3, float a4, float a5, float a6) {
    float *ctx = (float *)D_0018CE00;
    int hw = w >> 1;
    int hh = h >> 1;
    float fh;

    D_0013E600[0] = w;
    D_0013E600[1] = h;
    D_0013E600[2] = hw;
    D_0013E600[3] = hh;
    D_0013E600[4] = (0x800 - hw) << 4;
    D_0013E600[5] = (0x800 - hh) << 4;
    D_0013E600[6] = (hw + 0x800) << 4;
    D_0013E600[7] = (hh + 0x800) << 4;
    ctx[0x2C] = a2;
    ctx[0x28] = 32.0f;
    ctx[0x29] = 524288.0f;
    ctx[0x80] = IntToFloat(w) * 0.5f;
    fh = IntToFloat(h) * 0.5f;
    ctx[0x81] = fh;
    ctx[0x82] = ctx[0x80] * 4.0f;
    ctx[0x83] = fh * 4.0f;
    ctx[0x86] = a3;
    ctx[0x87] = a4;
    ctx[0x8A] = a5;
    ctx[0x8B] = a6;
    UpdateViewContext();
}

typedef struct {
    char disp_and_contexts[0x150];
    s16 display_width;
    s16 display_height;
    s16 psm;
    s16 fbp0;
    s16 storage_width;
    s16 storage_height;
} FsAaBuf;

typedef struct {
    int width;
    int height;
    int half_width;
    int half_height;
    int left;
    int top;
    int right;
    int bottom;
} ScreenExtent;

typedef struct {
    u64 pad0[2];
    u64 frame1;
    u64 pad18;
    u64 frame2;
    u64 pad28;
    u64 zbuf1;
    u64 pad38;
    u64 zbuf2;
    u64 pad48;
    u64 xyoffset1;
    u64 pad58;
    u64 xyoffset2;
    u64 pad68;
    u64 scissor1;
    u64 pad78;
    u64 scissor2;
} GsDrawEnvRegs;

typedef struct {
    char b[0x60];
} GsLoadImage;

extern FsAaBuf fs_aa_buffer_3890 __asm__("D_00151880");
extern ScreenExtent screen_extent_3890 __asm__("D_0013E600");
extern GsDrawEnvRegs draw_env_3890 __asm__("D_0013D010");
extern u64 depth_reg_3890 __asm__("D_0013D200");
extern u64 masked_depth_reg_3890 __asm__("D_0013D270");
extern unsigned char clear_buf_3890[] __asm__("D_001942C0");
extern int D_0015EF74 MACRO_ADDR;
extern int D_0015EF80 MACRO_ADDR;
extern int D_0015EF84 MACRO_ADDR;
extern int D_0015EF88 MACRO_ADDR;
extern int D_0015EF8C MACRO_ADDR;
extern void func_00118D80(int);
extern int func_00120858(int, int);
extern void func_001FAB40(int, int, int, int, int, int);
extern void func_001FB498(void);
extern void func_001FB530(void);
extern void func_001FB470(void);
extern int func_00122630(GsLoadImage *, s16, s16, s16, s16, s16, s16, s16);
extern int func_00122958(GsLoadImage *, unsigned char *);

/* Adapted from Lombyte (MIT): src/video/display/set_pal_mode.c, set_pal_mode. */
void func_001F3890(void) {
    GsLoadImage image;
    int n;
    int i;
    u64 zbuf;
    u64 frame;
    u64 scissor;
    s16 w;

    func_00118D80(0);
    if (D_0015EE80 != 0) {
        D_0015EF84 = 0x100000;
        D_0015EF88 = 0x1E0000;
        D_0015EF80 = 0;
        D_0015EF8C = 0x2C0000;
        func_001FAB40(0x200, 0x1C0, 0x200, 0x200, 4, 0);
    } else {
        D_0015EF84 = 0xE0000;
        D_0015EF88 = 0x1B0000;
        D_0015EF80 = 0;
        D_0015EF8C = 0x280000;
        func_001FAB40(0x200, 0x1A0, 0x200, 0x1C0, 0, 0);
    }
    w = fs_aa_buffer_3890.display_width;
    screen_extent_3890.width = w;
    screen_extent_3890.half_width = w >> 1;
    screen_extent_3890.half_height = (screen_extent_3890.height = fs_aa_buffer_3890.display_height) >> 1;
    screen_extent_3890.left = (0x800 - screen_extent_3890.half_width) << 4;
    screen_extent_3890.top = (0x800 - screen_extent_3890.half_height) << 4;
    screen_extent_3890.right = (screen_extent_3890.half_width + 0x800) << 4;
    screen_extent_3890.bottom = (screen_extent_3890.half_height + 0x800) << 4;
    func_00118D80(0);
    func_00120858(0, 0);
    frame = (D_0015EF84 >> 13) | ((u64)(screen_extent_3890.width >> 6) << 16);
    D_0015EF74 = D_0015EF8C;
    draw_env_3890.frame1 = frame;
    D_0015EF78 = D_0015EF8C;
    zbuf = (D_0015EF88 >> 13) | 0x1000000;
    draw_env_3890.frame2 = frame;
    draw_env_3890.xyoffset1 = screen_extent_3890.left | ((u64)screen_extent_3890.top << 32);
    draw_env_3890.zbuf1 = zbuf;
    draw_env_3890.zbuf2 = zbuf;
    draw_env_3890.xyoffset2 = screen_extent_3890.left | ((u64)screen_extent_3890.top << 32);
    scissor = ((u64)(screen_extent_3890.width - 1) << 16) | ((u64)(screen_extent_3890.height - 1) << 48);
    depth_reg_3890 = zbuf;
    draw_env_3890.scissor1 = scissor;
    draw_env_3890.scissor2 = scissor;
    masked_depth_reg_3890 = zbuf | ((u64)0x8000 << 17);
    func_00118D80(0);
    func_001FB498();
    func_001FB530();
    func_00118D80(0);
    func_00120858(0, 0);
    func_001FB470();
    func_001F99B0(clear_buf_3890, 0, 0x1000);
    n = (fs_aa_buffer_3890.storage_width * fs_aa_buffer_3890.storage_height) >> 10;
    for (i = 0; i < n; i++) {
        func_00122630(&image, i << 4, 1, 0, 0, 0, 32, 32);
        func_00118D80(0);
        func_00122958(&image, clear_buf_3890);
        func_00120858(0, 0);
    }
}

/*
 * ResetDrawGlobals. Seventeen zero stores in a row, in three addressing
 * forms that are all one assembler macro: lui/$at for most, plain $gp
 * for the three that really are small-data, and $gp again for the last
 * one because it lands in the jr delay slot where a two-instruction
 * expansion will not fit.
 */
extern int D_0015F430 MACRO_ADDR;
extern int D_0015F434 MACRO_ADDR;
extern short D_0015F44C;              /* SDA, gp -0x78B4 */
extern short D_0015F460;              /* SDA, gp -0x78A0 */
extern short D_0015F470;              /* SDA, gp -0x7890 */
extern int D_0015F544 MACRO_ADDR;
extern int D_0015F548 MACRO_ADDR;
extern int D_0015F564 MACRO_ADDR;
extern int D_0015F568 MACRO_ADDR;
extern int D_0015F56C MACRO_ADDR;
extern int D_0015F570 MACRO_ADDR;
extern int D_0015F574 MACRO_ADDR;
extern int D_00161290 MACRO_ADDR;
extern int D_00161294 MACRO_ADDR;
extern int D_00161298 MACRO_ADDR;
extern int D_0016129C MACRO_ADDR;

/* ResetDrawGlobals: clears the draw-callback counts and related draw counters. */
void ResetDrawGlobals(void) {
    D_0015F564 = 0;
    D_0015F56C = 0;
    D_0015F570 = 0;
    D_0015F568 = 0;
    D_0015F574 = 0;
    D_0015F430 = 0;
    D_0015F434 = 0;
    D_0015F544 = 0;
    D_0015F548 = 0;
    *(int *)&D_0015F44C = 0;
    *(int *)&D_0015F460 = 0;
    *(int *)&D_0015F470 = 0;
    D_0015F728 = 0;
    D_00161290 = 0;
    D_00161294 = 0;
    D_00161298 = 0;
    D_0016129C = 0;
}

extern int *D_00161000 MACRO_ADDR;
extern void func_00234C98(int, long);
extern char D_0013D0C0[];
extern char D_0013D010[];

/* ResetGsRegisters(void) */
void ResetGsRegisters(void) {
    D_00161000[0] = 0x30000013;
    D_00161000[1] = (int)D_0013D0C0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000013;
    D_00161000 += 4;
    D_00161000[0] = 0x3000000B;
    D_00161000[1] = (int)D_0013D010;
    D_00161000[2] = 0;
    D_00161000[3] = 0x5000000B;
    D_00161000 += 4;
    VU1_addGSregister(0x3D, (long)D_0018CE00[0x8C] | ((long)D_0018CE00[0x8D] << 8) |
                        ((long)D_0018CE00[0x8E] << 16));
}

extern long D_00151888[3];

/* GS privileged-register writes (0x1200_00XX = the GS's memory-mapped
   register block): CSR ack, PMODE, then SMODE2/DISPFB1/DISPFB2/DISPLAY1/
   DISPLAY2/BGCOLOR set from a 3-entry table. */
/* ResetGsRegistersPr(void) */
void ResetGsRegistersPr(void) {
    *(volatile long *)0x120000E0 = 0;
    *(volatile long *)0x12000000 = 0xFFA1;
    *(volatile long *)0x12000020 = D_00151888[0];
    *(volatile long *)0x12000070 = D_00151888[1];
    *(volatile long *)0x12000090 = D_00151888[1];
    *(volatile long *)0x12000080 = D_00151888[2];
    *(volatile long *)0x120000A0 = D_00151888[2];
    *(volatile long *)0x120000D0 = 0;
}

extern short D_0015F450;             /* SDA, gp -0x78B0: Stripes * */
extern char *D_0016055C MACRO_ADDR;
extern int D_0018C44C[];
extern int D_0015F534_f __asm__("D_0015F534") MACRO_ADDR;
extern int D_0015F704 MACRO_ADDR;
extern int D_0015F6E8 MACRO_ADDR;
extern unsigned char D_0015EF40_F3D78[] __asm__("D_0015EF40") MACRO_ADDR;
extern float D_0015F53C MACRO_ADDR;
extern float D_0015F540 MACRO_ADDR;
extern float D_0015F6F8 MACRO_ADDR;
extern unsigned char D_001611C0_F3D78[] __asm__("D_001611C0") MACRO_ADDR;
extern unsigned char D_001611C1_F3D78[] __asm__("D_001611C1") MACRO_ADDR;
extern unsigned char D_001611C2_F3D78[] __asm__("D_001611C2") MACRO_ADDR;
extern unsigned char D_001611C3_F3D78[] __asm__("D_001611C3") MACRO_ADDR;
extern unsigned short D_0010FA90 NOT_SDA;
extern char D_0010FAA0[];
extern char D_0015F480[];
extern char D_0015F490[];
extern char D_0015F4A0[];
extern char D_0015F4B0[];
extern char D_0015F4C0[];
extern char D_0015F4D0[];
extern char D_0015F4E0[];
extern char D_0015F4F0[];
extern char D_0015F4F8[];
extern char D_0015F500[];
extern char D_0015F510[];
extern char D_0015F518[];
extern char D_0015F528[];
extern char D_001E7BA0[];
extern char D_001E7BB8[];
extern char D_00100AE0[];
extern char D_001E1600[];
extern char D_001E3500[];
extern char D_001D9240[];

extern void func_001F2558_a(void *, int) __asm__("func_001F2558");
extern void func_001F2560_a(void *, int) __asm__("func_001F2560");
extern void func_001F2930_v(void) __asm__("func_001F2930");
extern void func_001FB530(void);
extern void func_001F2608(void);
extern void func_0020DAB0(void);
extern void func_001E9E70(void);
extern void func_002346C0(void);
extern void func_00235290(int);
extern void func_00236CA8(void);
extern void func_00236BE0(void);
extern void func_00234F40(void);
extern void func_001F4630(int);
extern void func_001F4A78(void);
extern void func_001F4748(void);
extern void func_00234EE0(void);
extern void func_00229E50(void);
extern void func_001F54E8(char *);
extern void func_001F7B70(void);
extern void func_001F4AF0(void);
extern void func_0020E2B0(void);
extern void func_00234B48(void *, int);
extern void func_001F4A00(void);
extern void func_001EDFF8(void);
extern void func_001F4C30(void);
extern void func_00118D80(int);
extern void func_00218B10(void);
extern void func_001F4BB8(void);
extern void func_001F9478(void);
extern void func_001EE6E0(void);
extern void func_001FB848(void);
extern void func_00238D88(void);
extern void func_001FFFB8(void);
extern void func_001FF1B0(void);
extern void func_001F5148(void);
extern void func_001F4F90(void);
extern void func_001F55C0(int, int, int, int);
extern void func_001F5368(void);
extern void func_002347F0(void *);
extern void func_00234AC8(int mask);
extern void func_002362B0(void *);
extern void func_00234620(void);
extern void func_00236BB0(void);
extern void func_00236B58(void);
extern void func_00238688(void *);
extern void func_00236A98(void);
extern void func_0022B8F8(void *);
extern void func_00229D48(void);
extern void func_0020DD48(void);

/* Draws the whole frame: each world layer's DMA chain in order, then the fades and the per-layer profiler bars. Adapted from Lombyte (MIT) for PAL: rendering/draw_debug_profiler.c, draw_debug_profiler. */
void DrawDebugProfiler(void) {
    float t;
    float div;

    if (D_0016055C == 0 || *(short *)(D_0016055C + 4) != 0 || ((D_0015F534_f ^ 1) & 1) || D_0018A3B0[2] == 0 ||
        D_0018C44C[0] != 0) {
        framebuf_appendLargeSetup();
    }
    BuildViewMatrices();
    AnimSlot_ReleaseStale();
    UpdateOcclusion();
    ResetGsRegisters();
    D_0015F704 = -1;
    func_001F2558_a(D_0015F480, 0xF);
    func_001F2560_a(D_0015F480, 0xF);
    if (D_0016055C != 0 && (D_0015F534_f & 1)) {
        if (D_0018A3B0[2] != 0) {
            Transition_DrawSky();
        }
        func_001F2560_a(D_0015F490, 0xE);
        func_001F2558_a(D_0015F490, 0xE);
    }
    if (D_0015F534_f & 2) {
        DrawTfrag();
    }
    Vif1ChainCmd(0x02010000);
    if (D_0015F534_f & 4) {
        if (D_0015EE80 != 0) {
            DrawTies_2();
        } else {
            DrawTies_1();
        }
    }
    Vif1ChainCmd(0x02020000);
    if (D_0018A3B0[0x34 / 4] != 0 && D_0015F56C != 0) {
        Dma_AppendRefDirect3();
        SetupGifPaging(1);
        ExecuteDrawCallbacks3();
        DoGifPaging();
        VU1_gsRegsNormal();
    }
    func_001F2558_a(D_0015F4A0, 6);
    func_001F2560_a(D_0015F4A0, 6);
    if (D_0015F534_f & 8) {
        DrawShrubs();
    }
    Vif1ChainCmd(0x02040000);
    if (D_0018A3B0[0x44 / 4] != 0 && *(int *)&D_0015F470 != 0) {
        draw_fogged_fullscreen_sprite((char *)&D_0015F470);
    }
    if (D_0015F534_f & 0x20) {
        SetupGifPaging(1);
        DrawOptionalExtraPass();
        DoGifPaging();
    }
    if (D_0018A3B0[0x34 / 4] != 0 && D_0015F570 != 0) {
        Dma_AppendRefDirect3();
        SetupGifPaging(1);
        ExecuteDrawCallbacks4();
        DoGifPaging();
        VU1_gsRegsNormal();
    }
    if (D_0015F534_f & 0x10) {
        DrawMobys();
    }
    Vif1ChainCmd(0x02080000);
    SetupGifPaging(0);
    if ((D_0015F534_f & 0x20) && D_0018A3B0[0x30 / 4] != 0 && D_0015F704 != 6) {
        VU1_addDataRef(D_0010FAA0, D_0010FA90);
        D_0015F704 = 6;
    }
    func_001F2560_a(D_0015F4B0, 4);
    func_001F2558_a(D_0015F4B0, 4);
    if (D_0015F534_f & 0x20) {
        Dma_AppendRefDirect3();
        if (D_0018A3B0[0x34 / 4] != 0) {
            if (D_0015F564 != 0) {
                ExecuteDrawCallbacks();
            }
            VU1_addGSregister(0x42, 0x8000000048L);
            LensFlareUpdate();
            Dma_AppendRefDirect3();
            DrawLightQuads();
        }
        func_001F2558_a(D_0015F4C0, 6);
        func_001F2560_a(D_0015F4C0, 6);
        if (D_0018A3B0[0x38 / 4] != 0) {
            VU1_addGSregister(8, 5);
            Dma_AppendRefDirect3();
            func_00118D80(0);
            PartProc();
            D_0015F704 = 8;
        }
        func_001F2558_a(D_0015F4D0, 8);
        func_001F2560_a(D_0015F4D0, 8);
        if (D_0018A3B0[0x3C / 4] != 0) {
            if (D_0015F568 != 0) {
                Dma_AppendRefDirect3();
                ExecuteDrawCallbacks2();
            }
            if (D_0015F6E8 == 0) {
                DrawBillboardBatch();
            }
            VU1_addGSregister(0x42, 0x8000000044L);
            DrawQueuedGlows();
        }
        func_001F2558_a(D_0015F4E0, 6);
        func_001F2560_a(D_0015F4E0, 6);
    }
    if (D_0018A3B0[0x48 / 4] != 0) {
        AA_BlurPass();
    }
    func_001F2560_a(D_0015F4F0, 0xF);
    Dma_AppendRefDirect3();
    if (D_0015F534_f & 0x10000) {
        DebugProfiler_Hook();
    }
    if ((D_0015F534_f & 0x80) && D_0018A3B0[0x40 / 4] != 0) {
        HudDraw();
        Help_Draw();
        DrawLetterboxBars();
    }
    if (D_0015F6E8 == 2 && D_0015EF40_F3D78[0] != 0) {
        DrawSubtitles();
    }
    func_001F2560_a(D_0015F4F8, 0xE);
    func_001F2558_a(D_0015F4F8, 0xE);
    DoGifPaging();
    if (D_0015F534_f & 0x40) {
        if (D_0018A3B0[0x44 / 4] != 0) {
            VU1_addGSregister(0x42, 0x8000000044L);
            if (D_001873D4 != 0) {
                emit_rgba_draw_packet(D_001611C0_F3D78[0], D_001611C1_F3D78[0], D_001611C2_F3D78[0], D_001611C3_F3D78[0]);
            }
            if (gFade > 0.0f) {
                if (gFade > 1.0f) {
                    gFade = 1.0f;
                }
                emit_rgba_draw_packet(0, 0, 0, func_001FA898_r(gFade * 128.0f));
            }
            if (D_0015F540 > 0.0f) {
                if (D_0015F540 > 1.0f) {
                    D_0015F540 = 1.0f;
                }
                emit_rgba_draw_packet(0xFF, 0xFF, 0xFF, func_001FA898_r(D_0015F540 * 128.0f));
            }
            if (*(int *)&D_0015F44C != 0 && *(int *)&D_0015F450 != 0) {
                DrawScreenEffect();
            }
        }
        func_001F2560_a(D_0015F500, 0xA);
    }
    VU0_loadMicroProgram(D_00100AE0);
    func_00118D80(0);
    t = IntToFloat(*(volatile int *)0x10000800);
    D_0015F6F8 = t / (D_0015EE80 != 0 ? 11520.0f : 9600.0f);
    VU1_syncChain(2);
    func_001F2558_a(D_0015F510, 0x11);
    if (D_0015F534_f & 2) {
        if (D_0018A3B0[0x10 / 4] != 0) {
            LightTfrags(D_001E1600);
            PatchTfragGifs();
        }
        func_001F2558_a(D_001E7BA0, 2);
    }
    VU1_syncChain(4);
    func_001F2558_a(D_0015F510, 0x11);
    if (D_0015F534_f & 4) {
        if (D_0018A3B0[0x18 / 4] != 0) {
            if (D_0015EE80 != 0) {
                LightTieBuffers();
                copy_render_buffer_pair();
            } else {
                LightTies(D_001E3500);
                PatchTieGifs();
            }
        }
        func_001F2558_a(D_0015F518, 5);
    }
    VU1_syncChain(8);
    func_001F2558_a(D_0015F510, 0x11);
    if (D_0015F534_f & 8) {
        if (D_0018A3B0[0x20 / 4] != 0) {
            LightShrubs(D_001D9240);
            PatchShrubGifs();
        }
        func_001F2558_a(D_001E7BB8, 7);
    }
    VU1_syncChain(0x10);
    func_001F2558_a(D_0015F510, 0x11);
    if (D_0015F534_f & 0x10) {
        if (D_0018A3B0[0x28 / 4] != 0) {
            PatchMobyGifs();
        }
        func_001F2558_a(D_0015F528, 3);
    }
    func_001F2930_v();
    D_0015F728 = 0;
}

extern int D_0015F6FC;
extern short D_0015F534;              /* SDA, gp -0x77CC */
extern void func_001F3D78(void);


/* D_0015F6FC is read through a MACRO_ADDR alias: retail's one-register
   lui $2 / lw $2 (an older note filed it as an allocator question). */
void drawNormalFrame(void) {
    if (D_0015F6FC_m == 0) {
        framebuf_appendLargeSetup();
        *(int *)&D_0015F534 = 0x7F;
        DrawDebugProfiler();
    }
}

LINKER_REMNANT("asm/remnants/text", func_001F4628);

extern int *D_00161000 MACRO_ADDR;
extern int *D_0015F550 MACRO_ADDR;
extern int D_0015EF74 MACRO_ADDR;
extern int D_0015F55C MACRO_ADDR;
extern int D_0015EF8C MACRO_ADDR;
extern int D_0015F558 MACRO_ADDR;
typedef struct {
    long unk0;
    long unk8;
} PageSlot;
extern PageSlot D_0018D540[];
extern char D_0019A4E8[];

static inline char *PagingArena(void) {
    return D_0019A4E8;
}

/* SetupGifPaging(int): marks the D_00161000 packet in D_0015F550 and
   reserves 0x10 bytes, copies D_0015EF78 to D_0015EF74, clears
   D_0015F558 and the first dword of the D_0015F55C paging slots, then,
   when arg0 is 0, clears the +4 half of the arena's 8-byte list entries:
   the list at D_0019A4E8 + 0x24 (count at +0x44 of the record at +0x18)
   where it is at least D_0015EF8C >> 8, and all of the list at + 0x28
   (count at +0x24). The slot loop has its own counter, which loop
   reversal copies from the count (retail's $v1). The first list reads
   the arena through a static inline accessor (a fresh pseudo per read
   gives retail's copy for the loop) with the element offset first
   (`i * 8 + base`); the second through a block-local pointer, whose
   %hi retail keeps. */
void SetupGifPaging(int arg0) {
    int *p = D_00161000;
    int i;

    D_0015F550 = p;
    p += 4;
    D_00161000 = p;
    D_0015EF74 = D_0015EF78;
    D_0015F558 = 0;
    {
        int k;

        for (k = 0; k < D_0015F55C; k++) {
            D_0018D540[k].unk0 = 0;
        }
    }
    if (arg0 == 0) {
        for (i = 0; i < *(int *)(*(char **)(PagingArena() + 0x18) + 0x44); i++) {
            unsigned short *el = (unsigned short *)(i * 8
                + *(int *)(PagingArena() + 0x24) + 4);

            if (*el >= (D_0015EF8C >> 8)) {
                *el = 0;
            }
        }
        {
            char *b = D_0019A4E8;
            int j;

            for (j = 0; j < *(int *)(*(char **)(b + 0x18) + 0x24); j++) {
                *(short *)(*(char **)(b + 0x28) + j * 8 + 4) = 0;
            }
        }
    }
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern int *D_00161000 MACRO_ADDR;
extern int *D_0015F554 MACRO_ADDR;
extern int D_0018A3DC;
extern void func_0020C2F8(void);
extern void func_00234E80(void);

/* DoGifPaging: pushes two 4-word GIF tags (0x20000000 in the first word)
   onto the D_00161000 packet, D_0015F554 marking where it started and
   D_0015F550's tag pointing at the second one; between the two, when
   D_0018A3DC is set, func_0020C2F8 and func_00234E80 add their own.
   The first advance goes through a local advanced in place, which keeps
   the old and new pointer in one register as retail does. */
void DoGifPaging(void) {
    int *p = D_00161000;

    D_0015F554 = p;
    p += 4;
    D_00161000 = p;
    D_0015F550[0] = 0x20000000;
    D_0015F550[1] = (int)D_00161000;
    D_0015F550[2] = 0;
    D_0015F550[3] = 0;
    if (D_0018A3DC != 0) {
        func_0020C2F8();
        VU1_texFlush();
    }
    D_00161000[0] = 0x20000000;
    D_00161000[1] = (int)(D_0015F550 + 4);
    D_00161000[2] = 0;
    D_00161000[3] = 0;
    D_00161000 += 4;
    D_0015F554[0] = 0x20000000;
    D_0015F554[1] = (int)D_00161000;
    D_0015F554[2] = 0;
    D_0015F554[3] = 0;
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

typedef struct EffectTexDef {
    long tex0;
    unsigned short texel_off;
    unsigned short pal_off;
    short wlog2;
    short hlog2;
} EffectTexDef;
typedef struct TexUpload {
    int clut_data;
    short unk4;
    short cbp;
    int image_data;
    unsigned char tw;
    unsigned char th;
    short tbp;
} TexUpload;
struct EffectTextureDefinition_et {
    u64 tex0;
    u16 texel_offset_quadwords;
    u16 palette_offset_quadwords;
    s16 width_log2;
    s16 height_log2;
};
struct TextureUpload_et {
    s32 clut_data;
    s16 unk4;
    s16 cbp;
    s32 image_data;
    u8 tw;
    u8 th;
    s16 tbp;
};

extern struct EffectTextureDefinition_et effect_texture_definitions_et[] __asm__("D_0018D540");
extern struct TextureUpload_et pending_texture_uploads_et[0x40] __asm__("D_0018D140");
extern s32 pending_texture_upload_count_et __asm__("D_0015F558") MACRO_ADDR;
extern s32 level_texture_payload_address_et __asm__("D_0015F560") MACRO_ADDR;
extern s32 gs_texture_allocation_cursor_et __asm__("D_0015EF74") MACRO_ADDR;

/* GetEffectTex: the GS TEX0 word of effect texture slot index. The first
   call for a slot allocates its CLUT and texels from the GS texture cursor,
   builds and caches the word, and, while fewer than 64 uploads are
   pending, queues the slot's CLUT and pixels for upload.
   Adapted from Lombyte (MIT): src/rendering/effects/get_effect_texture.c, get_effect_texture. */
u64 func_001F4868_r(s32 index) __asm__("func_001F4868");

/* GetEffectTex with its real 64-bit return: builds and returns the GS TEX0 word for an effect texture. */
u64 func_001F4868_r(s32 index) {
    struct EffectTextureDefinition_et *texture;
    struct TextureUpload_et *upload;
    s32 width_log2;
    s32 buffer_width_shift;
    s32 palette_block_offset;
    s32 texel_address;
    s32 texel_block_offset;
    u64 tex0_word;
    s64 width_bits;
    s64 palette_bits;
    s32 pending_texture_upload_count_snapshot;

    if (effect_texture_definitions_et[index].tex0 == 0) {
        texture = &effect_texture_definitions_et[index];
        width_log2 = texture->width_log2;
        buffer_width_shift = width_log2 - 6;
        palette_block_offset = gs_texture_allocation_cursor_et >> 8;
        texel_address = gs_texture_allocation_cursor_et + 0x400;
        texel_block_offset = texel_address >> 8;
        pending_texture_upload_count_snapshot = pending_texture_upload_count_et;
        tex0_word = ((u64)(1 << ((buffer_width_shift <= -1) ? 0 : buffer_width_shift)) << 14);
        tex0_word |= texel_block_offset;
        width_bits = ((s64)((u64)(u16)texture->width_log2 << 48) >> 22) | 0x1300000;
        tex0_word |= width_bits;
        tex0_word |= (s64)((u64)(u16)texture->height_log2 << 48) >> 18;
        palette_bits = (u64)palette_block_offset << 37;
        palette_bits |= (u64)1 << 34;
        width_log2 += texture->height_log2;
        tex0_word |= palette_bits;
        tex0_word |= (u64)1 << 63;
        gs_texture_allocation_cursor_et = texel_address + (1 << width_log2);
        texture->tex0 = tex0_word;
        if (pending_texture_upload_count_snapshot < 0x40) {
            upload = &pending_texture_uploads_et[pending_texture_upload_count_snapshot];
            upload->clut_data =
                level_texture_payload_address_et + texture->palette_offset_quadwords * 0x10;
            upload->cbp = palette_block_offset;
            upload->unk4 = 0;
            *(s32 *)((u8 *)pending_texture_uploads_et + pending_texture_upload_count_snapshot * 0x10 +
                     8) = level_texture_payload_address_et + texture->texel_offset_quadwords * 0x10;
            upload->tw = (u8)texture->width_log2;
            upload->th = (u8)texture->height_log2;
            upload->tbp = texel_block_offset;
            pending_texture_upload_count_et = pending_texture_upload_count_snapshot + 1;
        }
    }
    return effect_texture_definitions_et[index].tex0;
}

/*
 * Four parallel callback lists, each a (function, argument) pair of
 * arrays with its own count, plus a "register" and a "run them all"
 * function per list. The counts are reached with retail's one-register
 * macro form, so they are MACRO_ADDR.
 */
typedef void (*DrawCallback)(void *);
extern DrawCallback D_0018DC40[];
extern void *D_0018DD40[];
extern DrawCallback D_0018DE40[];
extern void *D_0018DF40[];
extern DrawCallback D_0018E040[];
extern void *D_0018E140[];
extern DrawCallback D_0018E240[];
extern void *D_0018E340[];

/* AddDrawCallback: appends fn(arg) to the first draw-callback list. */
void AddDrawCallback(DrawCallback fn, void *arg) {
    int count = D_0015F564;
    if (count < 0x40) {
        D_0018DC40[count] = fn;
        D_0018DD40[count] = arg;
        D_0015F564 = count + 1;
    }
}

/* ExecuteDrawCallbacks: calls every callback in the first list. */
void ExecuteDrawCallbacks(void) {
    int i;
    for (i = 0; i < D_0015F564; i++) {
        D_0018DC40[i](D_0018DD40[i]);
    }
}

/* Calls every callback in the second draw-callback list. */
void ExecuteDrawCallbacks3(void) {
    int i;
    for (i = 0; i < D_0015F56C; i++) {
        D_0018E040[i](D_0018E140[i]);
    }
}

/* Calls every callback in the third draw-callback list. */
void ExecuteDrawCallbacks4(void) {
    int i;
    for (i = 0; i < D_0015F570; i++) {
        D_0018E240[i](D_0018E340[i]);
    }
}

/* Appends fn(arg) to the fourth draw-callback list. */
void enqueue_callback_list_4(DrawCallback fn, void *arg) {
    int count = D_0015F568;
    if (count < 0x40) {
        D_0018DE40[count] = fn;
        D_0018DF40[count] = arg;
        D_0015F568 = count + 1;
    }
}

/* Calls every callback in the fourth draw-callback list. */
void ExecuteDrawCallbacks2(void) {
    int i;
    for (i = 0; i < D_0015F568; i++) {
        D_0018DE40[i](D_0018DF40[i]);
    }
}

typedef float FVec4[4] __attribute__((aligned(16)));
typedef struct {
    FVec4 v;
} FRow;
typedef struct {
    FRow pos;
    FRow dir;
} LightRec;
extern FRow D_0018CBA0[4];
extern LightRec D_0018E440[];
extern int func_001F4868(int);
/* GetEffectTex as it really returns: the 64-bit TEX0 (for the callers that keep it whole). */
extern long GetEffectTex64(int) __asm__("func_001F4868");
extern void func_001F9DC0(void *dst, void *src, float len);
extern void func_001F7EF8(void *, int, int);

/* Builds a 4-row matrix per light in D_0018E440 (count D_0015F574):
   each row starts as the light's position and is pushed along the
   corner D_0018CBA0[k], projected off the normalised light direction,
   scaled by the position's w; func_001F7EF8 draws it. The packet header,
   colours and UVs are filled on the stack but never sent. Both copies
   are qcopy (retail's lq/sq stay inside the loop), and the inner loop
   needs its own counter, not the one the setup loop used. */
void DrawLightQuads(void) {
    FRow m[4];
    int colors[4];
    float uv[4][2];
    unsigned long pkt[4];
    FRow dir;
    int i;
    int j;

    pkt[1] = GetEffectTex(0);
    pkt[2] = 0xFF9000000260;
    pkt[0] = 5;
    pkt[3] = 0x8000000044;
    for (j = 0; j < 4; j++) {
        uv[j][0] = D_0018CBA0[j].v[2];
        uv[j][1] = D_0018CBA0[j].v[3];
        colors[j] = 0x40808080;
    }
    for (i = 0; i < D_0015F574; i++) {
        float s;
        int k;

        qcopy(&dir, &D_0018E440[i].dir);
        func_001F9DC0(&dir, &dir, 1.0f);
        s = D_0018E440[i].pos.v[3];
        for (k = 0; k < 4; k++) {
            float cx = D_0018CBA0[k].v[0];
            float vx = dir.v[0];
            float cy = D_0018CBA0[k].v[1];
            float d = cx * vx + cy * dir.v[1];

            qcopy(&m[k], &D_0018E440[i].pos);
            m[k].v[0] += (cx - vx * d) * s;
            m[k].v[1] += (cy - dir.v[1] * d) * s;
            m[k].v[2] -= dir.v[2] * d * s;
        }
        func_001F7EF8(m, 0, 0);
    }
}

extern int func_00122598_i(int) __asm__("func_00122598");
extern void func_00234948(void);
extern void func_002349B8(void);
extern void func_001FB498(void);
extern void func_001FB598(void);
extern int *D_00161000 MACRO_ADDR;
extern int D_0015F538 MACRO_ADDR;
extern char D_0013CED0[];

/* FadeToBlack(frames, color): `frames` VU1 chains, each drawing a
   full-screen quad (D_0013CED0's GS packet) with its alpha ramped down
   from 0x80 by (n << 7) / (n + 1) of the count still to go; `color` is
   not read. Each chain is bracketed by func_00234AC8/func_00122598 and a
   D_0015F538 bump. func_00122598 (sceGsSyncV) returns int: declared that
   way, the bump's temporary moves to $v1 as in retail. D_0015F538 is
   MACRO_ADDR so each bump reloads it in one register. */
void FadeToBlack(int frames, unsigned int color) {
    int n;
    int q;

    VU1_syncChain(1);
    func_00122598_i(0);
    D_0015F538 = D_0015F538 + 1;
    VU1_initChain();
    n = frames - 1;

    if (n >= 0) {
        do {
            PutDrawBufferLarge();
            framebuf_appendLargeSetup();
            emit_rgba_draw_packet(0, 0, 0, 0x80);
            PutDrawBufferSmall();

            q = (n << 7) / (n + 1);
            n--;

            VU1_addGSregister(1, (long)(0x80 - q) << 24);

            D_00161000[0] = 0x30000014;
            D_00161000[1] = (int)D_0013CED0;
            D_00161000[2] = 0;
            D_00161000[3] = 0x50000014;
            D_00161000 += 4;

            VU1_syncChain(1);
            func_00122598_i(0);
            D_0015F538 = D_0015F538 + 1;
            VU1_sendChain();
            VU1_swapChain();
        } while (n >= 0);
    }

    VU1_syncChain(1);
    func_00122598_i(0);
    D_0015F538 = D_0015F538 + 1;
    VU1_initChain();
    PutDrawBufferLarge();
    framebuf_appendLargeSetup();
}

typedef struct {
    short start;   /* 0x0 */
    short end;     /* 0x2 */
    short text[6]; /* 0x4: string offsets, one per language */
} Subtitle;
typedef struct {
    char pad00[0x34];
    int time;      /* 0x34 */
    char pad38[0x14];
    char *subs;    /* 0x4C */
} SubState;
extern SubState D_0018CC20_s __asm__("D_0018CC20");
extern int D_0015EE88 MACRO_ADDR;
extern int D_0013E600[];
extern void func_001F7648(void *arg0, int a1, int a2, int a3, int a4, int a5,
                          int a6, int a7, int a8);
extern void func_001F7560_l(void *, long, char *, int) __asm__("func_001F7560");
extern void func_001F62C8(int, int, int, int, int);

/* Draws the subtitle showing at the current time (D_0018CC20+0x34): the
   list at +0x4C holds 16-byte entries (start, end, and one string offset
   into the list per language; a negative start ends it). The language
   D_0015EE88 picks the string (2..5 map to 1..4, anything else to 0). The
   text is measured in a FontSetWindow buffer (func_001F7648/7560), its
   box is kept 0x14 above the bottom of the screen (D_0013E600[1]), the
   frame is drawn (func_001F62C8) and the text printed with the measure
   flag (4) cleared. The list is tested and then read again for the
   loop, which gives retail's copy of it; the clamp test is written
   bottom-first, which gives retail's registers. */
void DrawSubtitles(void) {
    Subtitle *p;
    int lang;
    int idx;

    if (D_0018CC20_s.subs == 0) {
        return;
    }
    lang = D_0015EE88;
    idx = (lang >= 2 && lang <= 5) ? lang - 1 : 0;
    for (p = (Subtitle *)D_0018CC20_s.subs; p->start >= 0; p++) {
        if (D_0018CC20_s.time < p->start) {
            continue;
        }
        if (p->end < D_0018CC20_s.time) {
            continue;
        }
        {
            short win[16];
            short w, h;
            int hw, hh;

            FontSetWindow(win, 0xC8, 0x208, 0x28, 0x1D8, 0x100, D_0013E600[1] - 0x38, 0x12, 7);
            func_001F7560_l(win, 0x80B0B0B0, D_0018CC20_s.subs + p->text[idx], -1);
            win[5] = D_0013E600[1] - 0x3C;
            w = win[6];
            h = win[7];
            hh = (h >> 1) + 5;
            hw = (w >> 1) + 10;
            if (win[5] + hh > D_0013E600[1] - 0x14) {
                win[5] = D_0013E600[1] - 0x14 - hh;
            }
            DrawUIFrame(win[5] - hh, win[5] + hh, 0x100 - hw, 0x100 + hw, 0x60);
            win[9] &= ~4;
            func_001F7560_l(win, 0x80B0B0B0, D_0018CC20_s.subs + p->text[idx], -1);
            return;
        }
    }
}

extern char D_00160920[];
extern char D_00160930[];

/* Letterbox bars: while D_0015F544 is set the bar height D_0015F548
   grows to 24, otherwise it shrinks to 0. While it is non-zero, append
   a GIF packet (the D_00160920/D_00160930 register descriptors, PRIM
   0x104) drawing two full-width strips, the height in 16ths reaching in
   from the top and bottom of the D_0013E600 viewport, the same packet
   steps as func_001F5650. */
void DrawLetterboxBars(void) {
    int h;

    if (D_0015F544 != 0) {
        if (D_0015F548 < 24) {
            D_0015F548++;
        }
    } else {
        if (D_0015F548 == 0) {
            return;
        }
        D_0015F548--;
    }
    h = D_0015F548;
    if (h == 0) {
        return;
    }
    h <<= 4;
    D_00161000[0] = 0x10000007;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000007;
    {
        int *base = D_00161000;
        D_00161000 = base + 4;
        qcopy(D_00161000, D_00160920);
        *(short *)(base + 4) = 0x8001;
    }
    {
        int *base = D_00161000;
        long *p;
        D_00161000 = base + 4;
        p = (long *)D_00161000;
        p[0] = 0x104;
        p[1] = 0x80000000;
    }
    {
        int *base = D_00161000;
        D_00161000 = base + 4;
        qcopy(D_00161000, D_00160930);
        *(short *)(base + 4) = 0x8008;
    }
    {
        int *base = D_00161000;
        long *p;
        D_00161000 = base + 4;
        p = (long *)D_00161000;
        p[0] = D_0013E600[4] | ((long)D_0013E600[5] << 16) | ((long)0xFFFFF3 << 32);
        p[1] = D_0013E600[4] | ((long)(D_0013E600[5] + h) << 16) | ((long)0xFFFFF3 << 32);
        p[2] = D_0013E600[6] | ((long)D_0013E600[5] << 16) | ((long)0xFFFFF3 << 32);
        p[3] = D_0013E600[6] | ((long)(D_0013E600[5] + h) << 16) | ((long)0xFFFFF3 << 32);
        p[4] = D_0013E600[6] | ((long)D_0013E600[7] << 16) | ((long)0xFFFFF3 << 32);
        p[5] = D_0013E600[6] | ((long)(D_0013E600[7] - h) << 16) | ((long)0xFFFFF3 << 32);
        p[6] = D_0013E600[4] | ((long)D_0013E600[7] << 16) | ((long)0xFFFFF3 << 32);
        p[7] = D_0013E600[4] | ((long)(D_0013E600[7] - h) << 16) | ((long)0xFFFFF3 << 32);
    }
    D_00161000 = (int *)((char *)D_00161000 + 0x40);
}

typedef struct {
    char pad0[4];
    int color;
    long enable;
    int step0;
    int color0;
    long enable0;
    int step1;
    int color1;
    long enable1;
} Stripes;
extern short D_0015F450;             /* SDA, gp -0x78B0: Stripes * */
extern void func_001F5650(int, int, int, int, unsigned long);

#define STRIPES (*(Stripes **)&D_0015F450)

/* Fills the screen in vertical stripes: an optional full-screen colour
   first, then alternating stripes of widths step0/step1 in colours
   color0/color1, each with its own GS register 0x42 blend word when set.
   Adapted from Lombyte (MIT) for PAL. */
void DrawScreenEffect(void) {
    int i;
    short w;
    long mask;
    int c;
    long e;
    int v14;
    long e18;
    int v24;
    long e28;

    i = 0;
    w = D_00151880[0xA9];
    e = STRIPES->enable;
    if (e != 0) {
        VU1_addGSregister(0x42, e & 0xFF000000FFL);
    }
    c = STRIPES->color;
    if (c & 0xFF000000) {
        DrawRectOverlay_FiiiiUl(0, w, 0, D_00151880[0xA8], (unsigned long)((long)c << 0x20) >> 0x20);
    }
    if (w > 0) {
        mask = 0xFF000000FFL;
        do {
            e18 = STRIPES->enable0;
            if (e18 != 0) {
                VU1_addGSregister(0x42, e18 & mask);
            }
            v14 = STRIPES->color0;
            if (v14 & 0xFF000000) {
                DrawRectOverlay_FiiiiUl(i, (i + STRIPES->step0 < w - 1) ? i + STRIPES->step0 : w - 1, 0,
                              D_00151880[0xA8], (unsigned long)((long)v14 << 0x20) >> 0x20);
            }
            i = i + STRIPES->step0;
            e28 = STRIPES->enable1;
            if (e28 != 0) {
                VU1_addGSregister(0x42, e28 & mask);
            }
            v24 = STRIPES->color1;
            if (v24 & 0xFF000000) {
                DrawRectOverlay_FiiiiUl(i, (i + STRIPES->step1 < w - 1) ? i + STRIPES->step1 : w - 1, 0,
                              D_00151880[0xA8], (unsigned long)((long)v24 << 0x20) >> 0x20);
            }
            i = i + STRIPES->step1;
        } while (i < w);
    }
}

#undef STRIPES

extern int D_0015EF88 MACRO_ADDR;

/* GS register writes around an overlay: blend register 0x42 from the
   64-bit word at +8 while it is set, and when the colour at +4 has an
   alpha byte, register 0x4E switched around a full-screen
   func_001F5650 draw. The 64-bit constants are ps2eeas's dli
   sequences (tools/ps2eeas_dli.py). */
void draw_fogged_fullscreen_sprite(char *arg0) {
    long v = *(long *)(arg0 + 8);

    if (v != 0) {
        VU1_addGSregister(0x42, v & 0xFF000000FFL);
    }
    if ((*(int *)(arg0 + 4) & 0xFF000000) != 0) {
        VU1_addGSregister(0x4E, (D_0015EF88 >> 13) | 0x1000000 | 0x100000000L);
        DrawRectOverlay_FiiiiUl(0, D_00151880[0xA9], 0, D_00151880[0xA8],
                      *(unsigned int *)(arg0 + 4));
        VU1_addGSregister(0x4E, 0x1000000 | (D_0015EF88 >> 13));
    }
    if (*(long *)(arg0 + 8) != 0) {
        VU1_addGSregister(0x42, 0x8000000044L);
    }
}

/* Sets GS register 1 from four bytes packed into one 64-bit value, then
   restores the default register set. The parameters are int, widened in
   the expression: with long parameters the scheduler hoists the last
   dsll one slot early (it was a 6/144 near-miss that way). */
extern int *D_00161000 MACRO_ADDR;
extern char D_0013CD90[];

/* Appends a GIF packet that sets GS register 1 from the four bytes a, b, c, d. */
void emit_rgba_draw_packet(int a, int b, int c, int d) {
    VU1_addGSregister(1, (long)a | ((long)b << 8) | ((long)c << 16) | ((long)d << 24));
    D_00161000[0] = 0x30000014;
    D_00161000[1] = (int)D_0013CD90;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000014;
    D_00161000 += 4;
}


/* DrawRectOverlay: append a GIF packet drawing the rectangle x0..x1,
   y0..y1 (in 16ths, offset by the viewport origin D_0013E600[4]/[5]
   - 8) as a PRIM 0x144 sprite pair in colour rgba: the tag, the
   D_00160920 and D_00160930 register descriptors (ids 0x8001/0x8004),
   then four XYZ values at Z 0xFFFFF0. Each packet step has its own
   block-scoped base pointer, which gives retail's registers. */
void DrawRectOverlay_FiiiiUl(int y0, int y1, int x0, int x1, unsigned long rgba) {
    D_00161000[0] = 0x10000005;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000005;
    {
        int *base = D_00161000;
        D_00161000 = base + 4;
        qcopy(D_00161000, D_00160920);
        *(short *)(base + 4) = 0x8001;
    }
    {
        int *base = D_00161000;
        long *p;
        D_00161000 = base + 4;
        p = (long *)D_00161000;
        p[0] = 0x144;
        p[1] = rgba;
    }
    {
        int *base = D_00161000;
        D_00161000 = base + 4;
        qcopy(D_00161000, D_00160930);
        *(short *)(base + 4) = 0x8004;
    }
    {
        int *base = D_00161000;
        long *p;
        D_00161000 = base + 4;
        p = (long *)D_00161000;
        p[0] = (x0 * 16 + D_0013E600[4] - 8)
             | ((long)(y0 * 16 + D_0013E600[5] - 8) << 16)
             | ((long)0xFFFFF0 << 32);
        p[1] = (x1 * 16 + D_0013E600[4] - 8)
             | ((long)(y0 * 16 + D_0013E600[5] - 8) << 16)
             | ((long)0xFFFFF0 << 32);
        p[2] = (x0 * 16 + D_0013E600[4] - 8)
             | ((long)(y1 * 16 + D_0013E600[5] - 8) << 16)
             | ((long)0xFFFFF0 << 32);
        p[3] = (x1 * 16 + D_0013E600[4] - 8)
             | ((long)(y1 * 16 + D_0013E600[5] - 8) << 16)
             | ((long)0xFFFFF0 << 32);
    }
    D_00161000 = (int *)((char *)D_00161000 + 0x20);
}

LINKER_REMNANT("asm/remnants/text", func_001F57F8);

extern char D_00160940[];

/* Append a textured sprite as a four-vertex strip (PRIM 0x154): screen
   corners in 12.4 fixed point relative to the viewport origin, UVs from
   (u, v) to (u + uw, v + vh) in 16ths. */
void DrawTexturedQuad(int x, int y, int w, int h, int u, int v, int uw, int vh,
                   unsigned long rgba, unsigned long tex) {
    int x0 = x * 16 + D_0013E600[4] - 8;
    int x1 = (x + w) * 16 + D_0013E600[4] - 8;
    int y0 = y * 16 + D_0013E600[5] - 8;
    int y1 = (y + h) * 16 + D_0013E600[5] - 8;
    int s1 = (u + uw) * 16;
    int s0 = u * 16;
    int vb = v + vh;
    long *p;
    int *base;

    D_00161000[0] = 0x10000007;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000007;
    base = D_00161000;
    D_00161000 = base + 4;
    qcopy(base + 4, D_00160940);
    p = (long *)(base + 8);
    D_00161000 = base + 8;
    p[0] = tex;
    p[1] = 0x154;
    p[2] = rgba;
    p[3] = (v << 20) + s0;
    p[4] = x0 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[5] = (v << 20) + s1;
    p[6] = x1 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[7] = (vb << 20) + s0;
    p[8] = x0 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[9] = (vb << 20) + s1;
    p[10] = x1 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[11] = 0;
    D_00161000 = (int *)((char *)D_00161000 + 0x60);
}

extern int func_001FA898_r(float) __asm__("func_001FA898");

/* func_001F5800 with a float screen rectangle: each corner is rounded
   (func_001FA898) from 16ths before the viewport offset. */
void func_001F5988(float x, float y, float w, float h, int u, int v, int uw, int vh,
                   unsigned long rgba, unsigned long tex) {
    int x0 = func_001FA898_r(x * 16.0f) + D_0013E600[4] - 8;
    int x1 = func_001FA898_r((x + w) * 16.0f) + D_0013E600[4] - 8;
    int y0 = func_001FA898_r(y * 16.0f) + D_0013E600[5] - 8;
    int y1 = func_001FA898_r((y + h) * 16.0f) + D_0013E600[5] - 8;
    int s1 = (u + uw) * 16;
    int s0 = u * 16;
    int vb = v + vh;
    long *p;
    int *base;

    D_00161000[0] = 0x10000007;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000007;
    base = D_00161000;
    D_00161000 = base + 4;
    qcopy(base + 4, D_00160940);
    p = (long *)(base + 8);
    D_00161000 = base + 8;
    p[0] = tex;
    p[1] = 0x154;
    p[2] = rgba;
    p[3] = (v << 20) + s0;
    p[4] = x0 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[5] = (v << 20) + s1;
    p[6] = x1 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[7] = (vb << 20) + s0;
    p[8] = x0 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[9] = (vb << 20) + s1;
    p[10] = x1 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[11] = 0;
    D_00161000 = (int *)((char *)D_00161000 + 0x60);
}

extern char D_00160960[];

/* func_001F5988, skipped when the rectangle lies outside 0x7000..0x9000
   (12.4), with a CLAMP register (REGION_CLAMP to the texel rectangle)
   after the PRIM and a closing (5, 0) pair. */
void func_001F5BB8(float x, float y, float w, float h, int u, int v, int uw, int vh,
                   unsigned long rgba, unsigned long tex) {
    int x0 = func_001FA898_r(x * 16.0f) + D_0013E600[4] - 8;
    int x1 = func_001FA898_r((x + w) * 16.0f) + D_0013E600[4] - 8;
    int y0 = func_001FA898_r(y * 16.0f) + D_0013E600[5] - 8;
    int y1 = func_001FA898_r((y + h) * 16.0f) + D_0013E600[5] - 8;
    int s1;
    int s0;
    int vb;
    int ub;
    long *p;
    int *base;

    if (x0 > 0x9000 || x1 < 0x7000 || y0 > 0x9000 || y1 < 0x7000) {
        return;
    }
    D_00161000[0] = 0x10000008;
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = 0x50000008;
    ub = u + uw;
    vb = v + vh;
    s0 = u * 16;
    s1 = ub * 16;
    base = D_00161000;
    D_00161000 = base + 4;
    qcopy(base + 4, D_00160960);
    p = (long *)(base + 8);
    D_00161000 = base + 8;
    p[0] = tex;
    p[1] = 0x154;
    p[2] = 0xA | ((long)u << 4) | ((long)ub << 14) | ((long)v << 24) | ((long)vb << 34);
    p[3] = rgba;
    p[4] = (v << 20) + s0;
    p[5] = x0 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[6] = (v << 20) + s1;
    p[7] = x1 | ((long)y0 << 16) | ((long)0xFFFFF0 << 32);
    p[8] = (vb << 20) + s0;
    p[9] = x0 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[10] = (vb << 20) + s1;
    p[11] = x1 | ((long)y1 << 16) | ((long)0xFFFFF0 << 32);
    p[12] = 5;
    p[13] = 0;
    D_00161000 = (int *)((char *)D_00161000 + 0x70);
}

struct Screen {
    s32 width; /* 0x0: display width (D_00151780 +0x150) */
    s32 height; /* 0x4: display height (D_00151780 +0x152) */
    s32 half_width; /* 0x8: width >> 1 */
    s32 half_height; /* 0xC: height >> 1 */
    s32 left; /* 0x10: (0x800 - half_width) << 4 */
    s32 top; /* 0x14: (0x800 - half_height) << 4 */
    s32 right; /* 0x18: (0x800 + half_width) << 4 */
    s32 bottom; /* 0x1C: (0x800 + half_height) << 4 */
};
extern struct Screen D_0013E600_F5E60 __asm__("D_0013E600");
struct DmaTag {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
struct GifTag;
union PacketCursor {
    struct DmaTag *tag;
    struct GifTag *gif;
    s32 *words;
    u8 *bytes;
    s32 addr;
};
extern union PacketCursor D_00161000_F5E60 __asm__("D_00161000") MACRO_ADDR;
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9C30(void *, void *, f32);
extern f32 func_001F9F90(f32);
extern f32 func_001F9FA8(f32);
extern s32 func_001FA898(f32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/append_rotated_sprite_quad.c, append_rotated_sprite_quad. */
void func_001F5E60(f32 center_x, f32 center_y, f32 quad_width, f32 quad_height,
                                f32 angle, s32 texture_width, s32 texture_height,
                                s64 texture_tex0, s64 z_and_fog, s32 color, u8 flip_u, u8 flip_v,
                                f32 pivot_u, f32 pivot_v) {
    Vec4f vertical_edge;
    Vec4f horizontal_edge;
    Vec4f center;
    Vec4f temporary;
    Vec4f top_left;
    Vec4f top_right;
    Vec4f bottom_left;
    Vec4f bottom_right;
    struct DmaTag *tag;
    u64 *packet_words;
    s32 texture_left;
    s32 texture_right;
    s32 texture_top;
    s32 texture_bottom;
    f32 inverse_pivot_u;
    f32 inverse_pivot_v;

    if (flip_u != 0) {
        texture_left = ((u32)texture_width << 4);
        texture_right = 0x10;
    } else {
        texture_right = ((u32)texture_width << 4);
        texture_left = 0x10;
    }
    if (flip_v != 0) {
        texture_top = ((u32)texture_height << 20);
        texture_bottom = 0x100000;
    } else {
        texture_bottom = ((u32)texture_height << 20);
        texture_top = 0x100000;
    }
    center.x = center_x;
    center.y = center_y;
    inverse_pivot_v = 1.0f - pivot_v;
    vertical_edge.x = quad_height * FastSin(angle);
    vertical_edge.y = quad_height * FastCos(angle);
    horizontal_edge.x = quad_width * FastCos(angle);
    horizontal_edge.y = -quad_width * FastSin(angle);
    inverse_pivot_u = 1.0f - pivot_u;
    FastVecScale(&temporary, &vertical_edge, inverse_pivot_v);
    FastVecAdd(&top_left, &center, &temporary);
    FastVecScale(&temporary, &horizontal_edge, inverse_pivot_u);
    FastVecSub(&top_left, &top_left, &temporary);
    FastVecScale(&temporary, &vertical_edge, inverse_pivot_v);
    FastVecAdd(&top_right, &center, &temporary);
    FastVecScale(&temporary, &horizontal_edge, pivot_u);
    FastVecAdd(&top_right, &top_right, &temporary);
    FastVecScale(&temporary, &vertical_edge, pivot_v);
    FastVecSub(&bottom_left, &center, &temporary);
    FastVecScale(&temporary, &horizontal_edge, inverse_pivot_u);
    FastVecSub(&bottom_left, &bottom_left, &temporary);
    FastVecScale(&temporary, &vertical_edge, pivot_v);
    FastVecSub(&bottom_right, &center, &temporary);
    FastVecScale(&temporary, &horizontal_edge, pivot_u);
    FastVecAdd(&bottom_right, &bottom_right, &temporary);
    D_00161000_F5E60.tag->tag = 0x10000007;
    D_00161000_F5E60.tag->addr = 0;
    D_00161000_F5E60.tag->vif0 = 0;
    D_00161000_F5E60.tag->vif1 = 0x50000007;
    tag = D_00161000_F5E60.tag;
    packet_words = (u64 *)(tag + 1);
    D_00161000_F5E60.tag = tag + 1;
    packet_words[0] = 0xB400000000008001;
    packet_words[1] = 0x53535353106;
    packet_words[2] = texture_tex0;
    packet_words[3] = 0x154;
    packet_words[4] = color;
    packet_words[5] = texture_left | texture_top;
    packet_words[6] =
        (truncate_float_to_s32(top_left.x * 16.0f) + D_0013E600_F5E60.left - 8) |
        ((u64)(truncate_float_to_s32(top_left.y * 16.0f) + D_0013E600_F5E60.top - 8) << 16) |
        ((u64)z_and_fog << 32);
    packet_words[7] = texture_right | texture_top;
    packet_words[8] =
        (truncate_float_to_s32(top_right.x * 16.0f) + D_0013E600_F5E60.left - 8) |
        ((u64)(truncate_float_to_s32(top_right.y * 16.0f) + D_0013E600_F5E60.top - 8) << 16) |
        ((u64)z_and_fog << 32);
    packet_words[9] = texture_left | texture_bottom;
    packet_words[10] =
        (truncate_float_to_s32(bottom_left.x * 16.0f) + D_0013E600_F5E60.left - 8) |
        ((u64)(truncate_float_to_s32(bottom_left.y * 16.0f) + D_0013E600_F5E60.top - 8) << 16) |
        ((u64)z_and_fog << 32);
    packet_words[11] = texture_right | texture_bottom;
    packet_words[12] =
        (truncate_float_to_s32(bottom_right.x * 16.0f) + D_0013E600_F5E60.left - 8) |
        ((u64)(truncate_float_to_s32(bottom_right.y * 16.0f) + D_0013E600_F5E60.top - 8) << 16) |
        ((u64)z_and_fog << 32);
    packet_words[13] = 0;
    D_00161000_F5E60.tag = (struct DmaTag *)((u8 *)D_00161000_F5E60.tag + 0x70);
}

LINKER_REMNANT("asm/remnants/text", func_001F62C0);

/* Draws a bevelled frame: the box itself, then three shrinking bars
   above and three below it, all in grey 0x040404 with alpha a. */
void DrawUIFrame(int x0, int x1, int y0, int y1, int a) {
    int col = (a << 24) | 0x40404;

    DrawRectOverlay_FiiiiUl(x0, x1, y0, y1, col);
    DrawRectOverlay_FiiiiUl(x0 + 1, x1 - 1, y0 - 2, y0, col);
    DrawRectOverlay_FiiiiUl(x0 + 2, x1 - 2, y0 - 3, y0 - 2, col);
    DrawRectOverlay_FiiiiUl(x0 + 4, x1 - 4, y0 - 4, y0 - 3, col);
    DrawRectOverlay_FiiiiUl(x0 + 1, x1 - 1, y1, y1 + 2, col);
    DrawRectOverlay_FiiiiUl(x0 + 2, x1 - 2, y1 + 2, y1 + 3, col);
    DrawRectOverlay_FiiiiUl(x0 + 4, x1 - 4, y1 + 3, y1 + 4, col);
}

/* Emits a 9-point cross/star pattern of func_001F5650 draws around
   (a0, a1, a2, a3), offset by +-1/3/5 along each axis, all sharing the
   colour/flags word a4. The first call passes a4 with only its top byte
   kept and the low nibble forced to 4; the rest pass a4 unchanged. */
void DrawOutlinedRect(int a0, int a1, int a2, int a3, int a4) {
    int color;
    int t0, t1, t2;
    int t_a0m5, t4, t5;
    int t_a0p3, t6, t_a2m3;
    int t_a1p3, t3, t_a3p1;

    color = (a4 & (int)0xFF000000) | 4;
    DrawRectOverlay_FiiiiUl(a0, a1, a2, a3, color);

    t0 = a0 + 1;
    t1 = a2 + 3;
    t2 = a3 + 5;
    DrawRectOverlay_FiiiiUl(a0 - 1, t0, t1, t2, a4);

    t_a0m5 = a0 - 5;
    t4 = a2 - 1;
    t5 = a3 - 3;
    DrawRectOverlay_FiiiiUl(a0 - 3, t_a0m5, t4, t5, a4);

    t3 = a1 - 3;
    DrawRectOverlay_FiiiiUl(t_a0m5, t3, t4, a2 + 1, a4);

    t_a0p3 = a0 + 3;
    t6 = a1 + 1;
    t_a2m3 = a2 - 3;
    DrawRectOverlay_FiiiiUl(t_a0p3, t6, t_a2m3, a2 - 5, a4);

    DrawRectOverlay_FiiiiUl(a1 - 1, t6, t_a2m3, t5, a4);

    t_a1p3 = a1 + 3;
    t_a3p1 = a3 + 1;
    DrawRectOverlay_FiiiiUl(t_a1p3, a1 + 5, t1, t_a3p1, a4);

    DrawRectOverlay_FiiiiUl(t_a0p3, t_a1p3, a3 - 1, t_a3p1, a4);

    DrawRectOverlay_FiiiiUl(t0, t3, a3 + 3, t2, a4);
}

/* gp-relative: declared as a 2-byte type purely so -G2 places it in the
   small-data area (placement is decided by DECLARED size), then accessed
   as the 4-byte word it really is. gp base 0x166D00 - 0x7764 = 0x15F59C. */
extern short D_0015F59C;

/* Sets the word flag at D_0015F59C to 1. */
void Font_EnableColorCodes(void) {
    *(int *)&D_0015F59C = 1;
}

/* Clears the word flag at D_0015F59C. */
void Font_DisableColorCodes(void) {
    *(int *)&D_0015F59C = 0;
}

/* String width: sums the signed width byte (+3 of each 4-byte entry of
   `table`, indexed by character) over at most `count` characters of `str`,
   stopping at the NUL. Called by func_001F6600/20/40 with the three font
   tables. A do-while behind an entry test, with `p = str` set inside the
   if: the body's `*p` then sits after the loop label, so CSE keeps it
   apart from the entry test's `*str`, and `i = 0` is not folded into the
   first `i++`, both as in retail. */
int measure_text_width(unsigned char *str, int count, void *table) {
    int width = 0;
    int i = 0;
    unsigned char *p;

    if (count != 0 && *str != 0) {
        p = str;
        do {
            int c = *p;
            int w;

            i++;
            p++;
            w = ((signed char *)table)[c * 4 + 3];
            if (w != 0) {
                width += w;
            }
        } while (i != count && *p != 0);
    }
    return width;
}

extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];

/* measure_text_width_regular: width of the first arg1 characters of arg0 in the regular font table. */
int MeasureTextWidthRegular(unsigned char *arg0, int arg1) {
    return measure_text_width(arg0, arg1, D_001DF3D0);
}

/* measure_text_width_small: width of the first arg1 characters of arg0 in the small font table. */
int MeasureTextWidthSmall(unsigned char *arg0, int arg1) {
    return measure_text_width(arg0, arg1, D_001DF770);
}

/* measure_text_width_large: width of the first arg1 characters of arg0 in the large font table. */
int MeasureTextWidthLarge(unsigned char *arg0, int arg1) {
    return measure_text_width(arg0, arg1, D_001DFB10);
}

/*
 * Retail has 8 bytes of nop padding between func_001F6640 and
 * func_001F6668, and it lives *after* `endlabel` in
 * asm/nonmatchings/text/func_001F6640.s -- so the INCLUDE_ASM stub was
 * supplying it, and replacing that stub with C silently dropped it,
 * shifting every later function in the segment by -8 and corrupting
 * their `jal` targets (func_001F7B40 read 1/44 while being
 * instruction-for-instruction identical to retail). Emitted explicitly
 * to preserve the layout.
 *
 * Check for this whenever converting a stub: content after a .s file's
 * `endlabel` is inter-function padding the stub was carrying, and it has
 * to be reproduced or everything downstream drifts. Alignment directives
 * do not cover it -- both boundaries here are already 8-byte aligned.
 */
__asm__(".section .text\n\tnop\n\tnop\n");

struct Glyph_F6668 {
    u8 u;
    u8 v;
    s8 top;
    s8 adv;
};
extern s32 D_0015F5A0_F6668 __asm__("D_0015F5A0") MACRO_ADDR;
extern s32 D_0015F59C_F6668 SDATA(D_0015F59C);
extern s32 D_0018CBF8_F6668[] __asm__("D_0018CBF8");
extern void func_001F5800_F6668(s32, s32, s32, s32, s32, s32, s32, s32, s64, s64) __asm__("func_001F5800");

/* FontPrint: draws a string glyph by glyph; bytes 8..15 switch the colour, 0x80..0xA7 add an accent glyph,
   bytes below 0x20 are 24-pixel grey button glyphs.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/text/font_print.c, font_print. */
void func_001F6668_r(s32 x, s32 y, u64 color, u8 *text, s32 character_limit, s64 texture,
                      struct Glyph_F6668 *glyphs) __asm__("func_001F6668");
void func_001F6668_r(s32 x, s32 y, u64 color, u8 *text, s32 character_limit, s64 texture,
                      struct Glyph_F6668 *glyphs) {
    s32 character_count;
    u8 *cursor;
    u8 character;
    struct Glyph_F6668 *overlay_glyph;
    s32 gray_value;
    s32 gray_color;

    if (D_0015F5A0_F6668 == 0) {
        D_0018CBF8_F6668[0] = color;
    }
    character_count = 0;
    if (character_limit == 0 || *text == 0) {
        return;
    }
    cursor = text;
    do {
        /* Bytes 8..15 select a palette color, preserving the current alpha. */
        if ((u8)(*cursor - 8) < 8) {
            if (D_0015F59C_F6668 != 0) {
                color &= 0xFF000000;
                color |= D_0018CBF8_F6668[*cursor - 8] & 0xFFFFFF;
            }
        } else {
            character = *cursor;
            if (glyphs[character].adv != 0) {
                /* Characters 0x80..0xa7 also draw the glyph 0x40 entries later. */
                if ((u8)(character + 0x80) < 0x28) {
                    overlay_glyph = (struct Glyph_F6668 *)(((character + 0x40) << 2) + (s32)glyphs);
                    func_001F5800_F6668(x + overlay_glyph->adv, y + overlay_glyph->top, 16, 16,
                                       overlay_glyph->u, overlay_glyph->v, 16, 16, color, texture);
                }
                /* Control glyphs use 24-pixel grayscale sprites. */
                if (*cursor < 0x20) {
                    gray_value =
                        (s32)((color & 0xFF) + ((color >> 8) & 0xFF) + ((color >> 16) & 0xFF)) / 3;
                    gray_color = (s32)(color & 0xFF000000);
                    gray_color += gray_value << 16;
                    gray_color += gray_value << 8;
                    gray_value += gray_color;
                    func_001F5800_F6668(x, y + glyphs[*cursor].top, 24, 16, glyphs[*cursor].u,
                                       glyphs[*cursor].v, 24, 16, gray_value, texture);
                } else if (*cursor > 0x20) {
                    func_001F5800_F6668(x, y + glyphs[*cursor].top, 16, 16, glyphs[*cursor].u,
                                       glyphs[*cursor].v, 16, 16, color, texture);
                }
                x += glyphs[*cursor].adv;
            }
        }
        character_count++;
        if (character_count == character_limit) {
            break;
        }
        cursor++;
    } while (*cursor != 0);
}

extern void func_001F6668(void *, void *, void *, void *, void *, long,
                          unsigned char *);

/* Same shape as func_001F7560/func_001F75D0 below, one argument wider:
   mode 1 vs 2, D_001DF3D0 vs D_001DF770. Seven arguments, so EABI puts
   the fifth through seventh in $8/$9/$10. */
/* FontPrintLarge */
void FontPrintLarge(void *a, void *b, void *c, void *d, void *e) {
    long mode = GetEffectTex64(1);

    FontPrint(a, b, c, d, e, mode, D_001DF3D0);
}

/* FontPrintSmall */
void FontPrintSmall(void *a, void *b, void *c, void *d, void *e) {
    long mode = GetEffectTex64(2);

    FontPrint(a, b, c, d, e, mode, D_001DF770);
}

LINKER_REMNANT("asm/remnants/text", func_001F69E8);

struct Glyph {
    u8 u;
    u8 v;
    s8 top;
    s8 adv;
};
extern s32 D_0015F5A0 MACRO_ADDR;
extern s32 D_0018CBF8[];
extern f32 func_001FA888(s32);
extern void func_001F5BB8_F69F0(f32, f32, f32, f32, s32, s32, s32, s32, u64, s64) __asm__("func_001F5BB8");
void func_001F69F0(u64 color, u8 *s, s32 n, s64 tex, struct Glyph *g, f32 x, f32 y, f32 scale);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/audio/music/process_bgm_display_text_event.c, process_bgm_display_text_event. */
void Font_DrawString(u64 color, u8 *str, s32 n, s64 tex, struct Glyph *g, f32 x, f32 y, f32 scale) {
    u8 *s;
    s32 i;
    f32 size;
    f32 top;
    f32 dx;
    f32 dy;
    struct Glyph *e;
    s32 avg;
    s32 mk;

    if (D_0015F5A0 == 0) {
        D_0018CBF8[0] = color;
    }
    size = scale * 16.0f;
    i = 0;
    if (n == 0 || *str == 0) {
        return;
    }
    s = str;
    do {
        if ((u8)(*s - 8) < 8) {
            if ((*(s32 *)&D_0015F59C) != 0) {
                color &= 0xFF000000;
                color |= D_0018CBF8[*s - 8] & 0xFFFFFF;
            }
        } else if (g[*s].adv != 0) {
            top = IntToFloat(g[*s].top) * scale;
            if ((u8)(*s + 0x80) < 0x28) {
                e = (struct Glyph *)(((*s + 0x40) << 2) + (s32)g);
                dx = IntToFloat(e->adv) * scale;
                dy = IntToFloat(e->top) * scale;
                func_001F5BB8_F69F0(x + dx, y + dy, size, size, e->u, e->v, 16, 16, color, tex);
            }
            if (*s < 0x20) {
                avg = (s32)((color & 0xFF) + ((color >> 8) & 0xFF) + ((color >> 16) & 0xFF)) / 3;
                mk = (s32)(color & 0xFF000000);
                mk += avg << 16;
                mk += avg << 8;
                avg += mk;
                func_001F5BB8_F69F0(x, y + top, scale * 24.0f, scale * 16.0f, g[*s].u, g[*s].v, 24, 16, avg, tex);
            } else if (*s > 0x20) {
                func_001F5BB8_F69F0(x, y + top, size, size, g[*s].u, g[*s].v, 16, 16, color, tex);
            }
            x += IntToFloat(g[*s].adv) * scale;
        }
        i++;
        if (i == n) {
            break;
        }
        s++;
    } while (*s != 0);
}

LINKER_REMNANT("asm/remnants/text", func_001F6CE0);

extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);

/* func_001F6CF8/func_001F6D88/func_001F6E18 are func_001F68E8's family
   with a leading measure call: the same mode/table triple (1, 2, 3 and
   D_001DF3D0, D_001DF770, D_001DFB10), each paired with its own
   measuring helper, and the first argument stepped back by whatever that
   helper returns. */
void font_print_right(char *a, void *b, void *c, unsigned char *d, int e) {
    char *p = a - MeasureTextWidthRegular(d, e);
    long mode = GetEffectTex64(1);

    FontPrint(p, b, c, d, (void *)e, mode, D_001DF3D0);
}

/* Right-aligned FontPrint with the second font. */
void font_print_right_small(char *a, void *b, void *c, unsigned char *d, int e) {
    char *p = a - MeasureTextWidthSmall(d, e);
    long mode = GetEffectTex64(2);

    FontPrint(p, b, c, d, (void *)e, mode, D_001DF770);
}

/* Right-aligned FontPrint with the third font. */
void font_print_right_large(char *a, void *b, void *c, unsigned char *d, int e) {
    char *p = a - MeasureTextWidthLarge(d, e);
    long mode = GetEffectTex64(3);

    FontPrint(p, b, c, d, (void *)e, mode, D_001DFB10);
}

/* func_001F6EA8/func_001F6F40/func_001F6FD8 are the func_001F6CF8 triple
   centred instead of left-aligned: the step-back is half the measured
   value, and the adjusted position is returned. Typed all-int to match
   the extern func_001F7288 already declares for func_001F6FD8. */
/* FontPrintCenter */
int FontPrintCenter(int a, int b, int c, int d, int e) {
    int p = a - (MeasureTextWidthRegular((unsigned char *)d, e) >> 1);
    long mode = GetEffectTex64(1);

    FontPrint((void *)p, (void *)b, (void *)c, (void *)d, (void *)e,
                  mode, D_001DF3D0);
    return p;
}

/* FontPrintCenterSmall */
int FontPrintCenterSmall(int a, int b, int c, int d, int e) {
    int p = a - (MeasureTextWidthSmall((unsigned char *)d, e) >> 1);
    long mode = GetEffectTex64(2);

    FontPrint((void *)p, (void *)b, (void *)c, (void *)d, (void *)e,
                  mode, D_001DF770);
    return p;
}

/* FontPrintCenterLarge */
int FontPrintCenterLarge(int a, int b, int c, int d, int e) {
    int p = a - (MeasureTextWidthLarge((unsigned char *)d, e) >> 1);
    long mode = GetEffectTex64(3);

    FontPrint((void *)p, (void *)b, (void *)c, (void *)d, (void *)e,
                  mode, D_001DFB10);
    return p;
}

INCLUDE_ASM("asm/nonmatchings/text", func_001F7070); /* FontPrintWindow */

extern int func_001F4868(int);

/* func_001F7560 and func_001F75D0 are the same call with a different
   mode (1 vs 2) and a different table. Six arguments: EABI passes the
   fifth and sixth in $8/$9, which is why they appear alongside $4-$7
   rather than on the stack. */
void Font_PrintWindowRegular(void *a, void *b, void *c, void *d) {
    int mode = GetEffectTex(1);

    FontPrintWindow(a, b, c, d, mode, D_001DF3D0);
}

/* FontPrintWindow with the second font. */
void Font_PrintWindowAlt(void *a, void *b, void *c, void *d) {
    int mode = GetEffectTex(2);

    FontPrintWindow(a, b, c, d, mode, D_001DF770);
}

LINKER_REMNANT("asm/remnants/text", func_001F7640);

/* FontSetWindow */
void FontSetWindow(void *arg0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    short *s = (short *)arg0;
    s[0] = a1;
    s[1] = a2;
    s[2] = a3;
    s[3] = a4;
    s[4] = a5;
    s[5] = a6;
    s[8] = a7;
    s[9] = a8;
    s[6] = 0;
    s[7] = 0;
    s[10] = 0;
    s[11] = 0;
}

extern int D_0015EF84 MACRO_ADDR;
extern int D_0013E604[];
#define PK ((unsigned char *)D_00161000)

/* Uploads a block to the GS in DMA packets. */
void UploadToGs(unsigned int source) {
    int remaining = D_0013E604[0];
    int destination = D_0015EF84 >> 8;
    unsigned long count;
    unsigned long transfer;
    unsigned char *packet;
    unsigned long *words;

    do {
        count = (long)remaining;
        if (count > 0x80)
            count = 0x80;
        *(unsigned int *)(PK + 0) = 0x10000006;
        remaining -= 0x80;
        *(unsigned int *)(PK + 4) = 0;
        *(unsigned int *)(PK + 8) = 0;
        *(unsigned int *)(PK + 12) = 0x50000006;
        packet = PK;
        D_00161000 = (int *)(packet + 0x10);
        transfer = (unsigned long)count << 7;
        words = (unsigned long *)(packet + 0x10);
        words[0] = 0x4000000000000001UL;
        words[1] = 0x0eeeeeee;
        words[2] = ((unsigned long)(unsigned int)destination << 32) | 0x8000000000000UL;
        words[3] = 0x50;
        words[4] = 0;
        words[5] = 0x51;
        words[6] = ((unsigned long)count << 32) | 0x200;
        words[7] = 0x52;
        words[8] = 0;
        words[9] = 0x53;
        words[10] = transfer | 0x0800000000008000UL;
        words[11] = 0;
        D_00161000 = (int *)(packet + 0x70);
        *(unsigned int *)(packet + 0x70) = (unsigned int)(long)(int)(transfer | 0x30000000);
        *(unsigned int *)(PK + 4) = source;
        source += (int)(count * 0x800);
        destination += (int)(count * 8);
        *(unsigned int *)(PK + 8) = 0;
        *(unsigned int *)(PK + 12) = (unsigned int)transfer | 0x50000000;
        D_00161000 = (int *)(PK + 0x10);
    } while (remaining > 0);
    VU1_texFlush();
}

extern short D_0015F448;              /* SDA, gp -0x78B8 */
extern unsigned short D_0010E800 NOT_SDA;
extern char D_0010E810[];
extern char D_00187180[];
extern void func_001FA1C0(float *, float);
extern void func_00234FA8(void);

/* Build the VU1 setup packet: load microprogram 7 if another is
   resident, then a DIRECT/UNPACK chain with the two camera matrices
   (each scaled to 1024 and biased in z by D_0015F448), the screen scale
   and offsets from D_0018CE00, and the GIF register setup; the DMA tag's
   qword count is patched in at the end. */
void Draw_BuildVu1Setup(void) {
    float m[16];
    char *cam;
    char *g;
    int *base;
    int *p;

    func_001FA1C0(m, 1024.0f);
    cam = D_00187180;
    FastVecScale(&m[12], cam, -1024.0f);
    m[15] = 1.0f;
    if (D_0015F704 != 7) {
        VU1_addDataRef(D_0010E810, D_0010E800);
        D_0015F704 = 7;
    }
    D_00161000[0] = 0x10000000;
    D_00161000[1] = 0;
    D_00161000[2] = 0x11000000;
    D_00161000[3] = 0x01000404;
    base = D_00161000;
    base[4] = 0;
    base[5] = 0;
    base[6] = 0;
    base[7] = 0x6C0C43A4;
    p = base + 8;
    sce_vu0_mul_matrix(p, cam - 0x100, m);
    *(float *)(p + 14) += *(float *)&D_0015F448;
    p = base + 0x18;
    sce_vu0_mul_matrix(p, cam - 0x80, m);
    *(float *)(p + 14) += *(float *)&D_0015F448;
    g = (char *)D_0018CE00;
    base[0x28] = 0x8000;
    base[0x29] = 0x303EC000;
    base[0x2A] = 0x412;
    *(float *)(base + 0x2B) = *(float *)(g + 0x210);
    p = base + 0x2C;
    qcopy(p, g + 0x190);
    p = base + 0x30;
    qcopy(p, g + 0x1A0);
    *(float *)(base + 0x34) = *(float *)(g + 0x22C);
    *(float *)(base + 0x35) = *(float *)(g + 0x228);
    base[0x36] = 0;
    base[0x37] = 0;
    base[0x38] = 0x03000000;
    base[0x39] = 0x020001D2;
    base[0x3A] = 0x15000000;
    base[0x3B] = 0;
    p = base + 0x3C;
    D_00161000[0] |= ((char *)p - (char *)D_00161000 >> 4) - 1;
    D_00161000 = p;
    Dma_AppendRefDirect11();
}

extern void func_001FB608(int, int, int);
extern short D_001519EE NOT_SDA;

/* Sets up a (1 << a) x (1 << b) area, as vendor.c's func_0023A948 does:
   func_001FB608 gets the sizes and a base address, which is D_001519EE
   pages when `flag` is set, else D_0015EF8C less 4 << min(a + b, 16)
   bytes rounded down to a page (8 KB); func_001F3760 gets the sizes and
   the float setup (f, 0, 524288, 255, 0); GS registers 0x47 (0 with the
   flag, 0x30000 without) and 0x42 are then written. Each arm makes its
   own page-aligned base, so the two trailing `<< 13`s are cross-jumped
   into the one retail has ahead of the argument moves. */
void Framebuf_SetupRenderTarget(int a, int b, int flag, float f) {
    int base;

    if (flag != 0) {
        base = D_001519EE << 13;
    } else {
        int t = a + b;

        if (t > 16) {
            t = 16;
        }
        base = D_0015EF8C - (4 << t);
        base = (base >> 13) << 13;
    }
    Framebuf_SetRenderTarget(a, b, base);
    SetScreenSize(1 << a, 1 << b, f, 0.0f, 524288.0f, 255.0f, 0.0f);
    if (flag != 0) {
        VU1_addGSregister(0x47, 0);
    } else {
        VU1_addGSregister(0x47, 0x30000);
    }
    VU1_addGSregister(0x42, 0x8000000044L);
}

__asm__(".section .text\n\tnop\n");

extern void func_001F3008(void);

/* Frame start for the large draw buffer. */
void BeginDrawFrame(void) {
    PutDrawBufferLarge();
    InitViewContext();
    UpdateViewContext();
}

/* Two prototypes for one symbol: func_001F55C0 passes a 64-bit value
   (retail shifts it with dsll), func_001F7B70 passes plain ints
   (addiu, not daddiu). */
extern int D_0015F578 MACRO_ADDR;
extern short D_0015F448;              /* SDA, gp -0x78B8 */
extern void func_001F91B8(void);
extern void func_001F7868(void);
extern void func_001F8B6C(void);

/* Optional extra draw pass, enabled by D_0015F578. */
void DrawOptionalExtraPass(void) {
    if (D_0015F578 != 0) {
        VU1_addGSregister(8, 5);
        VU1_addGSregister(0x14, 0x61);
        VU1_addGSregister(0x47, 0x513F1);
        VU1_addGSregister(0x4A, 1);
        func_001F91B8();
        *(float *)&D_0015F448 = -0.04f;
        Draw_BuildVu1Setup();
        func_001F8B6C();
        *(int *)&D_0015F448 = 0;
        VU1_addGSregister(0x4A, 0);
    }
}

extern int D_0018E840[];

/* buildBitSwapLut: fills the alpha lookup table D_0018E840. */
void buildBitSwapLut(void) {
    int i;
    for (i = 0; i < 0x100; i++) {
        int v = i & 0xE7;
        if (i & 0x8)  v |= 0x10;
        if (i & 0x10) v |= 0x8;
        D_0018E840[v] = (i >> 1) << 24;
    }
}

LINKER_REMNANT("asm/remnants/text", func_001F7C50);
