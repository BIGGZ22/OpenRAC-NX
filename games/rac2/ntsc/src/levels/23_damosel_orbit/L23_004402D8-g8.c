#include "common.h"

/* func_L23_004402D8 is a unit of its own: it reads $gp (hence -g8), and it is built with -g,
   whose line labels make the assembler pad its two divisions with two nops each, as retail
   has them (the same effect as func_L23_003BF9B0). Each division stays on a line of its own.
   The structs are partial layouts measured from this function.
   Measured on this body:
   - the plain (unmarked) externs D_L23_001B2144, D_L23_001A8FF0, D_001A79F0 and D_L23_001B234C
     come out absolute (`lui`+op) outside delay slots and $gp-relative inside them, as retail;
     the floats marked sda are $gp-relative everywhere;
   - the entry address is an integer sum with the scaled index first, which gives retail's
     `addu $a2,$v1,$v0` (a pointer sum puts the table first);
   - the slot scan is a do-while: with a for loop, cc1 rolls the `< 0` break to the bottom and
     duplicates it before the loop;
   - each pair of interpolated values is computed before the two calls that take them;
   - the block that ends in `return` is written inside the loop: cc1 itself moves it behind
     the last clamp, where retail has it. */

typedef struct Vars_004402D8 {
    u8 pad00[0x20];
    s32 f20; /* 0x20 */
    f32 f24; /* 0x24 */
    s32 f28[6]; /* 0x28 */
    f32 f40; /* 0x40 */
    f32 f44; /* 0x44 */
    f32 f48; /* 0x48 */
    f32 f4C; /* 0x4C */
    s32 f50; /* 0x50 */
} Vars_004402D8;

typedef struct Entry_004402D8 {
    f32 x; /* 0x00 */
    f32 y; /* 0x04 */
    f32 z; /* 0x08 */
    u8 pad0C[0x10];
    Vars_004402D8 *vars; /* 0x1C */
} Entry_004402D8;

typedef struct Moby_004402D8 {
    u8 pad00[0x84];
    s16 f84; /* 0x84 */
} Moby_004402D8;

typedef struct Player_004402D8 {
    u8 pad000[0x80];
    f32 f80[4]; /* 0x080 */
    u8 pad090[0x20];
    f32 fB0[4]; /* 0x0B0 */
    u8 pad0C0[0xB0];
    f32 f170; /* 0x170 */
    u8 pad174[0x32A];
    u8 f49E; /* 0x49E */
    u8 pad49F[0x81];
    f32 f520; /* 0x520 */
    u8 pad524[0xF30];
    s16 f1454; /* 0x1454 */
    u8 pad1456[0xE3A];
    u8 *f2290; /* 0x2290 */
    s32 f2294; /* 0x2294 */
    u8 pad2298[0x4];
    s32 f229C; /* 0x229C */
    u8 pad22A0[0x14];
    u8 f22B4; /* 0x22B4 */
} Player_004402D8;

typedef struct Pad_004402D8 {
    u8 pad000[0x100];
    f32 f100; /* 0x100 */
    f32 f104; /* 0x104 */
} Pad_004402D8;

typedef struct CamSub_004402D8 {
    u8 pad000[0x250];
    s32 f250; /* 0x250 */
} CamSub_004402D8;

typedef struct Cam_004402D8 {
    u8 pad00[0x70];
    CamSub_004402D8 *f70; /* 0x70 */
    u8 pad74[0x12];
    s16 f86; /* 0x86 */
} Cam_004402D8;

typedef struct CamMgr_004402D8 {
    u8 pad000[0x140];
    f32 f140[4]; /* 0x140 */
    u8 pad150[0x40];
    Cam_004402D8 *cam; /* 0x190 */
} CamMgr_004402D8;

extern Entry_004402D8 *D_L23_001B2144;

extern s32 D_L23_001A8FF0;

extern s32 D_001A79F0;

extern s32 D_0018C0B4 __attribute__((nosda));

extern Player_004402D8 D_00189E20;

extern Pad_004402D8 D_00138180;

extern CamMgr_004402D8 D_L23_001BA500;

extern s32 D_L23_001B234C;

