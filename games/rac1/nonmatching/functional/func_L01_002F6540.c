/* functional: medium. Same reads, writes, calls and arguments in the same order as retail (no COP2/VU0 code; the lq/sq pairs are plain 16-byte copies, the jump table is a switch); the two divisions by a record float are guarded so that a zero divisor gives FLT_MAX as on the PS2; differs otherwise only in compiler choices. */
/* Level 1 (Novalis) per-frame update of a moby that follows a path (a "rail" mover with a state record
 * rec = m[0x78]; its follower state is rec+0x60, "pf": pf[0] = current point, pf[4] direction, pf[5] mode,
 * pf[0x10] the point list (count then 16-byte points), pf[0x14] the path index into D_L01_001B0C30). Called from the
 * level's moby update and from the Kerwan/Eudora ones. m[0x20] is its state:
 *  - 0: pick the path point list, move along it (func_L01_0028C690/C848/C958 steps, speed rec+0xFC, wobble
 *    rec+0x124/0x128/0x12C), set the facing m[0x44]/m[0x48], position m[0x10], spawn the tracked child mobys
 *    (rec+0x140..) and go to 3 while the hurt timer rec+0x11C is positive, else to 1;
 *  - 1: the follow state proper: advance by rec+0xF0, smooth the facing and roll/pitch with func_L00_00259148, and,
 *    when the moby is not the "special" kind, tilt them to the motion; collect the children (rec+0x120 bit mask); on the
 *    level kinds 3 and 9 throw debris (func_L00_0025F4A8) and run the death countdown rec+0x13A;
 *  - 2: leave to 0, freeing the effect slot rec+0x11E; 3: wait for the timer rec+0x11C then back to 1.
 * Before that: the group (m[0x21]) is put in state 3 when none of its members can see (func_L01_00276680), the
 * classes 0x33/0x46A/0x473/0x474 get the effect slot rec+0x11E, and classes 0x33, 0x3C, 0x4A and 0x40, 0x44 run the two reaction timers
 * rec+0x138 (jump table jtbl_L01_0020B700). "special" = func_L01_00276680 returned -1 (only for class != 0x336). */
typedef int Q_2F6540 __attribute__((mode(TI)));
typedef union { Q_2F6540 q; float f[4]; } U_2F6540;

extern int D_L01_00161B90_2F6540 __asm__("D_L01_00161B90") MACRO_ADDR;
extern int D_L01_00160058_2F6540 __asm__("D_L01_00160058") MACRO_ADDR;
extern int D_0015EE84_2F6540 __asm__("D_0015EE84") MACRO_ADDR;
extern int D_L01_0015F6B0_2F6540 __asm__("D_L01_0015F6B0");
extern char D_L01_001672C0_2F6540[] __asm__("D_L01_001672C0");
extern char D_L01_0020B6D0_2F6540[] __asm__("D_L01_0020B6D0");
extern char *D_L01_001B0C30_2F6540[] __asm__("D_L01_001B0C30");
extern char D_0013E633_2F6540[] __asm__("D_0013E633");
extern float D_0015EE60_2F6540 __asm__("D_0015EE60");
extern float D_0015EE6C_2F6540 __asm__("D_0015EE6C");
extern float D_0015EE70_2F6540 __asm__("D_0015EE70");
extern int D_L01_001607F0_2F6540 __asm__("D_L01_001607F0");

