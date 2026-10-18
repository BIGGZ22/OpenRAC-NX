/* functional: high. Corrected from the previous nonmatching/functional/func_L01_0030F318.c, which had only state 0 and the common tail: now all of states 0-7 (jump table jtbl_L01_0020BD40), the dispatch on the linked moby, the link-target base read from D_L01_00160058 in both branches (the old copy read hero+0x5893), and the missing level tunables (D_L01_00162128..00162150). Differences from retail: null guard on the linked pvars write at the top; divide-by-zero guard (retail traps) in state 6. */
/* MouseCritterUpdate (class 0x71A update): ambient critter state machine. */
#include "common.h"

typedef struct { float v[4]; } V4_30F318;
#define QCOPY_30F318(dst, src) (*(V4_30F318 *)(dst) = *(V4_30F318 *)(src))
#define QZERO_30F318(dst) do { V4_30F318 z_ = {{0, 0, 0, 0}}; *(V4_30F318 *)(dst) = z_; } while (0)

extern char *D_L01_00160058_30F318 __asm__("D_L01_00160058");
extern char D_0013E633_30F318[] __asm__("D_0013E633");
extern unsigned char D_0013D5CA_30F318[] __asm__("D_0013D5CA");
extern unsigned char D_0015EEB0_30F318[] __asm__("D_0015EEB0");
extern int D_0015EE84_30F318 __asm__("D_0015EE84");
extern float D_0015EE60_30F318 __asm__("D_0015EE60");
extern float D_0015EE6C_30F318 __asm__("D_0015EE6C");
extern float D_0015EE70_30F318 __asm__("D_0015EE70");
extern unsigned char D_0014171B_30F318[] __asm__("D_0014171B");
extern unsigned char D_L01_001BB9C0_30F318[] __asm__("D_L01_001BB9C0");
extern char D_L01_001DEE08_30F318[] __asm__("D_L01_001DEE08");
extern char D_L01_0020BD00_30F318[] __asm__("D_L01_0020BD00");
extern char D_L01_0020BD20_30F318[] __asm__("D_L01_0020BD20");
extern int D_L01_00174358_30F318 __asm__("D_L01_00174358");
extern char *D_L01_00167300_30F318 __asm__("D_L01_00167300");
/* per-level tunables in the lit block, read gp-relative by retail */
extern int D_L01_00162128_30F318 __asm__("D_L01_00162128");
extern int D_L01_0016212C_30F318 __asm__("D_L01_0016212C");
extern int D_L01_00162134_30F318 __asm__("D_L01_00162134");
extern int D_L01_00162138_30F318 __asm__("D_L01_00162138");
extern int D_L01_0016213C_30F318 __asm__("D_L01_0016213C");
extern float D_L01_00162140_30F318 __asm__("D_L01_00162140");
extern float D_L01_00162144_30F318 __asm__("D_L01_00162144");
extern float D_L01_00162148_30F318 __asm__("D_L01_00162148");
extern int D_L01_0016214C_30F318 __asm__("D_L01_0016214C");
extern float D_L01_00162150_30F318 __asm__("D_L01_00162150");