extern f32 D_L23_001B1D6C __attribute__((sda));

extern f32 D_L23_001B1EEC __attribute__((sda));

extern f32 D_L23_001B1EF4 __attribute__((sda));

extern f32 D_L23_001B1EF8 __attribute__((sda));

extern f32 D_L23_001B1EFC __attribute__((sda));

extern f32 D_L23_001B1F00 __attribute__((sda));

extern f32 D_L23_001B1F04 __attribute__((sda));

extern f32 D_L23_001B1F08 __attribute__((sda));

extern f32 D_L23_001B1F0C __attribute__((sda));

extern f32 D_L23_001B1F10 __attribute__((sda));

extern f32 D_L23_001B1F14 __attribute__((sda));

extern f32 D_L23_001B1F18 __attribute__((sda));

extern f32 D_L23_001B1F1C __attribute__((sda));

extern f32 D_L23_001B1F20 __attribute__((sda));

extern f32 func_00282E48(void *, void *);

extern f32 func_002B0360(void *);

extern s32 func_L00_00424940(void *);

extern s32 func_L00_00424978(void *);

extern s32 func_002ACBC8(void *, s32);

extern f32 func_00282C48(f32);

extern void func_L00_00424C00(void);

extern void func_L00_00424B48(s32);

extern void func_L00_00424C28(void);

extern void func_L00_00424B70(f32, f32);

extern s32 func_00282948(void *);

extern void func_L00_00424F00(void *, f32, f32);

extern f32 func_L00_002D2A10(f32, f32, f32);

extern s32 func_00283CF0(f32);

extern void func_L16_0042FB58(f32, f32);

extern void func_L00_00424BB8(f32, f32);

extern void func_L00_00424A88(s32, f32, f32);

extern void func_L00_00424A08(s32, f32, f32);

extern void func_L00_00424A50(f32, f32);

extern void func_L00_00424B20(f32);

