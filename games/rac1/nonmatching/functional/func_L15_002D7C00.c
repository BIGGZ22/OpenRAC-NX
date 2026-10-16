/* func_L15_002D7C00 -- src/overlays/shared/vendor_002D7C00.c (functional C for the port, not a match)
 * Update of the electrified water (class 655, levels 15 and 17). Out of its area cuboid (any state
 * but 0) it releases its hum and its point light. State 0 resets its switches (floor switches 0x33E:
 * their done bytes and save bits cleared; others through func_L15_002DA990) and turns on. State 1
 * (on): the on timer running out turns it to 2; Ratchet in the water and in one of its 12 water
 * cuboids is shocked (hum moved onto him, fog saved and whitened, a bolt from his body, a hit);
 * else sparks jump between two points of the line nearest him (bolt, point light). States 1 and 2:
 * the lamps flicker (steady in 2), the hum follows the lamp nearest the camera, the off timer
 * running out in 2 turns it back on, a pressed switch turns it off (state 3) for pvars +0x18
 * seconds. State 3: lamps steady, light and hum off, countdown drawn; standing on a floor switch
 * when re-armed tops the time up; the time out or Ratchet in a reset cuboid turns it back on.
 * Every frame: Ratchet in a water-level cuboid gets its water level.
 * Adapted with ReRAC's water_shock.rs (ISC), which documents the US twin (level15 0x2d6810).
 * equiv: DIFFERENT, but by code shape only as far as read: the remaining items are values the
 * tool cannot place (a base address rebuilt after a label or in a delay slot in retail: D_L15_001BB334,
 * D_0013F4D0, the hero block; the 0xFF in m[0x30] set before a label), branch polarity and loop
 * induction (the line-length count, the line loop's counter), and one reload of pv->sw[k] that retail
 * repeats before func_L11_00310738 and this CSEs. Not yet proven by the run-time check. */
typedef unsigned int Q_2D7C00 __attribute__((mode(TI), aligned(16)));
typedef struct {
    int sw[4];
    short onT;
    short offT;
    int voice0;
    float offSecs;
    int shocks;
    int water[12];
    float fog[4];
    int levels[4];
    int resetA;
    int lamps[3];
    int voice1;
    unsigned char *owner;
    unsigned char fogRGB[4];
    int armed;
    int lines[3][8];
    Q_2D7C00 start;
    Q_2D7C00 end;
    Q_2D7C00 aim;
    int light;
    int area;
    int resetB;
} Pv_2D7C00;
typedef struct {
    float v[4];
    unsigned char *attacker;
    int flags;
    unsigned char b28;
    unsigned char b29;
    unsigned short oclass;
    float damage;
} Hit_2D7C00;
typedef struct {
    unsigned char pad0[0xD0];
    Q_2D7C00 body;
    unsigned char padE0[0x2F0 - 0xE0];
    float waterLevel;
    unsigned char pad2F4[0x2FC - 0x2F4];
    unsigned char *standing;
    unsigned char pad300[0x30E - 0x300];
    short airTicks;
    unsigned char pad310[0x2080 - 0x310];
    unsigned char *moby;
} Hero_2D7C00;
typedef struct {
    float f0, f4, f8;
    int iC;
    Q_2D7C00 pos;
    unsigned char pad20[0x1C - 0x20 + 0x20];
} Light_2D7C00;

