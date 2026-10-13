/* Carried from the PAL decompilation's nonmatching/functional/func_L12_00307DB0.c (games/rac1/pal) through the version
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
/* functional: medium. Same reads, writes, calls and arguments as retail; the jump table is a switch, shared exits are gotos, quad copies use qcopy, repeated global and data loads are shared, and the integer remainder by the path point count is guarded against zero (retail traps with break 7). */
/* Level 12 (Hoven) moby update for a scripted camera/cutscene-style moby driven by a path (data = moby+0x78 is its
 * state block, 17 child slots of 0x30 bytes at data+0x20, each a moby index or -1). Before the state machine, a
 * nonzero state with D_L12_001FB6CC set restores the saved position (moby+0x10) and rotation (moby+0x40) quads.
 * 0 setup: when the save table byte for this moby is 0xFF it deletes its children and itself; otherwise it sets
 * up the children (func_L12_0027B268), copies the first path point (data+0x5C0 selects the point table in
 * D_L12_001B0930), faces the next point, resets D_L12_001FB688 and the per-child state, goes to 1.
 * 1 wait: raises or lowers the child flags 0x41 by the func_00214720 trigger, steers z with the water level
 * (D_0013E533+0xEA5), the springs for the pose and the child poses (func_L00_00262B00 / 00263BF8, func_L12_0027B370),
 * and when the second trigger fires (or D_L12_00161F00) marks the children 0xFF at +0xA4 and goes to 2.
 * 2 / 3 follow the path points: advance the progress at data+0x5F0, move along the segment, spring the heading and
 * switch to the next point when closer than 5 units or the heading is off by more than pi/2 (state 3 also bumps
 * D_L12_00161F34); state 2 goes to 3 when the progress reaches 0.
 * 4 the end sequence driven by the camera cut records (moby+0xBC 0, 1, 2): set up the cut, interpolate the two
 * camera records at D_L12_001600EC (position and rotation, func_001F9A40 / func_001FA5C8 / func_001FA580), finish it
 * and delete children and itself.
 * Every state then runs the tail: a per-frame child pass (func_L12_00306350 for 18 slots, func_L12_003064E8 and the
 * counter at data+0x5FA), func_L12_00306828, and the slot (data+0x604) from func_0022DA68 is claimed in states 1..3
 * and freed (func_L00_0028D918) otherwise.
 * No VU0 macro code here, only float arithmetic in the order of the asm. */
extern int D_L12_001FB98C_307DB0 __asm__("D_L12_001FB6CC");
extern char D_L12_00161FE0_307DB0[] __asm__("D_L12_00161F60");
extern char D_L12_00161FF0_307DB0[] __asm__("D_L12_00161F70");
extern int D_L12_00161A34_307DB0 __asm__("D_L12_001619B4") MACRO_ADDR;
extern int D_L12_00161A20_307DB0 __asm__("D_L12_001619A0") MACRO_ADDR;
extern int D_L12_0015F504_307DB0 __asm__("D_L12_0015F404") MACRO_ADDR;
extern int D_L12_00161FA4_307DB0 __asm__("D_L12_00161F24") MACRO_ADDR;
extern float D_L12_00161FA8_307DB0 __asm__("D_L12_00161F28") MACRO_ADDR;
extern float D_L12_00161FAC_307DB0 __asm__("D_L12_00161F2C") MACRO_ADDR;
extern float D_L12_00161FB0_307DB0 __asm__("D_L12_00161F30") MACRO_ADDR;
extern int D_L12_00161FB4_307DB0 __asm__("D_L12_00161F34") MACRO_ADDR;
extern float D_L12_00161FC0_307DB0 __asm__("D_L12_00161F40") MACRO_ADDR;
extern int D_L12_00161F80_307DB0 __asm__("D_L12_00161F00") MACRO_ADDR;
extern char *D_L12_00160058_307DB0 __asm__("D_L12_0015FFD8") MACRO_ADDR;
extern char *D_L12_0016016C_307DB0 __asm__("D_L12_001600EC") MACRO_ADDR;
extern int D_L12_001FB948_307DB0[] __asm__("D_L12_001FB688");
extern char D_L12_001672C0_307DB0[] __asm__("D_L12_00167240");
extern char *D_L12_001B0C30_307DB0[] __asm__("D_L12_001B0930");
extern unsigned char D_0013E633_307DB0[] __asm__("D_0013E533");
extern unsigned char D_0014171B_307DB0[] __asm__("D_0014161B");
extern int D_0015EE84_307DB0 __asm__("D_0015ED84") MACRO_ADDR;
extern float D_0015EE6C_307DB0 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE70_307DB0 __asm__("D_0015ED70") MACRO_ADDR;
extern float D_0015EE64_307DB0 __asm__("D_0015ED64") MACRO_ADDR;

