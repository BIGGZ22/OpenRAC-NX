/* functional: high. Written from the retail instructions; register allocation, frame layout and scheduling are the compiler's, while every read, write, call, argument and constant follows the asm in the same order. */
/* func_L04_002D9270 (level 4, Eudora; UpdateMoby_642): update of a drifting moby (a leaf or debris piece
 * blown around near D_L04_00166FC0). State 0 remembers its position, draws a timer (0x78..0x960 ticks) and a
 * speed (pvar 0x4C = speed * rand(0.8, 1.2)), then waits in state 1. State 1, within 32 units of
 * D_L04_00166FC0 and once its timer runs out, sets alpha/flags, builds a random velocity and spin from the
 * wind data at D_L04_001CBBC0 (direction +4, strength +0x14), redraws the timer (0x2D0..0x4B0) and goes to
 * state 2. State 2 jitters and integrates the spin and velocity, bounces both off collision
 * (func_L00_001EFFF0), clamps the speed, keeps a falling floor on z, eases the rotation toward the spin,
 * fades the speed (+0x2C) with the timer and, when the timer ends, returns to the remembered position
 * and state 1. Other states do nothing. */
#include "common.h"

typedef int DriftWide_2D9270 __attribute__((mode(TI)));
typedef union { DriftWide_2D9270 quad; float f[4]; } DriftVec_2D9270;
typedef struct {
    DriftVec_2D9270 vel;   /* 0x00 */
    DriftVec_2D9270 spin;  /* 0x10 */
    float rot_target[4];   /* 0x20 */
    DriftVec_2D9270 home;  /* 0x30 */
    s32 timer_start;       /* 0x40 */
    s32 timer;             /* 0x44 */
    char pad48[4];
    float speed;           /* 0x4C */
} DriftData_2D9270;
typedef struct {
    char pad0[0x10];
    DriftVec_2D9270 pos;   /* 0x10 */
    u8 state;              /* 0x20 */
    char pad21[0xB];
    float speed;           /* 0x2C */
    u8 alpha;              /* 0x30 */
    char pad31;
    s16 flags32;           /* 0x32 */
    char pad34[0xC];
    float rot[4];          /* 0x40 */
    char pad50[0x28];
    DriftData_2D9270 *data; /* 0x78 */
} DriftMoby_2D9270;
typedef struct { char pad0[4]; float dir; char pad8[0xC]; float strength; } DriftWind_2D9270;

extern DriftWind_2D9270 D_L04_001CBBC0;
extern char D_L04_00166FC0[];
extern char D_L04_00174080[];
extern DriftVec_2D9270 D_L04_00174060;
extern float D_0015EE6C MACRO_ADDR;