extern Hero_2D7C00 G_2D7C00 __asm__("D_0013F450");
extern char D_0013F4D0_2D7C00[] __asm__("D_0013F4D0");
extern unsigned char D_0013E650_2D7C00[] __asm__("D_0013E650");
extern unsigned char *D_L15_00160058_2D7C00 SDATA(D_L15_00160058);
extern char *D_L15_0016016C_2D7C00 SDATA(D_L15_0016016C);
extern int D_0015EE84_2D7C00 SDATA(D_0015EE84);
extern float D_L15_00161C48_2D7C00 SDATA(D_L15_00161C48);
extern float D_L15_00161C4C_2D7C00 SDATA(D_L15_00161C4C);
extern float D_L15_00161C50_2D7C00 SDATA(D_L15_00161C50);
extern unsigned char D_L15_001BB334_2D7C00[] __asm__("D_L15_001BB334");
extern unsigned char D_L15_001BBF94_2D7C00[] __asm__("D_L15_001BBF94");
extern int D_0014C290_2D7C00[] __asm__("D_0014C290");
extern int D_L15_001BADE0_2D7C00[] __asm__("D_L15_001BADE0");
extern float D_L15_00167440_2D7C00[] __asm__("D_L15_00167440");
extern char D_L15_00180940_2D7C00[] __asm__("D_L15_00180940");
extern unsigned char D_L15_0015F544_2D7C00 __asm__("D_L15_0015F544") MACRO_ADDR;
extern unsigned char D_L15_0015F545_2D7C00 __asm__("D_L15_0015F545") MACRO_ADDR;
extern unsigned char D_L15_0015F546_2D7C00 __asm__("D_L15_0015F546") MACRO_ADDR;
extern float D_L15_0015F548_2D7C00 __asm__("D_L15_0015F548") MACRO_ADDR;
extern float D_L15_0015F54C_2D7C00 __asm__("D_L15_0015F54C") MACRO_ADDR;
extern float D_L15_0015F550_2D7C00 __asm__("D_L15_0015F550") MACRO_ADDR;
extern float D_L15_0015F554_2D7C00 __asm__("D_L15_0015F554") MACRO_ADDR;
extern int D_L15_0015F548_i_2D7C00 SDATA(D_L15_0015F548);
extern int D_L15_0015F554_i_2D7C00 SDATA(D_L15_0015F554);