extern float func_L00_001FF860_30F318(float, float) __asm__("func_L00_001FF860");
extern void func_0020D960_30F318(void *, int, void *) __asm__("func_0020D960");
extern void func_0020D9D8_30F318(void *, void *) __asm__("func_0020D9D8");
extern void func_L01_0030F2A8_30F318(void *) __asm__("func_L01_0030F2A8");
extern int func_001F9850_30F318(int) __asm__("func_001F9850");
extern void func_00213DE0_30F318(void *, int, int, int) __asm__("func_00213DE0");
extern void func_L00_0025B040_30F318(void *, float) __asm__("func_L00_0025B040");
extern void func_L01_0030F178_30F318(void *, void *) __asm__("func_L01_0030F178");
extern void func_0020D678_30F318(void *) __asm__("func_0020D678");
extern void func_001E9730_30F318(void *) __asm__("func_001E9730");
extern void func_L01_0026E8E0_30F318(void *) __asm__("func_L01_0026E8E0");
extern void func_L00_002592B0_30F318(void *, void *, float, float, float, float) __asm__("func_L00_002592B0");
extern int func_00215570_30F318(void *, int) __asm__("func_00215570");
extern int func_L00_001EFFF0_30F318(void *, void *, int, void *, int) __asm__("func_L00_001EFFF0");
extern void func_L01_002E4430_30F318(void *, void *) __asm__("func_L01_002E4430");
extern void func_L01_0023D688_30F318(int, int) __asm__("func_L01_0023D688");
extern void func_L00_00232C10_30F318(int, int, float) __asm__("func_L00_00232C10");
extern int func_0022ED80_30F318(int, int, void *) __asm__("func_0022ED80");
extern float func_001F9D48_30F318(void *, void *) __asm__("func_001F9D48");
extern float func_001F9D10_30F318(void *, void *) __asm__("func_001F9D10");
extern void func_L01_0030E638_30F318(void *, void *) __asm__("func_L01_0030E638");
extern void func_L01_0030EC58_30F318(void *, void *) __asm__("func_L01_0030EC58");
extern void func_L01_0030E9C8_30F318(void *, void *) __asm__("func_L01_0030E9C8");
extern int func_L00_0025A060_30F318(void *, void *, void *, void *) __asm__("func_L00_0025A060");
extern int func_L00_00258BC8_30F318(int, int) __asm__("func_L00_00258BC8");
extern float func_002140F8_30F318(float, float) __asm__("func_002140F8");
extern int func_001F9908_30F318(void *) __asm__("func_001F9908");
extern int func_001F9938_30F318(void *) __asm__("func_001F9938");
extern void func_L00_0028EBF0_30F318(int) __asm__("func_L00_0028EBF0");
extern void func_L00_00260958_30F318(void *, float) __asm__("func_L00_00260958");
extern int func_001160D8_30F318(void) __asm__("func_001160D8");
extern void *func_L00_00272F00_30F318(void *, int, float, float, int, int, int, void *, float) __asm__("func_L00_00272F00");
extern float func_001FA748_30F318(float, float) __asm__("func_001FA748");

#define HERO_30F318 (D_0013E633_30F318 + 0xE1D)
#define SLOTS_30F318 (D_0013E633_30F318 + 0x1D)
#define MOBY_AT_30F318(i) (D_L01_00160058_30F318 + ((i) << 8))
#define PI_30F318 3.1415927f

static float bits_30F318(unsigned u) { union { unsigned u; float f; } x; x.u = u; return x.f; }