extern float func_002140F8_2D9270(float, float) __asm__("func_002140F8");
extern float func_001F9CE8_2D9270(void *) __asm__("func_001F9CE8");
extern float func_001F9D48_2D9270(void *, void *) __asm__("func_001F9D48");
extern int func_001F9908_2D9270(s32 *) __asm__("func_001F9908");
extern void func_L00_001FF4B0_2D9270(void *, void *, float) __asm__("func_L00_001FF4B0");
extern float func_001F9F90_2D9270(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2D9270(float) __asm__("func_001F9FA8");
extern void func_001F9BD8_2D9270(void *, void *, void *) __asm__("func_001F9BD8");
extern int func_L00_001EFFF0_2D9270(void *, void *, int, int, int) __asm__("func_L00_001EFFF0");
extern int func_002140B0_2D9270(int) __asm__("func_002140B0");
extern void func_L00_001FF610_2D9270(void *, void *, void *) __asm__("func_L00_001FF610");
extern void func_001F9C30_2D9270(void *, void *, float) __asm__("func_001F9C30");
extern float func_001FA748_2D9270(float, float) __asm__("func_001FA748");
extern s32 func_001F9850_2D9270(s32) __asm__("func_001F9850");
extern float func_001FA888_2D9270(s32) __asm__("func_001FA888");
extern float func_001F9878_2D9270(float) __asm__("func_001F9878");
extern s32 func_L00_00258BC8_2D9270(s32, s32) __asm__("func_L00_00258BC8");

void func_L04_002D9270(DriftMoby_2D9270 *m) {
    DriftVec_2D9270 jitter;   /* sp+0x00 */
    DriftVec_2D9270 start;    /* sp+0x10 */
    DriftVec_2D9270 tmp;      /* sp+0x20 */
    DriftVec_2D9270 spin;     /* sp+0x30 */
    DriftVec_2D9270 rnd;      /* sp+0x40 */
    DriftData_2D9270 *pv = m->data;
    DriftWind_2D9270 *w;
    float len, f, lim, z;
    s32 r, a, b;

    start.quad = 0;
    start.f[0] = func_002140F8_2D9270(-1.0f, 1.0f);
    start.f[1] = func_002140F8_2D9270(-1.0f, 1.0f);
    start.f[2] = func_002140F8_2D9270(-1.0f, 1.0f);
    jitter.quad = start.quad;
    len = func_001F9CE8_2D9270(pv);
    start.quad = m->pos.quad;

    switch (m->state) {
    case 0:
        pv->home.quad = m->pos.quad;
        r = func_L00_00258BC8_2D9270(0x78, 0x960);
        pv->timer_start = r;
        pv->timer = r;
        f = func_002140F8_2D9270(0.8f, 1.2f);
        pv->speed = m->speed * f;
        m->state = 1;
        m->speed = 0.0f;
        return;

    case 1:
        if (!(func_001F9D48_2D9270(&m->pos, D_L04_00166FC0) < 32.0f)) return;
        if (func_001F9908_2D9270(&pv->timer) == 0) return;
        rnd.quad = 0;
        rnd.f[0] = func_002140F8_2D9270(-1.0f, 1.0f);
        rnd.f[1] = func_002140F8_2D9270(-1.0f, 1.0f);
        f = func_002140F8_2D9270(-1.0f, 1.0f);
        m->flags32 = 0x40;
        m->alpha = 0xFF;
        m->speed = 0.0f;
        rnd.f[2] = f;
        w = &D_L04_001CBBC0;
        spin.quad = rnd.quad;
        pv = m->data;
        func_L00_001FF4B0_2D9270(&spin, &spin, w->strength / 5.0f);
        f = func_001F9F90_2D9270(w->dir);
        pv->vel.f[0] = w->strength * f;
        f = func_001F9FA8_2D9270(w->dir);
        pv->vel.f[1] = w->strength * f;
        pv->vel.f[2] = D_0015EE6C * -0.5f;
        func_L00_001FF4B0_2D9270(m->rot, &spin, 3.14159274f);
        func_L00_001FF4B0_2D9270(pv->rot_target, &spin, 0.0981747732f);
        func_L00_001FF4B0_2D9270(&pv->spin, &spin, w->strength * 0.5f);
        r = func_L00_00258BC8_2D9270(0x2D0, 0x4B0);
        pv->timer_start = r;
        pv->timer = r;
        m->state = 2;
        return;

    case 2:
        w = &D_L04_001CBBC0;
        func_001F9908_2D9270(&pv->timer);
        func_L00_001FF4B0_2D9270(&jitter, &jitter, w->strength / 10.0f);
        func_001F9BD8_2D9270(&pv->spin, &pv->spin, &jitter);
        func_L00_001FF4B0_2D9270(&tmp, &pv->spin, 5.0f);
        func_L00_001FF4B0_2D9270(&pv->spin, &pv->spin, w->strength * 0.5f);
        func_001F9BD8_2D9270(&tmp, &tmp, &m->pos);
        if (func_L00_001EFFF0_2D9270(&start, &tmp, 6, 0, 0) != 0 && func_002140B0_2D9270(8) == 0) {
            func_L00_001FF610_2D9270(&pv->spin, &pv->spin, D_L04_00174080);
            pv->spin.f[2] = pv->spin.f[2] + w->strength / 10.0f;
        }
        func_001F9BD8_2D9270(pv, pv, &pv->spin);
        f = func_001F9F90_2D9270(w->dir);
        tmp.f[0] = w->strength * f;
        f = func_001F9FA8_2D9270(w->dir);
        tmp.f[1] = w->strength * f;
        tmp.f[2] = D_0015EE6C * -0.5f;
        func_001F9C30_2D9270(&tmp, &tmp, 0.4f);
        func_001F9C30_2D9270(pv, pv, 0.6f);
        func_001F9BD8_2D9270(pv, &tmp, pv);
        func_001F9BD8_2D9270(&m->pos, &m->pos, pv);
        if (func_L00_001EFFF0_2D9270(&start, &m->pos, 6, 0, 0) != 0) {
            m->pos.quad = D_L04_00174060.quad;
            func_L00_001FF610_2D9270(pv, pv, (char *)&D_L04_00174060 + 0x20);
            func_001F9C30_2D9270(pv, pv, 0.25f);
            func_001F9BD8_2D9270(&m->pos, &m->pos, pv);
            pv->vel.f[2] = pv->vel.f[2] + w->strength / 5.0f;
        }
        f = w->strength;
        if (f < len) {
            func_L00_001FF4B0_2D9270(pv, pv, f);
        }
        lim = D_0015EE6C * -0.5f;
        z = pv->vel.f[2];
        if (lim < z) {
            pv->vel.f[2] = z + lim / 10.0f;
        } else {
            pv->vel.f[2] = lim;
        }
        m->rot[0] = func_001FA748_2D9270(m->rot[0], pv->rot_target[0]);
        m->rot[1] = func_001FA748_2D9270(m->rot[1], pv->rot_target[1]);
        m->rot[2] = func_001FA748_2D9270(m->rot[2], pv->rot_target[2]);
        r = func_001F9850_2D9270(10);
        a = pv->timer_start;
        b = pv->timer;
        if (a - r < b) {
            float t = func_001FA888_2D9270(b - a);
            float d = func_001F9878_2D9270(10.0f);
            m->speed = pv->speed * t / d;
        } else {
            r = func_001F9850_2D9270(100);
            if (pv->timer < r) {
                m->speed = m->speed * 0.99f;
            } else {
                m->speed = pv->speed;
            }
        }
        if (pv->timer > 0) return;
        m->pos.quad = pv->home.quad;
        r = func_L00_00258BC8_2D9270(0x78, 0x960);
        pv->timer_start = r;
        pv->timer = r;
        m->state = 1;
        m->speed = 0.0f;
        return;
    }
}