extern float func_001FA888_2F6540(int) __asm__("func_001FA888");
extern int func_L01_00276680_2F6540(char *, float) __asm__("func_L01_00276680");
extern int func_L00_0025A208_2F6540(int *, int, int, int) __asm__("func_L00_0025A208");
extern int func_L00_0025A2F0_2F6540(int *, int, int, int) __asm__("func_L00_0025A2F0");
extern int func_001F9850_2F6540(int) __asm__("func_001F9850");
extern int func_L00_0028EB98_2F6540(void *, int) __asm__("func_L00_0028EB98");
extern int func_0022ED80_2F6540(int, int, void *) __asm__("func_0022ED80");
extern int func_001F9938_2F6540(void *) __asm__("func_001F9938");
extern float func_001F9D10_2F6540(void *, void *) __asm__("func_001F9D10");
extern int func_002140B0_2F6540(int) __asm__("func_002140B0");
extern int func_L00_00258BC8_2F6540(int, int) __asm__("func_L00_00258BC8");
extern int func_001FA898_2F6540(float) __asm__("func_001FA898");
extern int func_001E9730_2F6540(char *, int, int) __asm__("func_001E9730");
extern float func_002140F8_2F6540(float, float) __asm__("func_002140F8");
extern float func_L00_001FF860_2F6540(float, float) __asm__("func_L00_001FF860");
extern void func_L01_0028C1D8_2F6540(char *) __asm__("func_L01_0028C1D8");
extern int func_L01_0028C2D8_2F6540(void *, void *, float) __asm__("func_L01_0028C2D8");
extern void func_L01_0028C690_2F6540(void *, int *, int, float, float) __asm__("func_L01_0028C690");
extern float func_001F9CE8_2F6540(void *) __asm__("func_001F9CE8");
extern void func_L01_002F6418_2F6540(char *, void *) __asm__("func_L01_002F6418");
extern float func_00214158_2F6540(void) __asm__("func_00214158");
extern void func_L01_0028C958_2F6540(void *, int, void *, float, float) __asm__("func_L01_0028C958");
extern void func_L00_00250800_2F6540(void *, int, void *) __asm__("func_L00_00250800");
extern void func_001F9BF0_2F6540(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_001FF4B0_2F6540(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_L00_0028EBF0_2F6540(int) __asm__("func_L00_0028EBF0");
extern int func_L01_0028C640_2F6540(void *) __asm__("func_L01_0028C640");
extern int func_L01_0028C5F0_2F6540(void *, int) __asm__("func_L01_0028C5F0");
extern float func_001F9FA8_2F6540(float) __asm__("func_001F9FA8");
extern float func_001F9F90_2F6540(float) __asm__("func_001F9F90");
extern float func_001FA748_2F6540(float, float) __asm__("func_001FA748");
extern float func_001FA790_2F6540(float, float) __asm__("func_001FA790");
extern void func_L01_0028C848_2F6540(float, void *, void *, void *, void *, void *) __asm__("func_L01_0028C848");
extern void func_001F9BC0_2F6540(void *) __asm__("func_001F9BC0");
extern float func_L00_00259148_2F6540(float, float, float, float, float, void *) __asm__("func_L00_00259148");
extern float func_L00_0025F368_2F6540(float) __asm__("func_L00_0025F368");
extern void func_L01_00280428_2F6540(void *, void *) __asm__("func_L01_00280428");
extern char *func_L00_0025B478_2F6540(void *, int, int) __asm__("func_L00_0025B478");
extern void func_001F9C30_2F6540(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BD8_2F6540(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_0025F4A8_2F6540(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");

#define F6540(p, o) (*(float *)((p) + (o)))
#define I6540(p, o) (*(int *)((p) + (o)))
#define H6540(p, o) (*(short *)((p) + (o)))
#define UH6540(p, o) (*(unsigned short *)((p) + (o)))
#define B6540(p, o) (*(unsigned char *)((p) + (o)))
#define Q6540(p, o) (*(Q_2F6540 *)((p) + (o)))

/* retail divides without a check; x/0 is +-FLT_MAX on the PS2, never inf (the bits, not a literal, so nothing goes to rodata) */
#define DIV6540(dst, a, b)                                                    \
    do {                                                                      \
        if ((b) == 0.0f) {                                                    \
            dbits = ((a) < 0.0f) ? 0xFF7FFFFF : 0x7F7FFFFF;                   \
            (dst) = *(float *)&dbits;                                         \
        } else {                                                              \
            (dst) = (a) / (b);                                                \
        }                                                                     \
    } while (0)

/* frees the effect slot rec+0x11E (func_L00_0028EBF0 when its entry still belongs to m), then marks it unused */
#define FREE_SLOT6540(m, rec)                                                 \
    do {                                                                      \
        fs5 = H6540(rec, 0x11E);                                              \
        if (fs5 != -1) {                                                      \
            fent = D_0013E633_2F6540 + 0x1D + fs5 * 0x70;                     \
            if (I6540(fent, 0x88) == (int)(m) && B6540(fent, 0x74) != 0) {    \
                func_L00_0028EBF0_2F6540(fs5);                                \
            }                                                                 \
        }                                                                     \
        H6540(rec, 0x11E) = -1;                                               \
    } while (0)

void func_L01_002F6540(char *m) {
    char *rec = *(char **)(m + 0x78);
    char *pf = rec + 0x60;
    int special = 0;
    int iter;
    int cls;
    int idx;
    int state;
    int i;
    int j;
    int ok;
    int id;
    int pid;
    int g;
    int bits;
    int dbits;
    int fs5;
    char *fent;
    char *s0;
    char *o;
    char *r;
    char *p;
    char *t;
    float f20;
    float f24;
    float f5;
    float s;
    U_2F6540 zero;
    U_2F6540 v50;
    U_2F6540 v60[4];
    U_2F6540 v70;
    U_2F6540 v80;
    U_2F6540 vA0;
    U_2F6540 v;

    zero.q = 0;
    if (H6540(m, 0xA6) != 0x336) {
        special = (func_L01_00276680_2F6540(m, func_001FA888_2F6540(H6540(m, 0x32))) == -1);
    }
    if (rec == 0) {
        return;
    }
    if (D_L01_00161B90_2F6540 != 0) {
        return;
    }

    /* the group of the moby: when none of its members is allowed, all go to state 3 */
    if (B6540(m, 0x21) != 0xFF && B6540(m, 0x20) != 0 && B6540(m, 0x20) != 3) {
        ok = 0;
        func_L00_0025A208_2F6540(&iter, B6540(m, 0x21), 0, 0);
        if (iter == (int)m) {
            s0 = (char *)iter;
            while (s0 != 0) {
                if (func_L01_00276680_2F6540(s0, func_001FA888_2F6540(H6540(s0, 0x32))) != -1) {
                    ok = 1;
                    break;
                }
                func_L00_0025A2F0_2F6540(&iter, iter, 0, 0);
                s0 = (char *)iter;
            }
            if (!ok) {
                func_L00_0025A208_2F6540(&iter, B6540(m, 0x21), 0, 0);
                if (iter != 0) {
                    do {
                        o = (char *)iter;
                        s0 = *(char **)(o + 0x78);
                        B6540(o, 0x20) = 3;
                        H6540(s0, 0x11C) = func_001F9850_2F6540(5);
                        func_L00_0025A2F0_2F6540(&iter, iter, 0, 0);
                    } while (iter != 0);
                }
            }
        }
    }

    cls = H6540(m, 0xA6);
    if (cls == 0x33 || cls == 0x46A || cls == 0x473 || cls == 0x474) {
        if ((D_L01_0015F6B0_2F6540 & 7) == (((int)m >> 8) & 7)
            && H6540(rec, 0x13A) <= 0
            && B6540(m, 0x20) != 0
            && func_L00_0028EB98_2F6540(m, H6540(rec, 0x11E)) == 0) {
            H6540(rec, 0x11E) = func_0022ED80_2F6540(0, 4, m);
        }
    }

    idx = (short)(UH6540(m, 0xA6) - 0x33);
    if ((unsigned)idx < 0x18) {
        switch (idx) {
        case 0:
        case 9:
        case 0x17:
            if (H6540(rec, 0x138) != -1) {
                if (func_001F9938_2F6540(rec + 0x138) == 0) {
                    break;
                }
                if (func_001F9D10_2F6540(m + 0x10, D_L01_001672C0_2F6540) < 30.0f) {
                    func_0022ED80_2F6540(1, 0, m);
                }
            }
            H6540(rec, 0x138) = func_001F9850_2F6540(
                func_L00_00258BC8_2F6540(H6540(rec, 0x13C), H6540(rec, 0x13C) + H6540(rec, 0x13E)));
            break;
        case 13:
        case 17:
            if (H6540(rec, 0x138) != -1) {
                if (func_001F9938_2F6540(rec + 0x138) == 0) {
                    goto dispatch;
                }
                func_0022ED80_2F6540((func_002140B0_2F6540(100) & 1) ? 1 : 2, 0, m);
            }
            H6540(rec, 0x138) = func_001F9850_2F6540(
                func_L00_00258BC8_2F6540(H6540(rec, 0x13C), H6540(rec, 0x13C) + H6540(rec, 0x13E)));
            break;
        default:
            break;
        }
    }

dispatch:
    state = B6540(m, 0x20);
    if (state == 0) {
        pid = I6540(pf, 0x14);
        if (pid != -1) {
            p = D_L01_001B0C30_2F6540[pid];
            if (func_001F9D10_2F6540(p + 0x10, p + (*(int *)p << 4)) < 0.5f) {
                *(int *)p = *(int *)p - 1;
            }
        } else {
            func_001E9730_2F6540(D_L01_0020B6D0_2F6540, H6540(m, 0xA6),
                                 func_001FA898_2F6540((float)(((int)m - D_L01_00160058_2F6540) >> 8)));
        }
        B6540(m, 0x30) = 0xFF;
        H6540(rec, 0x11E) = -1;
        f20 = 500.0f;
        if (f20 < F6540(rec, 0x130)) {
            F6540(rec, 0x130) = func_002140F8_2F6540(-1.0f, 1.0f) * F6540(rec, 0x130) / 1000.0f;
        }
        if (f20 < F6540(rec, 0x134)) {
            F6540(rec, 0x134) = func_002140F8_2F6540(-1.0f, 1.0f) * F6540(rec, 0x134) / 1000.0f;
        }
        if (I6540(pf, 0x14) == -1) {
            B6540(m, 0x20) = 2;
            return;
        }
        B6540(rec, 0x121) = (((int)m - D_L01_00160058_2F6540) >> 8) / 2 % 7;
        func_L01_0028C1D8_2F6540(pf);
        if (B6540(pf, 5) == 0) {
            if (!(I6540(pf, 0) > 0)) {
                idx = func_L01_0028C2D8_2F6540(m + 0x10, *(void **)(pf + 0x10), 0.0f);
                I6540(pf, 0) = idx;
                if (*(signed char *)(pf + 4) >= 0) {
                    if (idx == **(int **)(pf + 0x10) - 1) {
                        I6540(pf, 0) = 0;
                    }
                } else if (idx == 0) {
                    I6540(pf, 0) = 0;
                }
            }
            F6540(pf, 0xC) = func_001F9D10_2F6540(m + 0x10, *(char **)(pf + 0x10) + ((I6540(pf, 0) << 4) + 0x10));
            p = *(char **)(pf + 0x10) + (I6540(pf, 0) << 4);
            F6540(pf, 0x18) = func_L00_001FF860_2F6540(F6540(p, 0x10) - F6540(m, 0x10), F6540(p, 0x14) - F6540(m, 0x14));
            B6540(pf, 5) = 2;
            I6540(pf, 8) = 0;
            I6540(pf, 0x1C) = 0;
        }
        Q6540(rec, 0xE0) = Q6540(*(char **)(pf + 0x10) + (I6540(pf, 0) << 4), 0x10);
        F6540(rec, 0xF0) = 1.0f;
        func_L01_0028C690_2F6540(rec + 0xA0, (int *)pf, 0, F6540(rec, 0xF4), F6540(rec, 0xF8));
        v.q = Q6540(*(char **)(pf + 0x10) + (I6540(pf, 0) << 4), 0x10);
        Q6540(rec, 0xB0) = v.q;
        Q6540(rec, 0xC0) = v.q;
        I6540(m, 0x40) = 0;
        s = func_001F9CE8_2F6540(rec + 0xA0);
        F6540(m, 0x44) = -func_L00_001FF860_2F6540(s, F6540(rec, 0xA8));
        F6540(m, 0x48) = func_L00_001FF860_2F6540(F6540(rec, 0xA0) - zero.f[0], F6540(rec, 0xA4) - zero.f[1]);
        I6540(rec, 0x104) = 0;
        I6540(rec, 0x10C) = 0;
        Q6540(m, 0x10) = Q6540(rec, 0xE0);
        func_L01_002F6418_2F6540(m, m + 0x10);
        B6540(rec, 0x121) = 7;
        s = F6540(rec, 0x12C);
        if (6.2831855f < s) {
            F6540(rec, 0x12C) = s * 0.017453292f;
        } else if (!(F6540(rec, 0x124) == 0.0f) && s == -1.0f) {
            F6540(rec, 0x12C) = func_00214158_2F6540();
        }
        Q6540(rec, 0xD0) = Q6540(rec, 0xC0);
        func_L01_0028C958_2F6540(pf, B6540(rec, 0x120) & 0x20, m, F6540(rec, 0xF4), F6540(rec, 0xF8));
        bits = B6540(rec, 0x120);
        if ((bits & 0x1E) != 0 && (bits & 1) == 0) {
            for (i = 0; i < 4; i++) {
                if (B6540(rec, 0x120) & (2 << i)) {
                    id = I6540(rec, 0x140 + i * 4);
                    if (id != -1) {
                        o = (char *)(D_L01_00160058_2F6540 + (id << 8));
                        if (o != 0) {
                            r = *(char **)(o + 0x78);
                            if (r != 0) {
                                I6540(r, 0x70) = I6540(r, 0x70) | 0x1000;
                                func_L00_00250800_2F6540(m, i, &v60[i]);
                                func_001F9BF0_2F6540(&vA0, &v60[i], m + 0x10);
                                func_L00_001FF4B0_2F6540(&vA0, &vA0, 1.0f);
                                Q6540(o, 0x10) = v60[i].q;
                                Q6540(r, 0x80) = vA0.q;
                            }
                        }
                    }
                }
            }
        }
        if (H6540(rec, 0x11C) > 0) {
            B6540(m, 0x20) = 3;
            return;
        }
        if (D_0015EE84_2F6540 == 3 || D_0015EE84_2F6540 == 9) {
            H6540(rec, 0x13A) = 0;
        }
        B6540(m, 0x20) = 1;
        return;
    }
    if (state == 2) {
        if (I6540(pf, 0x14) == -1) {
            return;
        }
        B6540(m, 0x20) = 0;
        FREE_SLOT6540(m, rec);
        return;
    }
    if (state == 3) {
        if (func_001F9938_2F6540(rec + 0x11C) != 0) {
            H6540(rec, 0x11C) = 0;
            B6540(m, 0x20) = 1;
        }
        return;
    }
    if (state != 1) {
        return;
    }

    /* state 1 */
    v50.q = Q6540(m, 0x10);
    if (1.0f <= F6540(rec, 0xF0)) {
        F6540(rec, 0xF0) = F6540(rec, 0xF0) - 1.0f;
        idx = func_L01_0028C640_2F6540(pf);
        Q6540(rec, 0x90) = Q6540(rec, 0xA0);
        func_L01_0028C690_2F6540(rec + 0xA0, (int *)pf, 1, F6540(rec, 0xF4), F6540(rec, 0xF8));
        Q6540(rec, 0xB0) = Q6540(rec, 0xC0);
        idx = func_L01_0028C5F0_2F6540(pf, idx + 1);
        Q6540(rec, 0xC0) = Q6540(*(char **)(pf + 0x10), (idx << 4) + 0x10);
    }
    s = func_001F9FA8_2F6540(F6540(rec, 0x12C));
    f20 = F6540(rec, 0xFC) + F6540(rec, 0x124) * D_0015EE60_2F6540 * s * D_0015EE60_2F6540;
    DIV6540(f24, f20, F6540(rec, 0xBC));
    F6540(rec, 0x12C) = func_001FA748_2F6540(F6540(rec, 0x12C), F6540(rec, 0x128) * D_0015EE60_2F6540 * 0.017453292f);
    Q6540(rec, 0xE0) = Q6540(rec, 0xD0);
    func_L01_0028C848_2F6540(F6540(rec, 0xF0) + f24, rec + 0xD0, rec + 0xB0, rec + 0xC0, rec + 0x90, rec + 0xA0);
    if (!(B6540(rec, 0x120) & 0x40)) {
        f5 = func_001F9D10_2F6540(rec + 0xE0, rec + 0xD0);
        if (f5 != 0.0f) {
            f24 = f24 * (f20 / f5);
            func_L01_0028C848_2F6540(F6540(rec, 0xF0) + f24, rec + 0xD0, rec + 0xB0, rec + 0xC0, rec + 0x90, rec + 0xA0);
        }
    }
    if (special) {
        func_001F9BC0_2F6540(m + 0x40);
        F6540(m, 0x48) = func_L00_001FF860_2F6540(F6540(rec, 0xD0) - F6540(rec, 0xE0), F6540(rec, 0xD4) - F6540(rec, 0xE4));
    } else {
        f20 = (float)H6540(rec, 0x122) * 0.017453292f;
        func_001FA790_2F6540(F6540(m, 0x48), f20); /* result not used by retail */
        s = func_L00_001FF860_2F6540(F6540(rec, 0xD0) - F6540(rec, 0xE0), F6540(rec, 0xD4) - F6540(rec, 0xE4));
        s = func_001FA748_2F6540(s, f20);
        s = func_001FA790_2F6540(s, F6540(m, 0x48));
        DIV6540(f5, 1.0f, F6540(rec, 0x110));
        f20 = s * (f5 * D_0015EE60_2F6540);
        F6540(m, 0x48) = func_001FA748_2F6540(f20, F6540(m, 0x48));
        func_001F9BF0_2F6540(&v70, rec + 0xD0, rec + 0xE0);
        s = func_001F9CE8_2F6540(&v70);
        s = func_L00_001FF860_2F6540(s, v70.f[2]);
        F6540(m, 0x44) = func_L00_00259148_2F6540(
            F6540(m, 0x44), -s,
            F6540(rec, 0x108) * 10.0f * 0.017453292f * D_0015EE70_2F6540,
            F6540(rec, 0x108) * 5.0f * 0.017453292f * D_0015EE70_2F6540,
            F6540(rec, 0x108) * 0.017453292f * D_0015EE6C_2F6540, rec + 0x10C);
        s = func_L00_0025F368_2F6540(f20 * F6540(rec, 0x114));
        s = func_001FA790_2F6540(F6540(m, 0x40), s);
        s = func_L00_0025F368_2F6540(s * F6540(rec, 0x118));
        F6540(m, 0x40) = func_L00_00259148_2F6540(
            F6540(m, 0x40), s,
            F6540(rec, 0x100) * 10.0f * 0.017453292f * D_0015EE70_2F6540,
            F6540(rec, 0x100) * 5.0f * 0.017453292f * D_0015EE70_2F6540,
            F6540(rec, 0x100) * 0.017453292f * D_0015EE6C_2F6540, rec + 0x104);
    }
    F6540(rec, 0xF0) = F6540(rec, 0xF0) + f24;
    Q6540(m, 0x10) = Q6540(rec, 0xE0);
    if (!special) {
        func_L01_002F6418_2F6540(m, m + 0x10);
    }

    g = D_0015EE84_2F6540;
    if ((B6540(rec, 0x120) & 0x1F) != 0) {
        j = 0;
        for (i = 0; i < 5; i++) {
            if ((B6540(rec, 0x120) >> i) & 1) {
                func_L00_00250800_2F6540(m, j, &v60[0]);
                j++;
                func_001F9BF0_2F6540(&v70, &v60[0], m + 0x10);
                if (B6540(rec, 0x120) & 1) {
                    if (B6540(rec, 0x121) == 0) {
                        func_L00_001FF4B0_2F6540(&v70, &v70, func_002140F8_2F6540(0.016f, 0.031f) * D_0015EE60_2F6540);
                        func_L01_00280428_2F6540(&v60[0], &v70);
                        func_L00_001FF4B0_2F6540(&v70, &v70, func_002140F8_2F6540(0.016f, 0.031f) * D_0015EE60_2F6540);
                        func_L01_00280428_2F6540(&v60[0], &v70);
                    }
                } else {
                    id = I6540(rec, 0x140 + (i - 1) * 4);
                    if (id != -1) {
                        o = (char *)(D_L01_00160058_2F6540 + (id << 8));
                        if (o != 0) {
                            r = *(char **)(o + 0x78);
                            if (r != 0) {
                                func_L00_001FF4B0_2F6540(&v70, &v70, 1.0f);
                                I6540(r, 0x70) = I6540(r, 0x70) | 0x1000;
                                Q6540(o, 0x10) = v60[0].q;
                                Q6540(r, 0x80) = v70.q;
                            }
                        }
                    }
                }
            }
        }
        if (B6540(rec, 0x121) == 0) {
            B6540(rec, 0x121) = func_001F9850_2F6540(7);
        }
        B6540(rec, 0x121) = B6540(rec, 0x121) - 1;
        g = D_0015EE84_2F6540;
    }

    if (g != 3 && g != 9) {
        return;
    }
    t = func_L00_0025B478_2F6540(m, 0x210000, 0);
    if (t == 0 || UH6540(t, 0x28) != 0x102) {
        if (t != 0) {
            func_001F9BC0_2F6540(&v60[0]);
            if (H6540(m, 0xA6) == 0x33) {
                v60[0].f[0] = func_001F9F90_2F6540(F6540(m, 0x48)) * -2.747f;
                v60[0].f[1] = func_001F9FA8_2F6540(F6540(m, 0x48)) * -2.747f;
                v60[0].f[2] = 0.0f;
            }
            func_001F9BF0_2F6540(&v80, m + 0x10, &v50);
            v70.q = v80.q;
            func_001F9C30_2F6540(&v70, &v70, 0.5f);
            func_001F9BD8_2F6540(&v60[0], &v60[0], m + 0x10);
            H6540(rec, 0x13A) = func_001F9850_2F6540(0x258);
            cls = H6540(m, 0xA6);
            id = (cls == 0x532 || cls == 0x53B || cls == 0x53C) ? 1 : -1;
            func_L00_0025F4A8_2F6540(m, &v70, &v60[0], 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 3.0f, id, 15.0f,
                                     1, 1, -1, 0);
        }
    }
    D_L01_001607F0_2F6540 = (int)func_L01_002F6540;
    B6540(m, 0xA4) = 0xFF;
    if (H6540(rec, 0x13A) == 0) {
        return;
    }
    if (func_001F9938_2F6540(rec + 0x13A) != 0 && special) {
        UH6540(m, 0x34) = (UH6540(m, 0x34) & 0xFFBE) | 0x1000;
        I6540(m, 0x94) = I6540(*(char **)(m + 0x24), 0x10);
        return;
    }
    I6540(m, 0x94) = 0;
    UH6540(m, 0x34) = UH6540(m, 0x34) | 0x41;
    FREE_SLOT6540(m, rec);
    UH6540(m, 0x34) = UH6540(m, 0x34) & 0xEFFF;
}
