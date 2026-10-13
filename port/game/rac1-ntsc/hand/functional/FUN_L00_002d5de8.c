/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002D7298.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* FUN_L00_002d5de8 -- src/overlays/shared/vendor_002D1168.c (functional C for the port, not a match)
 * Infobot update (class 750, levels 0, 1, 3-8, 10, 12-15, 17). State 0 init (rest yaw, home, hidden
 * unless shown; ride cuboid -> 5; with paths: their segment lengths measured, placed on path 0 -> 3);
 * 1 nothing; 2 waiting (springs to the rest yaw, glow, deleted once its planet is unlocked, Ratchet
 * within 1 (3) with health -> 8); 3 at a path start (turns to Ratchet, within 8 -> 4 and its talk
 * flag); 4 flies the path (func_L00_002D5988); 5 orbits its ride's cuboid until the ride (class 0x336)
 * reaches state 6 -> 6 on its flight spline; 6 flies the spline (braking to its end) -> 7; 7 orbits the
 * end point, Ratchet within 2 -> 8; 8 collected (planet unlocked, mission done, checkpoint, scene);
 * 9 movie; 10 scene; 11 save, planet banner, deleted. After most states, on a ride cuboid with a
 * class-0x336 ride, the position is the cuboid's centre (carried) plus the orbit offset.
 * Adapted with ReRAC's infobot.rs (ISC), which documents the US twin (level01 0x2fbf80).
 * equiv: DIFFERENT by shape only: retail shares the roll spring's tail (and the pitch store in its
 * delay slot) between states 6 and 7 where this keeps one copy each (one more mul.s, one more
 * +0x44 store, the 2pi/3 constant's argument tag), one branch has the other polarity, and one
 * spline-count load is repeated. Every call, argument and float operation otherwise matches. */
typedef unsigned int Q_2D7298_c2d5de8 __attribute__((mode(TI), aligned(16)));
typedef struct {
    int shown;
    int planet;
    int checkpoint;
    signed char sceneA, movie, sceneB, padF;
    unsigned char glow[0x50 - 0x10];
    int attach;
    int ride;
    int flight;
    float orbit;
    float speed;
    float velX, velY, velZ;
    int paths[8];
    unsigned char pad90[0xA4 - 0x90];
    float radius;
    int path;
    float restYaw;
    int touch;
    unsigned char padB4[0xC0 - 0xB4];
    Q_2D7298_c2d5de8 home;
} Pv_2D7298_c2d5de8;
typedef struct {
    int n;
    unsigned char pad4[0xC];
    float pts[1][4];
} Spline_2D7298_c2d5de8;

extern Spline_2D7298_c2d5de8 *D_L00_001B0830_2D7298[] __asm__("D_L00_001B04B0");
extern unsigned char *D_L00_00160098_2D7298 SDATA(D_L00_0015FFD8);
extern char *D_L00_001601AC_2D7298 __asm__("D_L00_001600EC") MACRO_ADDR;
extern char *D_L00_001601AC_g_2D7298 SDATA(D_L00_001600EC);
extern unsigned char D_0013DE48_2D7298[] __asm__("D_0013DD40");
typedef struct {
    unsigned char pad0[0x80];
    float pos[4];
} Hero_2D7298_c2d5de8;
extern Hero_2D7298_c2d5de8 G_2D7298 __asm__("D_0013F350");
extern char D_0013F4D0_2D7298[] __asm__("D_0013F3D0");
extern float D_0015EE6C_2D7298 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE70_2D7298 __asm__("D_0015ED70") MACRO_ADDR;
extern float D_0015EE6C_g_2D7298 SDATA(D_0015ED6C);
extern float D_0015EE70_g_2D7298 SDATA(D_0015ED70);
extern int D_L00_0015F6A8_2D7298 __asm__("D_L00_0015F5C4") MACRO_ADDR;

extern void func_L00_002D80A0_2D7298(void *) __asm__("func_L00_002D6BF0");
extern void func_L00_002D8180_2D7298(void *) __asm__("func_L00_002D6CD0");
extern float func_001F9D10_2D7298(void *, void *) __asm__("func_001F9B48");
extern float func_L00_00259148_2D7298(float, float, float, float, float, float *) __asm__("func_L00_00258110");
extern float func_L00_001FF860_2D7298(float, float) __asm__("func_L00_001FF8B0");
extern void func_L00_002676A0_2D7298(void *, int) __asm__("func_L00_00266858");
extern void func_L00_002D6E38_2D7298(void *, void *, int) __asm__("func_L00_002D5988");
extern float func_001FA748_2D7298(float, float) __asm__("func_001FA580");
extern float func_001F9F90_2D7298(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_2D7298(float) __asm__("func_001F9DE0");
extern void func_L00_0025EFC0_2D7298(void *, void *, void *, int *, float *, int, float, float, float) __asm__("func_L00_0025DF68");
extern float func_001FA888_2D7298(int) __asm__("func_001FA6C0");
extern int func_001FA898_2D7298(float) __asm__("func_001FA6D0");
extern void func_001F9BF0_2D7298(void *, void *, void *) __asm__("func_001F9A28");
extern void func_001F9C30_2D7298(void *, void *, float) __asm__("func_001F9A68");
extern void func_001F9BD8_2D7298(void *, void *, void *) __asm__("func_001F9A10");
extern float func_001F9CB8_2D7298(void *) __asm__("func_001F9AF0");
extern void func_L00_001FF4B0_2D7298(void *, void *, float) __asm__("func_L00_001FF500");
extern float func_001FA790_2D7298(float, float) __asm__("func_001FA5C8");
extern void func_L00_00261848_2D7298(int) __asm__("func_L00_002607D0");
extern void func_L00_002512D8_2D7298(int) __asm__("func_L00_002502A0");
extern void func_L00_00286128_2D7298(void *, void *) __asm__("func_L00_00284E50");
extern void func_L00_00299B68_2D7298(int) __asm__("func_L00_00298840");
extern void func_L00_0029A7D0_2D7298(int) __asm__("func_L00_00299460");
extern void func_0020BFC8_2D7298(int, int) __asm__("func_0020B178");
extern void func_L00_00263DB0_2D7298(int) __asm__("func_L00_00262D38");
extern void func_0020D678_2D7298(void *) __asm__("func_0020C828");
extern void func_L00_00261478_2D7298(void *, void *, void *, void *, void *, void *) __asm__("func_L00_00260400");

#define SPRING_PITCH_2D7298(m, pv, target)                                                                  \
    *(float *)((m) + 0x44) = func_L00_00259148_2D7298(*(float *)((m) + 0x44), (target),                   \
                                                      D_0015EE70_2D7298 * 0.5235988f,                     \
                                                      D_0015EE70_2D7298 * 1.0471976f,                     \
                                                      D_0015EE6C_2D7298 * 0.7853982f, &(pv)->velY)
#define SPRING_ROLL_2D7298(m, pv, target)                                                                   \
    *(float *)((m) + 0x40) = func_L00_00259148_2D7298(*(float *)((m) + 0x40), (target),                   \
                                                      D_0015EE70_2D7298 * 1.0471976f,                     \
                                                      D_0015EE70_2D7298 * 2.0943952f,                     \
                                                      D_0015EE6C_2D7298 * 1.5707964f, &(pv)->velX)
/* the segment lengths of a spline (w of each point but the last two) */
#define PT_2D7298(P, i, c) (*(float *)((char *)(P) + ((i) << 4) + 0x10 + 4 * (c)))
#define PA_2D7298(P, i) ((char *)(P) + (((i) << 4) + 0x10))
#define MEASURE_2D7298(P)                                                                                   \
    for (j = 0; j < (P)->n - 2; j++) {                                                                      \
        (P)->pts[j][3] = func_001F9D10_2D7298((P)->pts[j], (P)->pts[j + 1]);                                \
    }

void FUN_L00_002d5de8(unsigned char *m) {
    float off[4];
    float v10[4];
    float v20[4];
    int seg;
    float t;
    Pv_2D7298_c2d5de8 *pv = *(Pv_2D7298_c2d5de8 **)(m + 0x78);
    Spline_2D7298_c2d5de8 *P;
    unsigned char *o;
    float r, a, d, s, rx, ry;
    int k, j;

    *(Q_2D7298_c2d5de8 *)off = 0;
    switch (m[0x20]) {
    case 0:
        m[0x30] = 0xFF;
        pv->restYaw = *(float *)(m + 0x48);
        pv->home = *(Q_2D7298_c2d5de8 *)(m + 0x10);
        if (pv->shown != 0) {
            m[0x20] = 2;
            func_L00_002D80A0_2D7298(m);
        } else {
            m[0x20] = pv->touch != 0 ? 2 : 1;
            *(unsigned short *)(m + 0x34) |= 0x41;
        }
        if (pv->attach != -1) {
            m[0x20] = 5;
            func_L00_002D80A0_2D7298(m);
            break;
        }
        if (pv->paths[0] == -1) {
            break;
        }
        for (k = 0; k < 8; k++) {
            if (pv->paths[k] == -1) {
                continue;
            }
            P = D_L00_001B0830_2D7298[pv->paths[k]];
            MEASURE_2D7298(P);
        }
        *(Q_2D7298_c2d5de8 *)(m + 0x10) = *(Q_2D7298_c2d5de8 *)D_L00_001B0830_2D7298[pv->paths[0]]->pts[0];
        pv->path = 0;
        m[0x20] = 3;
        break;
    case 1:
        break;
    case 2:
        r = 1.0f;
        if (pv->touch != 0 && (*(unsigned short *)(m + 0x34) & 1)) {
            r = 3.0f;
        }
        if (pv->planet == 12) {
            r = 3.0f;
        }
        *(float *)(m + 0x48) = func_L00_00259148_2D7298(*(float *)(m + 0x48), pv->restYaw, 0.01f, 0.3f, 0.1f, &pv->velZ);
        SPRING_PITCH_2D7298(m, pv, 0.0f);
        SPRING_ROLL_2D7298(m, pv, 0.0f);
        func_L00_002D8180_2D7298(m);
        if (D_0013DE48_2D7298[pv->planet] != 0) {
            func_0020D678_2D7298(m);
            return;
        }
        if (func_001F9D10_2D7298(D_0013F4D0_2D7298, m + 0x10) < r && *(int *)(D_0013F4D0_2D7298 + 0x2228) != 0) {
            m[0x20] = 8;
        }
        break;
    case 3:
        func_L00_002D8180_2D7298(m);
        if (D_0013DE48_2D7298[pv->planet] != 0) {
            func_0020D678_2D7298(m);
            return;
        }
        if (pv->paths[pv->path] == -1) {
            m[0x20] = 2;
            break;
        }
        a = func_L00_001FF860_2D7298(G_2D7298.pos[0] - *(float *)(m + 0x10), G_2D7298.pos[1] - *(float *)(m + 0x14));
        *(float *)(m + 0x48) = func_L00_00259148_2D7298(*(float *)(m + 0x48), a, 0.01f, 0.3f, 0.1f, &pv->velZ);
        SPRING_PITCH_2D7298(m, pv, 0.0f);
        SPRING_ROLL_2D7298(m, pv, 0.0f);
        if (func_001F9D10_2D7298(G_2D7298.pos, m + 0x10) < 8.0f) {
            m[0x20] = 4;
            func_L00_002676A0_2D7298(m, 1);
        }
        break;
    case 4:
        func_L00_002D6E38_2D7298(m, D_L00_001B0830_2D7298[pv->paths[pv->path]], 3);
        func_L00_002D8180_2D7298(m);
        if (m[0x20] != 4) {
            pv->path = pv->path + 1;
        }
        break;
    case 5:
        func_L00_002D8180_2D7298(m);
        if (D_0013DE48_2D7298[pv->planet] != 0) {
            func_0020D678_2D7298(m);
            return;
        }
        a = func_001FA748_2D7298(pv->orbit, D_0015EE6C_g_2D7298 * 3.1415927f);
        pv->orbit = a;
        off[0] = func_001F9F90_2D7298(a);
        off[1] = func_001F9FA8_2D7298(pv->orbit);
        off[2] = 3.0f;
        *(float *)(m + 0x48) = func_001FA748_2D7298(pv->orbit, 1.5707964f);
        if (pv->ride == -1) {
            break;
        }
        o = D_L00_00160098_2D7298 + (pv->ride << 8);
        if (*(short *)(o + 0xA6) != 0x336 || o[0xBC] != 6 || pv->flight == -1) {
            break;
        }
        m[0x20] = o[0xBC];
        pv->attach = -1;
        *(Q_2D7298_c2d5de8 *)(m + 0x10) = *(Q_2D7298_c2d5de8 *)D_L00_001B0830_2D7298[pv->flight]->pts[0];
        P = D_L00_001B0830_2D7298[pv->flight];
        *(float *)(m + 0x48) = func_L00_001FF860_2D7298(P->pts[1][0] - P->pts[0][0], P->pts[1][1] - P->pts[0][1]);
        P = D_L00_001B0830_2D7298[pv->flight];
        MEASURE_2D7298(P);
        break;
    case 6:
        P = D_L00_001B0830_2D7298[pv->flight];
        func_L00_002D8180_2D7298(m);
        func_L00_0025EFC0_2D7298(P, m + 0x10, v20, &seg, &t, 0, 999.0f, 5.0f, 0.0f);
        d = func_001FA888_2D7298(seg) * P->pts[0][3] + t + 2.0f;
        a = func_001F9D10_2D7298(m + 0x10, P->pts[P->n - 1]);
        s = D_0015EE70_2D7298 * 16.0f;
        r = pv->speed;
        if (a <= r * r / (s + s)) {
            r = r - s;
            pv->speed = r;
            if (r < 0.0f) {
                pv->speed = 0.0f;
            }
        } else {
            a = D_0015EE6C_g_2D7298 * 16.0f;
            r = r + s;
            pv->speed = r;
            if (a < r) {
                pv->speed = a;
            }
        }
        seg = func_001FA898_2D7298(d / P->pts[0][3]);
        t = (d - func_001FA888_2D7298(seg) * P->pts[0][3]) / P->pts[0][3];
        if (!(seg < P->n - 1)) {
            if (pv->speed <= D_0015EE70_2D7298 * 16.0f) {
                m[0x20] = 7;
                *(Q_2D7298_c2d5de8 *)(m + 0x10) = *(Q_2D7298_c2d5de8 *)P->pts[P->n - 1];
                break;
            }
            seg = P->n - 2;
            t = 1.0f;
        }
        func_001F9BF0_2D7298(v20, PA_2D7298(P, seg + 1), PA_2D7298(P, seg));
        func_001F9C30_2D7298(v20, v20, t);
        func_001F9BD8_2D7298(v20, v20, PA_2D7298(P, seg));
        a = func_L00_001FF860_2D7298(PT_2D7298(P, seg + 1, 0) - PT_2D7298(P, seg, 0),
                                     PT_2D7298(P, seg + 1, 1) - PT_2D7298(P, seg, 1));
        *(float *)(m + 0x48) = func_L00_00259148_2D7298(*(float *)(m + 0x48), a, 0.01f, 0.3f, 0.1f, &pv->velZ);
        func_001F9BF0_2D7298(v10, v20, m + 0x10);
        s = func_001F9CB8_2D7298(v10);
        if (pv->speed < s) {
            s = pv->speed;
        }
        func_L00_001FF4B0_2D7298(v10, v10, s);
        func_001F9BD8_2D7298(m + 0x10, m + 0x10, v10);
        a = func_001FA790_2D7298(func_L00_001FF860_2D7298(v10[0], v10[1]), *(float *)(m + 0x48));
        ry = pv->speed * 0.34906584f * func_001F9F90_2D7298(a) / (D_0015EE6C_2D7298 * 16.0f);
        rx = pv->speed * -0.34906584f * func_001F9FA8_2D7298(a) / (D_0015EE6C_2D7298 * 16.0f);
        SPRING_PITCH_2D7298(m, pv, ry);
        SPRING_ROLL_2D7298(m, pv, rx);
        break;
    case 7:
        P = D_L00_001B0830_2D7298[pv->flight];
        func_L00_002D8180_2D7298(m);
        pv->radius = pv->radius + D_0015EE6C_2D7298;
        if (1.0f < pv->radius) {
            pv->radius = 1.0f;
        }
        a = func_001FA748_2D7298(pv->orbit, D_0015EE6C_2D7298 * 3.1415927f);
        pv->orbit = a;
        off[0] = func_001F9F90_2D7298(a) * pv->radius;
        off[1] = func_001F9FA8_2D7298(pv->orbit) * pv->radius;
        *(int *)&off[2] = 0;
        a = func_001FA748_2D7298(pv->orbit, 1.5707964f);
        *(float *)(m + 0x48) = func_L00_00259148_2D7298(*(float *)(m + 0x48), a, D_0015EE6C_2D7298 * 25.132742f,
                                                        D_0015EE6C_2D7298 * 25.132742f,
                                                        D_0015EE6C_2D7298 * 12.566371f, &pv->velZ);
        func_001F9BD8_2D7298(v10, off, P->pts[P->n - 1]);
        *(Q_2D7298_c2d5de8 *)(m + 0x10) = *(Q_2D7298_c2d5de8 *)v10;
        if (func_001F9D10_2D7298(D_0013F4D0_2D7298, P->pts[P->n - 1]) < 2.0f &&
            *(int *)(D_0013F4D0_2D7298 + 0x2228) != 0) {
            m[0x20] = 8;
        }
        SPRING_PITCH_2D7298(m, pv, 0.0f);
        SPRING_ROLL_2D7298(m, pv, 0.0f);
        break;
    case 8:
        *(unsigned short *)(m + 0x34) |= 0x41;
        func_L00_00261848_2D7298(pv->planet);
        func_L00_002512D8_2D7298(m[0xB0]);
        if (pv->checkpoint != -1) {
            char *c = D_L00_001601AC_g_2D7298 + (pv->checkpoint << 7);
            func_L00_00286128_2D7298(c + 0x30, c + 0x70);
        }
        m[0x20] = 9;
        if (pv->sceneA != -1) {
            func_L00_00299B68_2D7298(pv->sceneA);
        }
        break;
    case 9:
        if (D_L00_0015F6A8_2D7298 == 2) {
            break;
        }
        m[0x20] = 10;
        if (pv->movie != -1) {
            func_L00_0029A7D0_2D7298(pv->movie);
        }
        break;
    case 10:
        if (D_L00_0015F6A8_2D7298 == 2) {
            break;
        }
        if (pv->sceneB != -1) {
            func_L00_00299B68_2D7298(pv->sceneB);
        }
        m[0x20] = 11;
        break;
    case 11:
        if (D_L00_0015F6A8_2D7298 == 2) {
            return;
        }
        func_0020BFC8_2D7298(0, -1);
        func_L00_00263DB0_2D7298(pv->planet);
        func_0020D678_2D7298(m);
        return;
    }

    /* on a ride: the attach cuboid's centre (carried by the ride) plus the orbit offset */
    if (pv->attach == -1 || pv->ride == -1) {
        return;
    }
    o = D_L00_00160098_2D7298 + (pv->ride << 8);
    if (*(short *)(o + 0xA6) != 0x336) {
        return;
    }
    *(Q_2D7298_c2d5de8 *)v10 = 0;
    func_L00_00261478_2D7298(m, o, D_L00_001601AC_2D7298 + (pv->attach << 7) + 0x30, v10,
                             D_L00_001601AC_2D7298 + (pv->attach << 7) + 0x30, v10);
    func_001F9BD8_2D7298(v20, off, D_L00_001601AC_2D7298 + (pv->attach << 7) + 0x30);
    *(Q_2D7298_c2d5de8 *)(m + 0x10) = *(Q_2D7298_c2d5de8 *)v20;
}