extern int func_00215570_2D7C00(void *, int) __asm__("func_00215570");
extern void func_L00_0028EBF0_2D7C00(int) __asm__("func_L00_0028EBF0");
extern void func_L00_0023F1D0_2D7C00(int) __asm__("func_L00_0023F1D0");
extern void func_L15_002DA990_2D7C00(void *) __asm__("func_L15_002DA990");
extern float func_001F9878_2D7C00(float) __asm__("func_001F9878");
extern int func_001FA898_2D7C00(float) __asm__("func_001FA898");
extern int func_001F9850_2D7C00(int) __asm__("func_001F9850");
extern int func_L15_002D7B60_2D7C00(void) __asm__("func_L15_002D7B60");
extern int func_001F9938_2D7C00(void *) __asm__("func_001F9938");
extern void func_L00_00251328_2D7C00(void *, int, int, int) __asm__("func_L00_00251328");
extern int func_L00_00258BC8_2D7C00(int, int) __asm__("func_L00_00258BC8");
extern int func_002140B0_2D7C00(int) __asm__("func_002140B0");
extern float func_001F9D10_2D7C00(void *, void *) __asm__("func_001F9D10");
extern int func_L00_0023F0D0_2D7C00(void *, float, float, float, float, float) __asm__("func_L00_0023F0D0");
extern void func_L15_002D8DD8_2D7C00(void *) __asm__("func_L15_002D8DD8");
extern void func_L15_002D8FA0_2D7C00(void *) __asm__("func_L15_002D8FA0");
extern void func_L15_002D9B00_2D7C00(void) __asm__("func_L15_002D9B00");
extern void func_L15_002D8BB0_2D7C00(void) __asm__("func_L15_002D8BB0");
extern void func_001F49B0_2D7C00(void (*)(void), void *) __asm__("func_001F49B0");
extern int func_L00_0028EF68_2D7C00(int, int, void *, int) __asm__("func_L00_0028EF68");
extern float func_002140F8_2D7C00(float, float) __asm__("func_002140F8");
extern float func_00214158_2D7C00(void) __asm__("func_00214158");
extern void func_00215C00_2D7C00(void *, float, float, float) __asm__("func_00215C00");
extern void func_L00_001FF240_2D7C00(void *, void *, void *) __asm__("func_L00_001FF240");
extern float func_001F9F90_2D7C00(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2D7C00(float) __asm__("func_001F9FA8");
extern void func_L00_001FF500_2D7C00(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_L00_0025AAC0_2D7C00(void *, void *) __asm__("func_L00_0025AAC0");
extern float func_001F9D48_2D7C00(void *, void *) __asm__("func_001F9D48");
extern int func_L00_0028EB98_2D7C00(void *, int) __asm__("func_L00_0028EB98");
extern int func_L11_00310738_2D7C00(void *) __asm__("func_L11_00310738");
extern int func_L00_0020DC00_2D7C00(void) __asm__("func_L00_0020DC00");

#define MOBY_2D7C00(i) (D_L15_00160058_2D7C00 + ((i) << 8))
#define CUB_2D7C00(i) (D_L15_0016016C_2D7C00 + ((i) << 7))
/* A sound voice stopped if it still plays for this owner. */
#define STOP_2D7C00(i, own)                                                                        \
    if ((i) != -1) {                                                                               \
        unsigned char *e_ = D_0013E650_2D7C00 + (i) * 0x70;                                                 \
        if (*(unsigned char **)(e_ + 0x88) == (own) && e_[0x74] != 0) {                                     \
            func_L00_0028EBF0_2D7C00(i);                                                           \
        }                                                                                          \
    }
#define RELEASE_2D7C00(pv)                                                                         \
    STOP_2D7C00((pv)->voice0, (pv)->owner);                                                        \
    (pv)->voice0 = -1;                                                                             \
    STOP_2D7C00((pv)->voice1, (pv)->owner);                                                        \
    (pv)->voice1 = -1;
#define SECS_2D7C00(x) func_001FA898_2D7C00(func_001F9878_2D7C00((x) * 60.0f))

void func_L15_002D7C00(unsigned char *m) {
    float tmp[4];
    Hit_2D7C00 hit;
    Pv_2D7C00 *pv = *(Pv_2D7C00 **)(m + 0x78);
    int armed = 1;
    int wet, inside, k, n, r;
    int *best, *line;
    unsigned char *near, *o;
    unsigned short u;
    float d1, a, b;

    if (m[0x20] != 0 && func_00215570_2D7C00(D_0013F4D0_2D7C00, pv->area) == 0) {
        if (pv->owner != 0) {
            RELEASE_2D7C00(pv);
        }
        if (pv->light != -1) {
            func_L00_0023F1D0_2D7C00(pv->light);
            pv->light = -1;
        }
        return;
    }

    switch (m[0x20]) {
    case 0:
        m[0x30] = 0xFF;
        for (k = 0; k < 4; k++) {
            if (pv->sw[k] == -1) {
                continue;
            }
            if (*(short *)(MOBY_2D7C00(pv->sw[k]) + 0xA6) == 0x33E) {
                D_L15_001BB334_2D7C00[*(short *)(MOBY_2D7C00(pv->sw[k]) + 0xB2)] = 0;
                D_L15_001BBF94_2D7C00[*(short *)(MOBY_2D7C00(pv->sw[k]) + 0xB2)] = 0;
                u = *(unsigned short *)(MOBY_2D7C00(pv->sw[k]) + 0xB2);
                *(int *)((char *)D_0014C290_2D7C00 + (((short)u >> 5) << 2) + (D_0015EE84_2D7C00 << 8)) &=
                    ~(1 << (u & 0x1F));
                u = *(unsigned short *)(MOBY_2D7C00(pv->sw[k]) + 0xB2);
                *(int *)((char *)D_L15_001BADE0_2D7C00 + (((short)u >> 5) << 2)) &= ~(1 << (u & 0x1F));
            } else {
                func_L15_002DA990_2D7C00(MOBY_2D7C00(pv->sw[k]));
            }
        }
        pv->onT = SECS_2D7C00(D_L15_00161C48_2D7C00);
        pv->offT = func_001F9850_2D7C00(200);
        pv->shocks = 0;
        pv->light = -1;
        m[0x20] = 1;
        pv->voice1 = -1;
        pv->voice0 = -1;
        break;

    case 1:
        inside = 0;
        wet = func_L15_002D7B60_2D7C00();
        if (func_001F9938_2D7C00(&pv->onT) != 0) {
            pv->offT = SECS_2D7C00(D_L15_00161C4C_2D7C00);
            m[0x20] = 2;
            if (pv->owner != 0) {
                STOP_2D7C00(pv->voice1, pv->owner);
                pv->voice1 = -1;
            }
        }
        for (k = 0; k < 12; k++) {
            if (func_00215570_2D7C00(D_0013F4D0_2D7C00, pv->water[k]) != 0) {
                inside = 1;
                break;
            }
        }
        if (m[0x20] == 2) {
            for (k = 0; k < 3; k++) {
                if (pv->lamps[k] != -1) {
                    func_L00_00251328_2D7C00(MOBY_2D7C00(pv->lamps[k]), 0x40, 0x40, 0x40);
                }
            }
        } else {
            for (k = 0; k < 3; k++) {
                r = func_L00_00258BC8_2D7C00(0x40, 0x7F);
                if (pv->lamps[k] != -1) {
                    func_L00_00251328_2D7C00(MOBY_2D7C00(pv->lamps[k]), r, r, r);
                }
            }
        }

        if ((!inside || !wet) && func_002140B0_2D7C00(1) == 0 && D_L15_00161C50_2D7C00 == 0.0f) {
            best = 0;
            for (k = 0; k < 3; k++) {
                line = pv->lines[k];
                if (line[0] == -1) {
                    continue;
                }
                if (best != 0) {
                    d1 = func_001F9D10_2D7C00(CUB_2D7C00(best[0]) + 0x30, D_0013F4D0_2D7C00);
                    if (!(func_001F9D10_2D7C00(CUB_2D7C00(line[0]) + 0x30, D_0013F4D0_2D7C00) < d1)) {
                        continue;
                    }
                }
                best = line;
            }
            if (best != 0) {
                n = 8;
                if (best[0] == -1) {
                    n = 0;
                } else {
                    for (r = 1; r < 8; r++) {
                        if (best[r] == -1) {
                            n = r;
                            break;
                        }
                    }
                }
                if (n >= 2) {
                    best += func_L00_00258BC8_2D7C00(0, n - 2);
                    pv->start = *(Q_2D7C00 *)(CUB_2D7C00(best[0]) + 0x30);
                    pv->end = pv->aim = *(Q_2D7C00 *)(CUB_2D7C00(best[1]) + 0x30);
                    if (pv->light == -1) {
                        pv->light = func_L00_0023F0D0_2D7C00(&pv->start, 8.0f, 0.0f, 1.0f, 1.0f, 0.5f);
                    } else {
                        Light_2D7C00 *l = (Light_2D7C00 *)(D_L15_00180940_2D7C00 + (pv->light << 5));
                        qcopy(&l->pos, &pv->start);
                        *(float *)((char *)l + 0x1C) = 8.0f;
                        l->f4 = 1.0f;
                        l->f8 = 0.5f;
                        l->iC = 0;
                        l->f0 = 1.0f;
                    }
                    func_L15_002D8DD8_2D7C00(m);
                    func_L15_002D8FA0_2D7C00(m);
                    func_001F49B0_2D7C00(func_L15_002D9B00_2D7C00, m);
                }
            }
        }

        if (inside && wet) {
            if (pv->owner != 0 && pv->owner != G_2D7C00.moby) {
                RELEASE_2D7C00(pv);
            }
            pv->owner = G_2D7C00.moby;
            pv->voice0 = func_L00_0028EF68_2D7C00(0, 4, G_2D7C00.moby, 0x28F);
            pv->voice1 = func_L00_0028EF68_2D7C00(1, 4, G_2D7C00.moby, 0x28F);
            if (pv->shocks == 0) {
                pv->fog[0] = D_L15_0015F548_2D7C00;
                pv->fog[1] = D_L15_0015F54C_2D7C00;
                pv->fog[2] = D_L15_0015F550_2D7C00;
                pv->fog[3] = D_L15_0015F554_2D7C00;
                pv->fogRGB[0] = D_L15_0015F544_2D7C00;
                pv->fogRGB[1] = D_L15_0015F545_2D7C00;
                pv->fogRGB[2] = D_L15_0015F546_2D7C00;
            }
            pv->shocks = pv->shocks + 1;
            D_L15_0015F546_2D7C00 = 0x7F;
            D_L15_0015F544_2D7C00 = 0x7F;
            D_L15_0015F545_2D7C00 = 0x7F;
            D_L15_0015F54C_2D7C00 = 131072.0f;
            D_L15_0015F548_i_2D7C00 = 0;
            D_L15_0015F550_2D7C00 = func_002140F8_2D7C00(128.0f, 255.0f);
            D_L15_0015F554_i_2D7C00 = 0;
            d1 = func_002140F8_2D7C00(5.0f, 8.0f);
            a = func_00214158_2D7C00();
            b = func_00214158_2D7C00();
            func_00215C00_2D7C00(&pv->aim, d1, a, b);
            func_L00_001FF240_2D7C00(tmp, &pv->aim, &G_2D7C00.body);
            pv->end = pv->aim;
            pv->start = G_2D7C00.body;
            func_L15_002D8DD8_2D7C00(m);
            func_L15_002D8FA0_2D7C00(m);
            func_001F49B0_2D7C00(func_L15_002D9B00_2D7C00, m);
            hit.attacker = m;
            hit.flags = 0x10001;
            hit.damage = 1.0f;
            hit.v[0] = func_001F9F90_2D7C00(*(float *)(m + 0x48)) * 0.2f;
            hit.v[1] = func_001F9FA8_2D7C00(*(float *)(m + 0x48)) * 0.2f;
            *(int *)&hit.v[2] = 0;
            func_L00_001FF500_2D7C00(hit.v, hit.v, 1.0f);
            hit.b29 = 1;
            hit.oclass = *(unsigned short *)(m + 0xA6);
            hit.v[2] = 1.0f;
            hit.v[3] = 5627.9248f;
            hit.b28 = 0;
            func_L00_0025AAC0_2D7C00(G_2D7C00.moby, &hit);
            if (m[0x20] == 2) {
                D_L15_0015F548_2D7C00 = pv->fog[0];
                D_L15_0015F54C_2D7C00 = pv->fog[1];
                D_L15_0015F550_2D7C00 = pv->fog[2];
                D_L15_0015F554_2D7C00 = pv->fog[3];
                D_L15_0015F544_2D7C00 = pv->fogRGB[0];
                D_L15_0015F545_2D7C00 = pv->fogRGB[1];
                D_L15_0015F546_2D7C00 = pv->fogRGB[2];
            }
        }
        /* fall through: the hum and the switches, as in state 2 */
    case 2:
        near = 0;
        for (k = 0; k < 3; k++) {
            if (pv->lamps[k] == -1) {
                continue;
            }
            if (near != 0) {
                d1 = func_001F9D48_2D7C00(near + 0x10, D_L15_00167440_2D7C00);
                if (!(func_001F9D48_2D7C00(MOBY_2D7C00(pv->lamps[k]) + 0x10, D_L15_00167440_2D7C00) < d1)) {
                    continue;
                }
            }
            near = MOBY_2D7C00(pv->lamps[k]);
        }
        if (pv->owner != 0) {
            d1 = func_001F9D48_2D7C00(pv->owner + 0x10, D_L15_00167440_2D7C00);
            if (func_001F9D48_2D7C00(near + 0x10, D_L15_00167440_2D7C00) < d1) {
                if (pv->owner != 0) {
                    RELEASE_2D7C00(pv);
                }
                pv->owner = near;
            }
        } else {
            pv->owner = near;
        }
        if (func_L00_0028EB98_2D7C00(pv->owner, pv->voice0) == 0) {
            pv->voice0 = func_L00_0028EF68_2D7C00(0, 5, pv->owner, 0x28F);
        }
        if (m[0x20] == 1 && func_L00_0028EB98_2D7C00(pv->owner, pv->voice1) == 0) {
            pv->voice1 = func_L00_0028EF68_2D7C00(1, 5, pv->owner, 0x28F);
        }
        if (func_001F9938_2D7C00(&pv->offT) != 0 && m[0x20] == 2) {
            pv->onT = SECS_2D7C00(D_L15_00161C48_2D7C00);
            pv->offT = func_001F9850_2D7C00(200);
            pv->shocks = 0;
            m[0x20] = 1;
        }
        for (k = 0; k < 4; k++) {
            if (pv->sw[k] == -1) {
                continue;
            }
            if (!(*(short *)(MOBY_2D7C00(pv->sw[k]) + 0xA6) == 0x33E && MOBY_2D7C00(pv->sw[k])[0x20] == 2)) {
                if (pv->sw[k] == -1) {
                    continue;
                }
                if (func_L11_00310738_2D7C00(MOBY_2D7C00(pv->sw[k])) == 0) {
                    continue;
                }
            }
            pv->offT = SECS_2D7C00(pv->offSecs);
            m[0x20] = 3;
            func_L00_0028EF68_2D7C00(2, 0, MOBY_2D7C00(pv->sw[k]), 0x28F);
            pv->armed = 0;
        }
        break;

    case 3:
        for (k = 0; k < 3; k++) {
            if (pv->lamps[k] != -1) {
                func_L00_00251328_2D7C00(MOBY_2D7C00(pv->lamps[k]), 0x40, 0x40, 0x40);
            }
        }
        if (pv->light != -1) {
            func_L00_0023F1D0_2D7C00(pv->light);
            pv->light = -1;
        }
        if (pv->offT != 0) {
            func_001F49B0_2D7C00(func_L15_002D8BB0_2D7C00, m);
        }
        if (pv->owner != 0) {
            RELEASE_2D7C00(pv);
            pv->owner = 0;
        }
        for (k = 0; k < 4; k++) {
            if (pv->sw[k] == -1) {
                continue;
            }
            o = MOBY_2D7C00(pv->sw[k]);
            if (G_2D7C00.standing != o || *(short *)(G_2D7C00.standing + 0xA6) != 0x33E || G_2D7C00.airTicks != 0) {
                continue;
            }
            armed = 0;
            if (pv->armed == 0) {
                continue;
            }
            n = pv->offT;
            if (n < SECS_2D7C00(pv->offSecs) - func_001F9850_2D7C00(0x3C)) {
                pv->offT = SECS_2D7C00(pv->offSecs);
                func_L00_0028EF68_2D7C00(2, 0, MOBY_2D7C00(pv->sw[k]), 0x28F);
            }
            armed = 0;
        }
        pv->armed = armed;
        if (func_001F9938_2D7C00(&pv->offT) == 0) {
            if (func_00215570_2D7C00(D_0013F4D0_2D7C00, pv->resetA) == 0 || *(short *)(D_0013F4D0_2D7C00 + 0x28E) != 0) {
                if (func_00215570_2D7C00(D_0013F4D0_2D7C00, pv->resetB) == 0) {
                    break;
                }
            }
        }
        for (k = 0; k < 4; k++) {
            if (pv->sw[k] == -1) {
                continue;
            }
            if (*(short *)(MOBY_2D7C00(pv->sw[k]) + 0xA6) == 0x33E) {
                MOBY_2D7C00(pv->sw[k])[0x20] = 1;
            } else {
                func_L15_002DA990_2D7C00(MOBY_2D7C00(pv->sw[k]));
            }
        }
        pv->onT = SECS_2D7C00(D_L15_00161C48_2D7C00);
        pv->offT = func_001F9850_2D7C00(200);
        pv->shocks = 0;
        m[0x20] = 1;
        break;
    }

    {
        Hero_2D7C00 *g = (Hero_2D7C00 *)(D_0013F4D0_2D7C00 - 0x80);
        for (k = 0; k < 4; k++) {
            if (func_00215570_2D7C00(D_0013F4D0_2D7C00, pv->levels[k]) != 0 && func_L00_0020DC00_2D7C00() != 0) {
                g->waterLevel = *(float *)(CUB_2D7C00(pv->levels[k]) + 0x38);
            }
        }
    }
}