void func_L23_004402D8(Moby_004402D8 *moby)
{
    Entry_004402D8 *entry = (Entry_004402D8 *)(moby->f84 * 32 + (s32)D_L23_001B2144);
    Vars_004402D8 *vars = entry->vars;
    s32 i;

    if (vars->f20 == 0) {
        if (D_L23_001A8FF0 != 0 || (D_001A79F0 == 16 && D_0018C0B4 == 0x81)) {
            f32 tmp[4];

            tmp[0] = entry->x;
            tmp[1] = entry->y;
            tmp[2] = entry->z;
            tmp[3] = 0.0f;
            vars->f24 = func_00282E48(tmp, D_00189E20.fB0);
            vars->f20 = 1;
        }
        return;
    }
    if (D_L23_001BA500.cam->f86 != 0
        || (D_L23_001A8FF0 == 0 && (D_001A79F0 != 16 || D_0018C0B4 != 0x81))) {
        if (D_L23_001B1F20 < func_002B0360(D_L23_001BA500.f140)) {
            D_L23_001B234C = 3;
        } else {
            D_L23_001B234C = 0;
        }
        func_L00_00424940(moby);
        return;
    }
    if (func_L00_00424978(moby) <= 0) {
        return;
    }
    i = 0;
    do {
        if (vars->f28[i] < 0) {
            break;
        }
        if (func_002ACBC8(D_00189E20.f80, vars->f28[i]) != 0) {
            func_L16_0042FB58(0.04f, 0.2f);
            func_L00_00424A08(0, vars->f40, 0.003f);
            func_L00_00424A50(vars->f44, 0.003f);
            return;
        }
        i++;
    } while (i < 6);
    if (D_00189E20.f22B4 == 2) {
        if (D_00189E20.f1454 != 0 || D_00189E20.f2294 == 0x5B
            || func_00282C48(D_00138180.f100) >= 0.3f || func_00282C48(D_00138180.f104) >= 0.3f) {
            func_L00_00424C00();
            func_L00_00424B48(0);
        }
    } else {
        func_L00_00424C00();
        func_L00_00424B48(0);
    }
    func_L00_00424C28();
    func_L00_00424B70(D_L23_001B1EF4, 0.2f);
    if (func_00282948(&vars->f50) != 0 && !(D_00189E20.f170 > 0.005f)) {
        func_L00_00424F00(D_00189E20.f2290 + 0xC0, 0.0052359886f, 0.0f);
    }
    if (func_00282C48(D_00138180.f100) >= 0.3f || func_00282C48(D_00138180.f104) >= 0.3f) {
        vars->f50 = 300;
    }
    if (D_00189E20.f22B4 == 2) {
        s32 n;

        D_L23_001BA500.cam->f70->f250 = 2;
        n = 0;
        if (D_00189E20.f49E != 0) {
            n = func_00283CF0(func_L00_002D2A10(90.0f, D_00189E20.f520, -1.0f));
        }
        if (D_00189E20.f2294 == 0x5B && (D_00189E20.f49E == 0 || n >= 20)) {
            func_L00_00424B70(0.5f, 1.0f);
            func_L16_0042FB58(0.05f, 0.2f);
            func_L00_00424BB8(0.04f, 0.0f);
            func_L00_00424A88(0, D_L23_001B1F14, 0.005f);
            func_L00_00424A08(1, D_L23_001B1F08, 0.01f);
            func_L00_00424A50(D_L23_001B1F0C, 0.01f);
            D_L23_001B1D6C = D_L23_001B1F1C * 0.017453294f;
            func_L00_00424B20(D_L23_001B1F18 * 0.017453294f);
        } else {
            f32 d;
            f32 t;
            f32 a;
            f32 b;

            func_L00_00424A88(0, D_L23_001B1F10, 0.03f);
            if (D_00189E20.f229C == 2) {
                func_L16_0042FB58(0.04f, 0.2f);
            } else {
                func_L16_0042FB58(0.015f, 0.2f);
            }
            d = func_00282E48(D_00189E20.f80, D_00189E20.fB0) - vars->f24;
            if (D_L23_001B1EEC < d) {
                d = D_L23_001B1EEC;
            } else if (d < 0.0f) {
                d = 0.0f;
            }
            t = d / D_L23_001B1EEC;
            a = (D_L23_001B1F00 - D_L23_001B1EF8) * t + D_L23_001B1EF8;
            b = (D_L23_001B1F04 - D_L23_001B1EFC) * t + D_L23_001B1EFC;
            func_L00_00424A08(1, a, 0.005f);
            func_L00_00424A50(b, 0.005f);
        }
        if (D_L23_001B1F20 < func_002B0360(D_L23_001BA500.f140)) {
            D_L23_001B234C = 3;
        } else {
            D_L23_001B234C = 0;
        }
    } else if (D_00189E20.f2294 == 0x81) {
        func_L16_0042FB58(0.05f, 0.2f);
        func_L00_00424BB8(0.04f, 0.0f);
        if (D_001A79F0 == 16) {
            func_L00_00424A88(0, 0.5f, 0.04f);
            func_L00_00424A08(0, 4.0f, 0.002f);
            func_L00_00424A50(2.0f, 0.04f);
        } else {
            func_L00_00424A88(0, 1.0f, 0.04f);
            func_L00_00424A08(0, 2.0f, 0.002f);
            func_L00_00424A50(7.0f, 0.04f);
        }
    } else {
        if (D_00189E20.f229C == 2) {
            func_L16_0042FB58(0.04f, 0.2f);
        } else {
            func_L16_0042FB58(0.015f, 0.2f);
        }
        if (D_00189E20.f229C == 5) {
            func_L00_00424A08(0, 4.0f, 0.003f);
            func_L00_00424A50(6.0f, 0.003f);
        } else {
            f32 d;
            f32 t;
            f32 a;
            f32 b;

            d = func_00282E48(D_00189E20.f80, D_00189E20.fB0) - vars->f24;
            if (D_L23_001B1EEC < d) {
                d = D_L23_001B1EEC;
            } else if (d < 0.0f) {
                d = 0.0f;
            }
            t = d / D_L23_001B1EEC;
            a = (vars->f48 - vars->f40) * t + vars->f40;
            b = (vars->f4C - vars->f44) * t + vars->f44;
            func_L00_00424A08(0, a, 0.003f);
            func_L00_00424A50(b, 0.003f);
        }
    }
}