extern void func_0020D678_307DB0(void *) __asm__("func_0020C828");
extern void func_L12_0027C260_307DB0(void *, void *, int) __asm__("func_L12_0027B268");
extern void func_L12_0027C368_307DB0(void *, void *, int) __asm__("func_L12_0027B370");
extern float func_L00_001FF860_307DB0(float, float) __asm__("func_L00_001FF8B0");
extern int func_00215570_307DB0(void *, int) __asm__("func_00214720");
extern float func_00214D28_307DB0(float *, float, float) __asm__("func_00213ED8");
extern int func_001F9850_307DB0(int) __asm__("func_001F9688");
extern int func_001F9908_307DB0(void *) __asm__("func_001F9740");
extern void func_L00_0025C918_307DB0(float *, float *, float, float, float, float) __asm__("func_L00_0025B8C0");
extern void func_L00_00263B78_307DB0(void *, void *, void *, float, float) __asm__("func_L00_00262B00");
extern void func_L00_00263BF8_307DB0(void *, void *, void *, float, float, float) __asm__("func_L00_00262B80");
extern void func_001F9BF0_307DB0(void *, void *, void *) __asm__("func_001F9A28");
extern void func_L00_001FF4B0_307DB0(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_001F9BD8_307DB0(void *, void *, void *) __asm__("func_001F9A10");
extern float func_001F9D48_307DB0(void *, void *) __asm__("func_001F9B80");
extern float func_001FA850_307DB0(float, float) __asm__("func_001FA688");
extern void func_L00_002592B0_307DB0(void *, void *, float, float, float, float) __asm__("func_L00_00258278");
extern float func_L00_0025CCF0_307DB0(void *, float, void *, int, float, float, float) __asm__("func_L00_0025BC98");
extern void func_L12_00306890_307DB0(void *) __asm__("func_L12_003054B0");
extern float func_00214D88_307DB0(float *, float *, float, float, float, float) __asm__("func_00213F38");
extern void func_001F9C08_307DB0(void *, void *, void *, float) __asm__("func_001F9A40");
extern void func_001F9BC0_307DB0(void *) __asm__("func_001F99F8");
extern float func_001FA790_307DB0(float, float) __asm__("func_001FA5C8");
extern float func_001FA748_307DB0(float, float) __asm__("func_001FA580");
extern void func_L00_002EBE88_307DB0(void *) __asm__("func_L00_002EA9D8");
extern void func_L00_002EBEE0_307DB0(void *) __asm__("func_L00_002EAA30");
extern void func_L00_002EC0C8_307DB0(int) __asm__("func_L00_002EAC18");
extern void func_L00_00299B68_307DB0(int) __asm__("func_L00_00298840");
extern void func_L12_00307730_307DB0(void *, int) __asm__("func_L12_00306350");
extern void func_L12_003078C8_307DB0(void *) __asm__("func_L12_003064E8");
extern void func_L12_00307C08_307DB0(void *) __asm__("func_L12_00306828");
extern int func_0022ED80_307DB0(int, int, void *) __asm__("func_0022DA68");
extern void func_L00_0028EBF0_307DB0(int) __asm__("func_L00_0028D918");

#define U8_307DB0(p, o) (*(unsigned char *)((char *)(p) + (o)))
#define S8_307DB0(p, o) (*(signed char *)((char *)(p) + (o)))
#define U16_307DB0(p, o) (*(unsigned short *)((char *)(p) + (o)))
#define S16_307DB0(p, o) (*(short *)((char *)(p) + (o)))
#define I_307DB0(p, o) (*(int *)((char *)(p) + (o)))
#define F_307DB0(p, o) (*(float *)((char *)(p) + (o)))
#define P_307DB0(p, o) (*(char **)((char *)(p) + (o)))

void FUN_L12_003069d0(char *moby)
{
    char *data = P_307DB0(moby, 0x78);
    char *p;
    char *m;
    char *a;
    char *b;
    int i;
    int v;
    int idx;
    int nxt;
    int cnt;
    float vec[8];
    float f20;
    float f12;
    float t;

    if (U8_307DB0(moby, 0x20) != 0 && D_L12_001FB98C_307DB0 != 0) {
        qcopy(moby + 0x10, D_L12_00161FE0_307DB0);
        qcopy(moby + 0x40, D_L12_00161FF0_307DB0);
    }

    switch (U8_307DB0(moby, 0x20)) {
    case 0:
        D_L12_00161A34_307DB0 = 0;
        if (D_0014171B_307DB0[0xAA35 + U8_307DB0(moby, 0xB0) + D_0015EE84_307DB0 * 16] == 0xFF) {
            for (i = 0; i < 17; i++) {
                v = I_307DB0(data, 0x20 + i * 0x30);
                if (v != -1) {
                    func_0020D678_307DB0(D_L12_00160058_307DB0 + v * 256);
                }
            }
            func_0020D678_307DB0(moby);
            return;
        }
        D_L12_00161FA4_307DB0 = 0;
        func_L12_0027C260_307DB0(moby, data, 0x11);
        p = D_L12_001B0C30_307DB0[I_307DB0(data, 0x5C0)];
        qcopy(moby + 0x10, p + 0x10);
        F_307DB0(data, 0x618) = F_307DB0(moby, 0x18);
        F_307DB0(moby, 0x48) = func_L00_001FF860_307DB0(F_307DB0(p, 0x20) - F_307DB0(p, 0x10),
                                                         F_307DB0(p, 0x24) - F_307DB0(p, 0x14));
        if (*(float *)(D_0013E633_307DB0 + 0xEA5) < 52.0f) {
            F_307DB0(moby, 0x18) = F_307DB0(moby, 0x18) + 3.0f;
        }
        for (i = 0; i < 5; i++) {
            v = I_307DB0(data, 0x5D0 + i * 4);
            m = D_L12_00160058_307DB0 + I_307DB0(data, 0x20 + v * 0x30) * 256;
            F_307DB0(P_307DB0(m, 0x78), 0x80) = 10.0f;
        }
        D_L12_00161FC0_307DB0 = -1.0f;
        for (i = 17; i >= 0; i--) {
            D_L12_001FB948_307DB0[i] = 0;
        }
        I_307DB0(data, 0x604) = -1;
        S16_307DB0(data, 0x5FC) = 100;
        U8_307DB0(moby, 0x20) = 1;
        break;

    case 1:
        if (func_00215570_307DB0(D_L12_001672C0_307DB0, I_307DB0(data, 0x624)) != 0) {
            U16_307DB0(moby, 0x34) = U16_307DB0(moby, 0x34) | 0x41;
            for (i = 0; i < 17; i++) {
                v = I_307DB0(data, 0x20 + i * 0x30);
                if (v != -1) {
                    m = D_L12_00160058_307DB0 + v * 256;
                    U16_307DB0(m, 0x34) = U16_307DB0(m, 0x34) | 0x41;
                }
            }
        } else {
            U16_307DB0(moby, 0x34) = U16_307DB0(moby, 0x34) & 0xFFBE;
            for (i = 0; i < 17; i++) {
                v = I_307DB0(data, 0x20 + i * 0x30);
                if (v != -1) {
                    m = D_L12_00160058_307DB0 + v * 256;
                    U16_307DB0(m, 0x34) = U16_307DB0(m, 0x34) & 0xFFBE;
                }
            }
        }
        v = func_00215570_307DB0(D_0013E633_307DB0 + 0xE9D, I_307DB0(data, 0x628));
        if (52.0f < *(float *)(D_0013E633_307DB0 + 0xEA5) && v == 0) {
            func_00214D28_307DB0((float *)(moby + 0x18), F_307DB0(data, 0x618), D_0015EE6C_307DB0 + D_0015EE6C_307DB0);
        } else {
            f20 = 3.0f;
            if (v != 0) {
                f20 = 10.0f;
            }
            if (func_00215570_307DB0(D_0013E633_307DB0 + 0xE9D, I_307DB0(data, 0x61C)) != 0) {
                I_307DB0(data, 0x620) = func_001F9850_307DB0(0x5A);
            }
            if (func_001F9908_307DB0(data + 0x620) == 0) {
                f20 = 19.0f;
            }
            func_L00_0025C918_307DB0((float *)(moby + 0x18), (float *)(data + 0x62C), F_307DB0(data, 0x618) + f20,
                                     D_0015EE64_307DB0 * 0.0299999993f, D_0015EE64_307DB0 * 0.300000012f,
                                     D_0015EE6C_307DB0 * 2.70000005f);
        }
        func_L00_00263B78_307DB0(moby, data + 0x5CC, data + 0x5C8, 0.699999988f, D_0015EE6C_307DB0 * 1.22173047f);
        func_L00_00263BF8_307DB0(moby, data + 0x5E4, data + 0x5E8, D_L12_00161FB0_307DB0,
                                 D_L12_00161FA8_307DB0 * 0.0174532924f * D_0015EE6C_307DB0,
                                 D_L12_00161FAC_307DB0 * 0.0174532924f * D_0015EE6C_307DB0);
        func_L12_0027C368_307DB0(moby, data, 0x11);
        if (func_00215570_307DB0(D_0013E633_307DB0 + 0xE9D, I_307DB0(data, 0x5EC)) == 0 &&
            D_L12_00161F80_307DB0 == 0) {
            break;
        }
        D_L12_00161A34_307DB0 = 1;
        for (i = 0; i < 5; i++) {
            v = I_307DB0(data, 0x5D0 + i * 4);
            m = D_L12_00160058_307DB0 + I_307DB0(data, 0x20 + v * 0x30) * 256;
            U8_307DB0(m, 0xA4) = 0xFF;
        }
        U8_307DB0(moby, 0x20) = 2;
        break;

    case 2:
        func_L00_00263B78_307DB0(moby, data + 0x5CC, data + 0x5C8, 0.699999988f, D_0015EE6C_307DB0 * 1.22173047f);
        func_L00_00263BF8_307DB0(moby, data + 0x5E4, data + 0x5E8, D_L12_00161FB0_307DB0,
                                 D_L12_00161FA8_307DB0 * 0.0174532924f * D_0015EE6C_307DB0,
                                 D_L12_00161FAC_307DB0 * 0.0174532924f * D_0015EE6C_307DB0);
        p = D_L12_001B0C30_307DB0[I_307DB0(data, 0x5C0)];
        if (S16_307DB0(data, 0x5F8) < I_307DB0(p, 0) - 11) {
            func_00214D28_307DB0((float *)(data + 0x5F0), D_0015EE6C_307DB0 * 8.0f, D_0015EE70_307DB0 * 10.0f);
        } else {
            f20 = 0.0f;
            func_00214D28_307DB0((float *)(data + 0x5F0), f20, D_0015EE70_307DB0 * 8.0f);
            if (F_307DB0(data, 0x5F0) == f20) {
                U8_307DB0(moby, 0x20) = 3;
                break;
            }
        }
        idx = S16_307DB0(data, 0x5F8);
        cnt = I_307DB0(p, 0);
        nxt = cnt != 0 ? (idx + 1) % cnt : 0;
        func_001F9BF0_307DB0(vec, p + 0x10 + nxt * 16, p + idx * 16 + 0x10);
        func_L00_001FF4B0_307DB0(vec, vec, F_307DB0(data, 0x5F0));
        func_001F9BD8_307DB0(moby + 0x10, moby + 0x10, vec);
        a = p + S16_307DB0(data, 0x5F8) * 16;
        b = p + nxt * 16;
        f12 = func_L00_001FF860_307DB0(F_307DB0(b, 0x10) - F_307DB0(a, 0x10), F_307DB0(b, 0x14) - F_307DB0(a, 0x14));
        if (I_307DB0(p, 0) - 0x14 < S16_307DB0(data, 0x5F8)) {
            f12 = 3.05432606f;
        }
        func_L00_002592B0_307DB0(moby, data + 0x5F4, f12, D_0015EE64_307DB0 * 0.00150000001f,
                                 D_0015EE64_307DB0 * 0.300000012f, D_0015EE6C_307DB0 * 0.17453292f);
        if (!(func_001F9D48_307DB0(moby + 0x10, p + S16_307DB0(data, 0x5F8) * 16 + 0x10) < 5.0f)) {
            a = p + S16_307DB0(data, 0x5F8) * 16;
            t = func_L00_001FF860_307DB0(F_307DB0(a, 0x10) - F_307DB0(moby, 0x10),
                                         F_307DB0(a, 0x14) - F_307DB0(moby, 0x14));
            if (!(1.57079637f < func_001FA850_307DB0(F_307DB0(moby, 0x48), t))) {
                goto finish;
            }
        }
        idx = S16_307DB0(data, 0x5F8);
        cnt = I_307DB0(p, 0);
        S16_307DB0(data, 0x5F8) = cnt != 0 ? (idx + 1) % cnt : 0;
        goto finish;

    case 3:
        D_L12_00161FB4_307DB0 = D_L12_00161FB4_307DB0 + 1;
        func_L00_0025CCF0_307DB0(moby + 0x48, 3.05432606f, data + 0x5F4, -1, D_0015EE64_307DB0 * 0.00150000001f,
                                 D_0015EE64_307DB0 * 0.300000012f, D_0015EE6C_307DB0 * 0.17453292f);
        func_L00_00263B78_307DB0(moby, data + 0x5CC, data + 0x5C8, 0.699999988f, D_0015EE6C_307DB0 * 1.22173047f);
        func_L00_00263BF8_307DB0(moby, data + 0x5E4, data + 0x5E8, D_L12_00161FB0_307DB0,
                                 D_L12_00161FA8_307DB0 * 0.0174532924f * D_0015EE6C_307DB0,
                                 D_L12_00161FAC_307DB0 * 0.0174532924f * D_0015EE6C_307DB0);
    finish:
        func_L12_0027C368_307DB0(moby, data, 0x11);
        func_L12_00306890_307DB0(moby);
        break;

    case 4:
        if (I_307DB0(data, 0x608) != -1 && I_307DB0(data, 0x60C) != -1) {
            v = U8_307DB0(moby, 0xBC);
            if (v == 0) {
                U8_307DB0(moby, 0xBC) = 1;
                func_L00_002EBE88_307DB0(D_L12_0016016C_307DB0 + I_307DB0(data, 0x608) * 128 + 0x30);
                func_L00_002EBEE0_307DB0(D_L12_0016016C_307DB0 + I_307DB0(data, 0x608) * 128 + 0x70);
                D_L12_00161A20_307DB0 = 1;
                D_L12_0015F504_307DB0 = 1;
            } else if (v == 1) {
                if (F_307DB0(data, 0x610) < 1.0f) {
                    t = D_0015EE70_307DB0 * 0.666000009f;
                    func_00214D88_307DB0((float *)(data + 0x610), (float *)(data + 0x614), 1.0f, t, t,
                                         D_0015EE6C_307DB0 * 0.400000006f);
                    a = D_L12_0016016C_307DB0 + I_307DB0(data, 0x608) * 128;
                    b = D_L12_0016016C_307DB0 + I_307DB0(data, 0x60C) * 128;
                    func_001F9C08_307DB0(vec, a + 0x30, b + 0x30, F_307DB0(data, 0x610));
                    func_001F9BC0_307DB0(vec + 4);
                    /* vec[4] / vec[5] / vec[6]: start angle plus the eased difference to the end record */
                    t = func_001FA790_307DB0(F_307DB0(b, 0x70), F_307DB0(a, 0x70)) * F_307DB0(data, 0x610);
                    f12 = vec[5];
                    vec[4] = t;
                    vec[4] = func_001FA748_307DB0(F_307DB0(a, 0x70), f12);
                    t = func_001FA790_307DB0(F_307DB0(b, 0x74), F_307DB0(a, 0x74)) * F_307DB0(data, 0x610);
                    vec[5] = t;
                    vec[5] = func_001FA748_307DB0(F_307DB0(a, 0x74), t);
                    t = func_001FA790_307DB0(F_307DB0(b, 0x78), F_307DB0(a, 0x78)) * F_307DB0(data, 0x610);
                    vec[6] = t;
                    vec[6] = func_001FA748_307DB0(F_307DB0(a, 0x78), t);
                    func_L00_002EBE88_307DB0(vec);
                    func_L00_002EBEE0_307DB0(vec + 4);
                } else {
                    U8_307DB0(moby, 0xBC) = 2;
                    D_L12_0015F504_307DB0 = 0;
                    func_L00_002EC0C8_307DB0(3);
                }
            } else if (v == 2) {
                for (i = 0; i < 17; i++) {
                    v = I_307DB0(data, 0x20 + i * 0x30);
                    if (v != -1) {
                        m = D_L12_00160058_307DB0 + v * 256;
                        if (S8_307DB0(m, 0x20) >= 0) {
                            func_0020D678_307DB0(m);
                        }
                        I_307DB0(data, 0x20 + i * 0x30) = -1;
                    } else if (D_L12_001FB948_307DB0[i] > 0) {
                        m = D_L12_00160058_307DB0 + D_L12_001FB948_307DB0[i] * 256;
                        if (S8_307DB0(m, 0x20) >= 0) {
                            func_0020D678_307DB0(m);
                        }
                    }
                }
                D_L12_00161A34_307DB0 = 0;
                func_L00_00299B68_307DB0(7);
                func_0020D678_307DB0(moby);
                return;
            }
        }
        if (S16_307DB0(data, 0x5FA) == 0) {
            for (i = 0; i < 17; i++) {
                if (D_L12_001FB948_307DB0[i] != 0) {
                    I_307DB0(data, 0x20 + i * 0x30) = -1;
                } else {
                    D_L12_001FB948_307DB0[i] = I_307DB0(data, 0x20 + i * 0x30);
                    I_307DB0(data, 0x20 + i * 0x30) = -1;
                }
            }
            D_L12_001FB948_307DB0[17] = 1;
            for (i = 0; i < 18; i++) {
                func_L12_00307730_307DB0(moby, i);
            }
        }
        U16_307DB0(data, 0x5FA) = U16_307DB0(data, 0x5FA) + 1;
        func_L12_003078C8_307DB0(moby);
        break;

    default:
        break;
    }

    func_L12_00307C08_307DB0(moby);
    if ((unsigned)(U8_307DB0(moby, 0x20) - 1) < 3) {
        if (I_307DB0(data, 0x604) == -1) {
            I_307DB0(data, 0x604) = func_0022ED80_307DB0(0, 4, moby);
        }
    } else {
        v = I_307DB0(data, 0x604);
        if (v != -1) {
            m = (char *)D_0013E633_307DB0 + 0x1D + v * 0x70;
            if (P_307DB0(m, 0x88) == moby && U8_307DB0(m, 0x74) != 0) {
                func_L00_0028EBF0_307DB0(v);
            }
        }
        I_307DB0(data, 0x604) = -1;
    }
}