/* the slot in the table at D_0013E633+0x1D (0x70 bytes each) is still ours and live */
static int slot_ok_30F318(char *moby, int k) {
    char *e = SLOTS_30F318 + k * 0x70;
    return *(char **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0;
}

/* turn toward (x, y) */
static void face_30F318(char *moby, float x, float y) {
    float a = func_L00_001FF860_30F318(x - *(float *)(moby + 0x10), y - *(float *)(moby + 0x14));
    func_L00_002592B0_30F318(moby, D_L01_001DEE08_30F318 + 0x14, a,
                             D_0015EE70_30F318 * bits_30F318(0x40C90FDB),
                             D_0015EE70_30F318 * PI_30F318,
                             D_0015EE6C_30F318 * bits_30F318(0x41384E88));
}

/* release our slot (if live) and clear it */
static void drop_slot_30F318(char *moby, char *pv) {
    int k = *(int *)(pv + 0x64);
    if (k != -1 && slot_ok_30F318(moby, k)) {
        func_L00_0028EBF0_30F318(k);
    }
    *(int *)(pv + 0x64) = -1;
}

static void ensure_slot_30F318(char *moby, char *pv) {
    int k = *(int *)(pv + 0x64);
    if (!slot_ok_30F318(moby, k)) {
        *(int *)(pv + 0x64) = -1;
        *(int *)(pv + 0x64) = func_0022ED80_30F318(1, 4, moby);
    }
}

static void talk_anim_30F318(char *moby) {
    if (moby[0x53] != 1) {
        func_00213DE0_30F318(moby, 1, 0, func_001F9850_30F318(func_L00_00258BC8_30F318(7, 12)));
    }
    *(float *)(moby + 0x58) = func_002140F8_30F318(0.95f, 1.05f);
}

void func_L01_0030F318(char *moby) {
    char *pv;
    char *hero = HERO_30F318;
    char *S = D_L01_001DEE08_30F318;
    int idx;
    int st;

    if (moby == 0) return;
    pv = *(char **)(moby + 0x78);
    if (pv == 0) return;

    idx = *(int *)(pv + 0x48);
    if (idx != -1) {
        char *L = MOBY_AT_30F318(idx);
        float dx, dy, ang;
        char *lp;
        if ((unsigned char)moby[0x20] >= 5) {
            dx = *(float *)(moby + 0x10) - *(float *)(L + 0x10);
            dy = *(float *)(moby + 0x14) - *(float *)(L + 0x14);
        } else {
            dx = *(float *)(hero + 0x80) - *(float *)(L + 0x10);
            dy = *(float *)(hero + 0x84) - *(float *)(L + 0x14);
        }
        ang = func_L00_001FF860_30F318(dx, dy);
        lp = *(char **)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0x78);
        if (lp) *(float *)lp = ang;
        if (*(int *)(pv + 0x48) != -1) {
            *(int *)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0x94) = 0;
        }
    }

    st = (unsigned char)moby[0x20];
    if (st >= 3) {
        if (D_0015EEB0_30F318[0] != 0) {
            if ((unsigned char)pv[0x91] == 0) {
                func_0020D960_30F318(moby, 5, pv + 0x90);
            }
            *(float *)(pv + 0xB8) = 2.1f;
            *(float *)(pv + 0xB0) = 2.1f;
            *(float *)(pv + 0xB4) = 2.1f;
        } else if ((unsigned char)pv[0x91] != 0) {
            func_0020D9D8_30F318(moby, pv + 0x90);
        }
        func_L01_0030F2A8_30F318(moby);
        st = (unsigned char)moby[0x20];
    }

    switch (st) {
    case 0: {
        short r;
        QZERO_30F318(pv);
        QZERO_30F318(pv + 0x10);
        *(short *)(pv + 0x50) = 0;
        *(short *)(pv + 0x52) = 0;
        if (moby[0x53] != 0) {
            func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(10));
        }
        moby[0x30] = (char)0xFF;
        *(short *)(moby + 0x32) = 0xFF;
        idx = *(int *)(pv + 0x48);
        if (idx == -1) {
            func_001E9730_30F318(D_L01_0020BD20_30F318);
            func_0020D678_30F318(moby);
            return;
        }
        QCOPY_30F318(moby + 0x10, MOBY_AT_30F318(idx) + 0x10);
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + 0.02f;
        r = *(short *)(moby + 0xB2);
        if (D_L01_001BB9C0_30F318[0x454 + r] != 0 ||
            ((*(int *)(D_0014171B_30F318 + 0xAB75 + (D_0015EE84_30F318 << 8) + ((r >> 5) << 2)) >> (r & 0x1F)) & 1)) {
            func_001E9730_30F318(D_L01_0020BD00_30F318);
            func_0020D678_30F318(moby);
            return;
        }
        QCOPY_30F318(pv + 0x20, moby + 0x10);
        QCOPY_30F318(pv + 0x30, moby + 0x10);
        func_L01_0026E8E0_30F318(S);
        *(int *)(S + 0x00) = 500;
        *(float *)(S + 0x0C) = 0.15f;
        *(float *)(S + 0x14) = D_0015EE60_30F318 * 0.4f;
        *(float *)(S + 0x24) = D_0015EE6C_30F318 * 2.7f;
        *(float *)(S + 0x1C) = D_0015EE60_30F318 * 0.01f;
        *(float *)(S + 0x04) = 0.3f;
        *(float *)(S + 0x30) = 0.005f;
        *(int *)(S + 0x38) = *(int *)(S + 0x38) | 8;
        *(float *)(S + 0x3C) = D_0015EE70_30F318 * bits_30F318(0x40C90FDB);
        *(float *)(S + 0x40) = D_0015EE70_30F318 * PI_30F318;
        *(float *)(S + 0x44) = D_0015EE6C_30F318 * bits_30F318(0x41384E88);
        *(float *)(S + 0x08) = 0.15f;
        *(float *)(S + 0x10) = D_0015EE60_30F318 * 0.01f;
        *(int *)(S + 0x48) = func_001F9850_30F318(4);
        *(float *)(S + 0x28) = PI_30F318;
        moby[0x20] = 1;
        *(short *)(moby + 0x32) = 0;
        break;
    }
    case 1: {
        int k;
        if (D_0013D5CA_30F318[3] == 0) break;
        if (*(int *)(hero + 0x1158) != 5) break;
        *(int *)(pv + 0x5C) = 0;
        *(int *)(pv + 0x58) = func_001F9850_30F318(D_L01_00162128_30F318);
        if (moby[0x31] != 0 && (moby[0x70] & 2) && moby[0x53] != 2) {
            func_00213DE0_30F318(moby, 2, 0, func_001F9850_30F318(10));
        }
        if (*(int *)(pv + 0x48) != -1) {
            int v = func_00215570_30F318(hero + 0x80, *(int *)(pv + 0x40));
            *(unsigned char *)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0xBC) = v ? 3 : 1;
        }
        if (*(int *)(hero + 0x1130) == 0) break;
        if (*(int *)(hero + 0x1158) != 5) break;
        face_30F318(moby, *(float *)(hero + 0x80), *(float *)(hero + 0x84));
        {
            float a[4], b[4];
            QCOPY_30F318(a, MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0x10);
            a[2] = a[2] + 0.5f;
            QCOPY_30F318(b, hero + 0x80);
            b[2] = b[2] + 0.2f;
            if (func_00215570_30F318(hero + 0x80, *(int *)(pv + 0x40)) == 0) break;
            k = func_L00_001EFFF0_30F318(a, b, 0, moby, 0);
            if (k != 0 && D_L01_00174358_30F318 != *(int *)(hero + 0x2080)) break;
        }
        func_L01_002E4430_30F318(moby, pv);
        {
            char *h = *(char **)(hero + 0x1130);
            if (h != 0 && *(short *)(h + 0xA6) == 0x1B1) {
                unsigned char s = (unsigned char)h[0x20];
                if (s != 0xFE && s != 0xFD) h[0x20] = 1;
            }
        }
        *(char **)(S + 0x50) = moby;
        func_L01_0023D688_30F318(0x1F, 0);
        func_L00_00232C10_30F318(0, 0, (float)func_001F9850_30F318(6));
        if (*(int *)(pv + 0x48) != -1) {
            *(unsigned char *)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0xBC) = 5;
            moby[0x20] = 2;
        } else {
            *(short *)(moby + 0x32) = 0xFF;
            if (moby[0x53] != 0) {
                func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(10));
            }
            moby[0x20] = 3;
        }
        break;
    }
    case 2: {
        idx = *(int *)(pv + 0x48);
        if (idx != -1) {
            if ((unsigned char)MOBY_AT_30F318(idx)[0x20] != 5) break;
            if (moby[0x53] != 0) {
                func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(10));
            }
            moby[0x20] = 3;
            *(short *)(pv + 0x52) = 0;
            *(short *)(moby + 0x32) = 0xFF;
        } else {
            *(short *)(pv + 0x52) = 0;
            *(short *)(moby + 0x32) = 0xFF;
            if (moby[0x53] != 0) {
                func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(20));
            }
            if (*(int *)(pv + 0x64) == -1) {
                *(int *)(pv + 0x64) = func_0022ED80_30F318(1, 4, moby);
            }
            moby[0x20] = 3;
        }
        break;
    }
    case 3: {
        int v2;
        float d;
        ensure_slot_30F318(moby, pv);
        d = func_001F9D48_30F318(moby + 0x10, pv + 0x30);
        if (0.6f < d && *(short *)(pv + 0x52) == 0) {
            *(float *)(S + 0x04) = 0.7f;
            func_L01_0030E638_30F318(moby, pv);
            func_L01_0030EC58_30F318(moby, pv);
            v2 = (*(short *)(pv + 0x50) < 0x15) ? 0 : 2;
        } else {
            func_L01_002E4430_30F318(moby, pv);
            face_30F318(moby, *(float *)(pv + 0x00), *(float *)(pv + 0x04));
            *(int *)(pv + 0x18) = 0;
            v2 = func_L00_0025A060_30F318(moby, S, pv, pv + 0x10);
        }
        if ((v2 & 6) != 0 || func_001F9D10_30F318(moby + 0x10, pv) < 0.25f) {
            if (moby[0x53] != 2) {
                func_00213DE0_30F318(moby, 2, 0, func_001F9850_30F318(10));
            }
            QZERO_30F318(pv + 0x10);
            moby[0x20] = 4;
            *(short *)(pv + 0x52) = (short)func_001F9850_30F318(0x78);
        }
        if (moby[0x70] & 2) {
            talk_anim_30F318(moby);
        }
        break;
    }
    case 4: {
        ensure_slot_30F318(moby, pv);
        func_L01_0023D688_30F318(0, 1);
        *(unsigned short *)(moby + 0x34) &= 0xFFBE;
        idx = *(int *)(pv + 0x48);
        if (idx != -1) {
            *(unsigned short *)(MOBY_AT_30F318(idx) + 0x34) &= 0xFFBE;
        }
        moby[0x20] = 5;
        break;
    }
    case 5: {
        int s16, go_chat = 0;
        ensure_slot_30F318(moby, pv);
        s16 = *(short *)(pv + 0x52);
        if (s16 == func_001F9850_30F318(20) || *(int *)(pv + 0x5C) > D_L01_0016212C_30F318 - 5) {
            if (moby[0x53] != 2) {
                func_00213DE0_30F318(moby, 2, 0, func_001F9850_30F318(5));
            }
        } else {
            s16 = *(short *)(pv + 0x52);
            if (func_001F9850_30F318(20) < s16) {
                if (moby[0x70] & 2) {
                    talk_anim_30F318(moby);
                }
            }
        }
        if (*(int *)(hero + 0x1130) != 0 &&
            *(int *)(hero + 0x1158) == 5 &&
            *(int *)(hero + 0x208C) != 0x16 &&
            *(int *)(hero + 0x208C) != 0x14 &&
            *(unsigned char *)(hero + 0x20A4) == 0 &&
            *(short *)(D_L01_00167300_30F318 + 0x86) != 0x14 &&
            func_001F9908_30F318(pv + 0x58) == 0 &&
            *(int *)(pv + 0x5C) < D_L01_0016212C_30F318) {
            func_L01_0030E638_30F318(moby, pv);
            func_L01_0030EC58_30F318(moby, pv);
            go_chat = 1;
        }
        if (!go_chat) {
            QZERO_30F318(pv + 0x10);
            *(short *)(pv + 0x52) = (short)D_L01_00162134_30F318;
            {
                int k = *(int *)(pv + 0x64);
                if (k != -1 && slot_ok_30F318(moby, k)) {
                    func_L00_0028EBF0_30F318(k);
                }
            }
            *(int *)(pv + 0x64) = -1;
            func_0022ED80_30F318(2, 0, moby);
            moby[0x20] = 6;
            if (*(int *)(pv + 0x48) != -1) {
                *(unsigned char *)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0xBC) = 5;
            }
        }
        func_L01_0030E9C8_30F318(moby, pv);
        break;
    }
    case 6: {
        drop_slot_30F318(moby, pv);
        if (func_001F9938_30F318(pv + 0x52) != 0) {
            moby[0x23] = (char)0x80;
            QCOPY_30F318(moby + 0x10, pv + 0x30);
            QCOPY_30F318(pv, pv + 0x30);
            QZERO_30F318(pv + 0x10);
            moby[0x20] = 7;
            *(unsigned short *)(moby + 0x34) &= 0xFDFF;
        } else {
            float p[4], vel[4];
            unsigned c;
            int r4, sgn, life, q;
            float size0;
            QZERO_30F318(vel);
            QCOPY_30F318(p, moby + 0x10);
            func_L00_00260958_30F318(p, D_L01_00162140_30F318);
            c = (unsigned)((func_001160D8_30F318() + 0x30) & 0x3F);
            c |= (unsigned)((func_001160D8_30F318() + 0x20) & 0x3F) << 8;
            c |= (unsigned)(func_001160D8_30F318() & 0x2F) << 16;
            c |= (unsigned)D_L01_0016214C_30F318 << 24;
            r4 = func_001160D8_30F318();
            sgn = r4 % 2;
            if (*(unsigned short *)(pv + 0x52) & 1) sgn = -sgn;
            life = func_L00_00258BC8_30F318(D_L01_00162138_30F318, D_L01_0016213C_30F318);
            size0 = func_002140F8_30F318(D_L01_00162144_30F318, D_L01_00162148_30F318);
            func_L00_00272F00_30F318(p, life, size0, 0.001f, (int)c, 2, sgn, vel,
                                     D_L01_00162150_30F318 * D_0015EE70_30F318);
            *(unsigned short *)(moby + 0x34) |= 0x200;
            q = D_L01_00162134_30F318 ? 0x80 / D_L01_00162134_30F318 : 0;
            moby[0x23] = (char)((unsigned char)moby[0x23] - q);
            *(float *)(moby + 0x48) = func_001FA748_30F318(*(float *)(moby + 0x48),
                                                         D_0015EE6C_30F318 * bits_30F318(0x41C90FDB));
        }
        break;
    }
    case 7: {
        char *L;
        drop_slot_30F318(moby, pv);
        QZERO_30F318(pv + 0x10);
        L = MOBY_AT_30F318(*(int *)(pv + 0x48));
        QCOPY_30F318(pv, L + 0x10);
        *(int *)(S + 0x50) = 0;
        face_30F318(moby, *(float *)(pv + 0x00), *(float *)(pv + 0x04));
        if (moby[0x53] != 0) {
            func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(6));
        }
        *(unsigned char *)(MOBY_AT_30F318(*(int *)(pv + 0x48)) + 0xBC) = 1;
        moby[0x20] = 1;
        *(short *)(moby + 0x32) = 0;
        break;
    }
    default:
        break;
    }

    if ((unsigned char)moby[0x20] >= 2) {
        func_L00_0025B040_30F318(moby, 0.3f);
        func_L01_0030F178_30F318(moby, pv);
    }
    idx = *(int *)(pv + 0x48);
    if (idx != -1) {
        char *L = MOBY_AT_30F318(idx);
        *(int *)(L + 0x94) = *(int *)(*(char **)(L + 0x24) + 0x10);
    }
}
