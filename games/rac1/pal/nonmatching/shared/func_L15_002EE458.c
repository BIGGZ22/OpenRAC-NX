/* NON_MATCHING func_L15_002EE458 -- src/overlays/shared/vendor_002D7C00.c
 * Best so far: SIZE ours 908 / retail 912, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: the hero through Hero (ground_moby, air_frames, state.current, unk20A4, motion.pos and rot),
 *   the pad through PadState.pressed, and the copy at the end goes into the hero's own unk1D00 (0x141150 is
 *   hero + 0x1D00, retail addresses both from one base). SIZE 908/912, was 904. Left: retail dispatches the
 *   state with a switch (slti 2 / beq, the 3-or-1 choice as branches, not movn), where the candidate has an if
 *   chain. Plain VecQuad assignments for the copy give 892 (the ends merge).
 */
#include "hero.h"
#include "pad.h"
extern Hero D_0013F450_2EE458 __asm__("D_0013F450");
extern PadState D_0013CA40_2EE458 __asm__("D_0013CA40");
extern void func_0020D678(void *);
extern void func_00215F80(int, int);
extern void func_L15_002092E0(void);
extern void func_00213D28(void *, int, int);
extern float func_00214358(void *, int, float);
extern void func_L00_00286128(void *, void *);
extern void func_L00_00217718(void *, void *, int, int);
extern void func_L00_002110C0(int, int, char *);
extern void func_L00_00299B68(int);
extern void func_L00_002664B0(int, int);
extern int D_0015EE84 MACRO_ADDR;
extern int D_L15_0015F6A8 MACRO_ADDR;
extern int D_L15_00160058_m __asm__("D_L15_00160058") MACRO_ADDR;
extern int D_L15_0015F674 MACRO_ADDR;
extern char D_0014171B[] NOT_SDA;

// Update for the giant Clank pad moby: a small state machine that sets up its effect block and its timers.
void func_L15_002EE458(char *m) {
    char *data = *(char **)(m + 0x78);
    char *other;
    char *o3;
    char *o2;
    char *x;
    float V[4];
    float f;
    int st;
    int idx;
    int v;

    st = ((unsigned char *)m)[0x20];
    if (st == 1) {
        m[0x20] = 2;
        return;
    }
    if (st == 0) {
        idx = *(int *)(data + 0x60);
        if (idx == -1) {
            func_0020D678(m);
            return;
        }
        other = (char *)D_L15_00160058_m + (idx << 8);
        *(unsigned short *)(other + 0x34) = *(unsigned short *)(other + 0x34) | 1;
        other[0x31] = 0;
        *(int *)(other + 0x94) = 0;
        if (D_0015EE84 == 0x12) {
            x = (char *)D_0014171B + 0xAA35 + ((unsigned char *)m)[0xB0];
            st = 3;
            if (((unsigned char *)x)[0x120] != 0xFF) st = 1;
        } else {
            st = 1;
        }
        m[0x20] = st;
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (st != 2) return;

    if ((char *)D_0013F450_2EE458.ground_moby != m) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (D_0013F450_2EE458.air_frames.s != 0) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (D_L15_0015F6A8 == st) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (D_0013F450_2EE458.state.current == 0x1D) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    v = D_0013F450_2EE458.unk20A4;
    if (v == 0) {
        func_00215F80(4, 0x3AA4);
    } else if (v == 2) {
        if (D_0015EE84 != 0x12) func_00215F80(4, 0x3AA5);
    }
    if ((D_0013CA40_2EE458.pressed & 0x10) == 0) return;
    if (D_L15_0015F674 != 4) return;
    if (D_0013F450_2EE458.unk20A4 == 2) {
        if (D_0015EE84 == 0xF) {
            other = (char *)D_L15_00160058_m + (*(int *)(data + 0x60) << 8);
            *(unsigned short *)(other + 0x34) = *(unsigned short *)(other + 0x34) | 1;
            other[0x31] = 0;
            *(int *)(other + 0x94) = 0;
            func_L15_002092E0();
            func_00213D28(other, 0, 0);
            if (D_0015EE84 == 0xF) {
                func_L00_00299B68(9);
                func_L00_002664B0(0, 5);
            }
        }
        return;
    }
    if (D_0013F450_2EE458.unk20A4 != 0) return;

    idx = *(int *)(data + 0x60);
    other = (char *)D_L15_00160058_m + (idx << 8);
    if (*(int *)(data + 0x70) != -1) {
        o2 = (char *)D_L15_00160058_m + (*(int *)(data + 0x70) << 8);
        if (o2 != 0 && ((unsigned char *)o2)[0x20] != 0xFE) {
            if (((unsigned char *)o2)[0x20] != 0xFD) {
                func_0020D678(o2);
                *(int *)(data + 0x70) = -1;
            }
        }
    }
    qcopy(V, m + 0x10);
    f = func_00214358(V, 0, 0.5f);
    f = f + 0.1f;
    *(float *)((char *)V + 8) = f;
    if (D_0015EE84 == 0xF) func_L00_00286128(V, m + 0x40);
    func_L00_00217718(V, m + 0x40, 0, 1);
    o3 = *(char **)(other + 0x24);
    v = *(unsigned short *)(other + 0x34) & 0xFFFE;
    other[0x31] = 1;
    *(int *)(other + 0x94) = *(int *)(o3 + 0x10);
    *(unsigned short *)(other + 0x34) = v;
    func_L00_002110C0(2, 0x5A, other);
    if (D_0015EE84 == 0xF) {
        func_L00_00299B68(8);
        qcopy(D_0013F450_2EE458.unk1D00, &D_0013F450_2EE458.motion.pos);
        qcopy(D_0013F450_2EE458.unk1D00 + 0x10, &D_0013F450_2EE458.motion.rot);
        func_L00_002664B0(2, 4);
        return;
    }
    if (D_0015EE84 != 0x12) return;
    func_L00_00299B68(6);
    qcopy(D_0013F450_2EE458.unk1D00, &D_0013F450_2EE458.motion.pos);
    qcopy(D_0013F450_2EE458.unk1D00 + 0x10, &D_0013F450_2EE458.motion.rot);
    m[0x20] = 3;
}
