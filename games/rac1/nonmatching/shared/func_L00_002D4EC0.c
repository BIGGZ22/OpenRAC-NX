/* NON_MATCHING func_L00_002D4EC0 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 3036 / retail 3068, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Moby class 639 update, 3068 bytes: eases the moby's tracked floats (0x54..0x5C) toward targets per flag bits i
 *   Best: p6.c, 3036 bytes, 32 short. The gap is three joins of the form `if (x == 0) {A}` with retail's `beqz $2;
 *   Unblock: the C form that makes gcc 2.95 emit an always-taken `bnel $2,$0` on a constant 1 (perhaps a flag vari
 *   Declarations: func_L00_00200290 and func_0020D678 are declared later in vendor_002D1168.c with V_D83D8 and P t
 */
extern int func_L00_00200290_v(void *, float) __asm__("func_L00_00200290");
extern void func_0020D678_m(void *) __asm__("func_0020D678");
extern char *D_L00_00160098 MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern unsigned char D_L00_001803C0[];
extern float func_001FA888(int);
extern int func_001F9908(int *);
extern void func_L00_0023F1D0(int);
extern float func_002140F8(float, float);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9938(void *);
extern s16 func_001F9850_D83D8(int) __asm__("func_001F9850");
extern int func_L00_001FEF78(void *);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_L00_0023F0D0(float *, float, float, float, float, float);

/* moby class 639 update: runs its state machine and eases its tracked values toward their targets */
void func_L00_002D4EC0(unsigned char *m) {
    unsigned char *o;
    Vx tp;
    Vx t2;
    Vx t3;
    unsigned char *v21;
    void *v20;
    void *v19;
    int c0 = 0;
    int r;

    if (m == 0) return;
    o = *(unsigned char **)(m + 0x78);
    if (o == 0) return;
    if (m[0x20] == 0) {
        unsigned char b1c = o[0x1C];
        m[0x30] = 0xFF;
        m[0xBC] = b1c;
        qcopy(o + 0x10, m + 0x10);
        {
            unsigned short a6e = *(unsigned short *)(o + 0x6E);
            int a4c = *(int *)(o + 0x4C);
            unsigned short a6c = *(unsigned short *)(o + 0x6C);
            unsigned short a70 = *(unsigned short *)(o + 0x70);
            unsigned short a72 = *(unsigned short *)(o + 0x72);
            unsigned char a25 = o[0x25];
            *(int *)(o + 0x50) = -1;
            *(short *)(o + 0x76) = a6e;
            o[0x1C] = b1c;
            *(int *)(o + 0xC) = a4c;
            *(short *)(o + 0x74) = a6c;
            *(short *)(o + 0x78) = a70;
            *(short *)(o + 0x7A) = a72;
            *(float *)(o + 0x54) = func_001FA888(a25) / 100.0f;
            *(float *)(o + 0x58) = func_001FA888(o[0x2A]) / 100.0f;
            {
                float f5 = func_001FA888(o[0x2F]) / 100.0f;
                *(float *)(o + 0x60) = *(float *)(o + 0x34);
                *(float *)(o + 0x5C) = f5;
            }
        }
        if (*(int *)(o + 0x64) != -1) {
            *(char **)(o + 0x20) = D_L00_00160098 + (*(int *)(o + 0x64) << 8);
        }
        if (*(float *)(o + 0x40) < 0.0f) *(float *)(o + 0x40) = -*(float *)(o + 0x40);
        if (*(float *)(o + 0x44) < 0.0f) *(float *)(o + 0x44) = -*(float *)(o + 0x44);
        m[0x20] = 1;
    }
    if (m[0xBC] != 0) {
        m[0xBC] = m[0xBC] - 1;
        return;
    }
    if (*(int *)(o + 0x48) != -1 && func_001F9908((int *)(o + 0x48))) {
        if (*(int *)(o + 0x50) != -1) {
            func_L00_0023F1D0(*(int *)(o + 0x50));
            *(int *)(o + 0x50) = -1;
        }
        func_0020D678_m(m);
        return;
    }
    v21 = m + 0x10;
    qcopy(&tp, m + 0x10);
    tp.w = *(float *)(o + 0x60);
    r = func_L00_00200290_v(&tp, 255.0f);
    if (r != -1) {
        if (*(unsigned char **)(o + 0x20) != 0) {
            qcopy(&t2, *(unsigned char **)(o + 0x20) + 0x10);
        } else {
            qcopy(&t2, o + 0x10);
        }
        v20 = &t2;
        if (!(*(int *)(o + 0x4C) & 0x800000)) {
            if (*(int *)(o + 0x4C) & 0x100000) {
                if (*(int *)(o + 0x4C) & 0x200000) {
                    t3.x = func_002140F8(-*(float *)(o + 0x0), *(float *)(o + 0x0));
                    t3.y = func_002140F8(-*(float *)(o + 0x4), *(float *)(o + 0x4));
                    t3.z = func_002140F8(-*(float *)(o + 0x8), *(float *)(o + 0x8));
                    v19 = &t3;
                } else {
                    int c = *(int *)(o + 0xC);
                    void *p = &t3;
                    *(int *)(o + 0xC) = 0;
                    v19 = p;
                    func_001F9C30(p, o, func_002140F8(*(float *)(o + 0xC), 1.0f));
                    *(int *)(o + 0xC) = c;
                }
            } else {
                int c = *(int *)(o + 0xC);
                void *p = &t3;
                *(int *)(o + 0xC) = 0;
                qcopy(p, o);
                *(int *)(o + 0xC) = c;
                v19 = p;
            }
            if (*(int *)(o + 0x4C) & 0x400000) {
                if (*(unsigned char **)(o + 0x20) != 0) {
                    func_001F9EC0(v19, v19, *(unsigned char **)(o + 0x20) + 0xC0);
                }
            }
            func_001F9BD8(v21, v20, v19);
        } else {
            qcopy(v21, v20);
        }
        if (*(unsigned short *)(o + 0x72) != 0) {
            if (func_001F9938(o + 0x7A)) {
                *(short *)(o + 0x7A) = func_001F9850_D83D8(*(unsigned short *)(o + 0x72));
                if (*(int *)(o + 0x4C) & 4) {
                    c0 = 1;
                    *(float *)(o + 0x60) = *(float *)(o + 0x3C);
                    *(int *)(o + 0x4C) &= 0xFFFFFFF7;
                }
            }
        }
        if (!(*(int *)(o + 0x4C) & 0x80000)) {
            s64 q = *(s64 *)(o + 0x48);
            if ((q & (0xC000L << 18)) == (0x8000L << 18)) *(int *)(o + 0x4C) ^= 0x2;
            if (!(*(int *)(o + 0x4C) & 0x2)) {
                if (*(int *)(o + 0x4C) & 0x8) {
                    float t = *(float *)(o + 0x60) - *(float *)(o + 0x44) * D_0015EE60;
                    *(float *)(o + 0x60) = t;
                    if (t <= *(float *)(o + 0x3C)) {
                        *(float *)(o + 0x60) = *(float *)(o + 0x3C);
                        if (*(int *)(o + 0x4C) & 0x4) {
                            if (func_001F9850_D83D8(*(unsigned short *)(o + 0x72)) == 0) *(int *)(o + 0x4C) ^= 0x8;
                        }
                    }
                } else {
                    float t = *(float *)(o + 0x60) + *(float *)(o + 0x40) * D_0015EE60;
                    *(float *)(o + 0x60) = t;
                    if (*(float *)(o + 0x38) <= t) {
                        *(float *)(o + 0x60) = *(float *)(o + 0x38);
                        if (!(*(int *)(o + 0x4C) & 0x1)) {
                            *(int *)(o + 0x4C) ^= 0x8;
                        } else {
                            *(unsigned char *)(o + 0x7F) = func_001F9850_D83D8(*(unsigned char *)(o + 0x6B));
                            *(int *)(o + 0x4C) = (*(int *)(o + 0x4C) | 0x2) ^ 0x8;
                        }
                    }
                }
            } else {
                if (*(unsigned char *)(o + 0x6B) != 0 && func_L00_001FEF78(o + 0x7F)) *(int *)(o + 0x4C) ^= 0x2;
            }
        } else {
            if (*(unsigned short *)(o + 0x72) == 0 || c0 == 1) {
                *(float *)(o + 0x60) = func_002140F8(*(float *)(o + 0x3C), *(float *)(o + 0x38));
            }
        }
        if (*(unsigned short *)(o + 0x6C) != 0) {
            if (func_001F9938(o + 0x74)) {
                *(short *)(o + 0x74) = func_001F9850_D83D8(*(unsigned short *)(o + 0x6C));
                *(int *)(o + 0x4C) &= 0xFFFFFF7F;
                *(float *)(o + 0x54) = (float)*(unsigned char *)(o + 0x27);
            }
        }
        if (!(*(int *)(o + 0x4C) & 0x10000)) {
            s64 q = *(s64 *)(o + 0x48);
            if ((q & (0xC000L << 22)) == (0x8000L << 22)) *(int *)(o + 0x4C) ^= 0x20;
            if (!(*(int *)(o + 0x4C) & 0x20)) {
                if (*(int *)(o + 0x4C) & 0x80) {
                    float a = func_001FA888(o[0x27]) / 100.0f;
                    float b = func_001FA888(o[0x29]) / 100.0f;
                    float t = *(float *)(o + 0x54) - b * D_0015EE60;
                    *(float *)(o + 0x54) = t;
                    if (t <= a) {
                        *(float *)(o + 0x54) = a;
                        if (*(int *)(o + 0x4C) & 0x40) {
                            if (*(unsigned short *)(o + 0x6C) == 0) *(int *)(o + 0x4C) ^= 0x80;
                        }
                    }
                } else {
                    float a = func_001FA888(o[0x26]) / 100.0f;
                    float b = func_001FA888(o[0x28]) / 100.0f;
                    float t = *(float *)(o + 0x54) + b * D_0015EE60;
                    *(float *)(o + 0x54) = t;
                    if (a <= t) {
                        *(float *)(o + 0x54) = a;
                        if (!(*(int *)(o + 0x4C) & 0x10)) {
                            *(int *)(o + 0x4C) ^= 0x80;
                        } else {
                            *(unsigned char *)(o + 0x7C) = func_001F9850_D83D8(o[0x68]);
                            *(int *)(o + 0x4C) = (*(int *)(o + 0x4C) | 0x20) ^ 0x80;
                        }
                    }
                }
            } else {
                if (o[0x68] != 0 && func_L00_001FEF78(o + 0x7C)) *(int *)(o + 0x4C) ^= 0x20;
            }
        } else {
            if (*(unsigned short *)(o + 0x6C) == 0) {
                float a = (float)func_001FA898_r((float)o[0x27]) / 100.0f;
                float b = (float)func_001FA898_r((float)o[0x26]) / 100.0f;
                *(float *)(o + 0x54) = func_002140F8(a, b);
            }
        }
        if (*(unsigned short *)(o + 0x6E) != 0) {
            if (func_001F9938(o + 0x76)) {
                *(short *)(o + 0x76) = func_001F9850_D83D8(*(unsigned short *)(o + 0x6E));
                *(int *)(o + 0x4C) &= 0xFFFFF7FF;
                *(float *)(o + 0x58) = (float)o[0x2C];
            }
        }
        if (!(*(int *)(o + 0x4C) & 0x20000)) {
            s64 q = *(s64 *)(o + 0x48);
            if ((q & (0xC000L << 26)) == (0x8000L << 26)) *(int *)(o + 0x4C) ^= 0x200;
            if (!(*(int *)(o + 0x4C) & 0x200)) {
                if (*(int *)(o + 0x4C) & 0x800) {
                    float a = func_001FA888(o[0x2C]) / 100.0f;
                    float b = func_001FA888(o[0x2E]) / 100.0f;
                    float t = *(float *)(o + 0x58) - b * D_0015EE60;
                    *(float *)(o + 0x58) = t;
                    if (t <= a) {
                        *(float *)(o + 0x58) = a;
                        if (*(int *)(o + 0x4C) & 0x400) {
                            if (*(unsigned short *)(o + 0x6E) == 0) *(int *)(o + 0x4C) ^= 0x800;
                        }
                    }
                } else {
                    float a = func_001FA888(o[0x2B]) / 100.0f;
                    float b = func_001FA888(o[0x2D]) / 100.0f;
                    float t = *(float *)(o + 0x58) + b * D_0015EE60;
                    *(float *)(o + 0x58) = t;
                    if (a <= t) {
                        *(float *)(o + 0x58) = a;
                        if (!(*(int *)(o + 0x4C) & 0x100)) {
                            *(int *)(o + 0x4C) ^= 0x800;
                        } else {
                            *(unsigned char *)(o + 0x7D) = func_001F9850_D83D8(o[0x69]);
                            *(int *)(o + 0x4C) = (*(int *)(o + 0x4C) | 0x200) ^ 0x800;
                        }
                    }
                }
            } else {
                if (o[0x69] != 0 && func_L00_001FEF78(o + 0x7D)) *(int *)(o + 0x4C) ^= 0x200;
            }
        } else {
            if (*(unsigned short *)(o + 0x6E) == 0) {
                float a = (float)func_001FA898_r((float)o[0x2C]) / 100.0f;
                float b = (float)func_001FA898_r((float)o[0x2B]) / 100.0f;
                *(float *)(o + 0x58) = func_002140F8(a, b);
            }
        }
        if (*(unsigned short *)(o + 0x70) != 0) {
            if (func_001F9938(o + 0x78)) {
                *(short *)(o + 0x78) = func_001F9850_D83D8(*(unsigned short *)(o + 0x70));
                *(int *)(o + 0x4C) &= 0xFFFF7FFF;
                *(float *)(o + 0x5C) = (float)o[0x31];
            }
        }
        if (!(*(int *)(o + 0x4C) & 0x40000)) {
            s64 q = *(s64 *)(o + 0x48);
            if ((q & (0xC000L << 30)) == (0x8000L << 30)) *(int *)(o + 0x4C) ^= 0x2000;
            if (!(*(int *)(o + 0x4C) & 0x2000)) {
                if (*(int *)(o + 0x4C) & 0x8000) {
                    float a = func_001FA888(o[0x31]) / 100.0f;
                    float b = func_001FA888(o[0x33]) / 100.0f;
                    float t = *(float *)(o + 0x5C) - b * D_0015EE60;
                    *(float *)(o + 0x5C) = t;
                    if (t <= a) {
                        *(float *)(o + 0x5C) = a;
                        if (*(int *)(o + 0x4C) & 0x4000) {
                            if (*(unsigned short *)(o + 0x70) == 0) *(int *)(o + 0x4C) ^= 0x8000;
                        }
                    }
                } else {
                    float a = func_001FA888(o[0x30]) / 100.0f;
                    float b = func_001FA888(o[0x32]) / 100.0f;
                    float t = *(float *)(o + 0x5C) + b * D_0015EE60;
                    *(float *)(o + 0x5C) = t;
                    if (a <= t) {
                        *(float *)(o + 0x5C) = a;
                        if (!(*(int *)(o + 0x4C) & 0x1000)) {
                            *(int *)(o + 0x4C) ^= 0x8000;
                        } else {
                            *(unsigned char *)(o + 0x7E) = func_001F9850_D83D8(o[0x6A]);
                            *(int *)(o + 0x4C) = (*(int *)(o + 0x4C) | 0x2000) ^ 0x8000;
                        }
                    }
                }
            } else {
                if (o[0x6A] != 0 && func_L00_001FEF78(o + 0x7E)) *(int *)(o + 0x4C) ^= 0x2000;
            }
        } else {
            if (*(unsigned short *)(o + 0x70) == 0) {
                float a = (float)func_001FA898_r((float)o[0x31]) / 100.0f;
                float b = (float)func_001FA898_r((float)o[0x30]) / 100.0f;
                *(float *)(o + 0x5C) = func_002140F8(a, b);
            }
        }
        r = *(int *)(o + 0x50);
        if (r == -1) {
            float kk = func_001FA888(o[0x24]) / 100.0f;
            *(int *)(o + 0x50) = func_L00_0023F0D0((float *)v21, *(float *)(o + 0x60), kk, *(float *)(o + 0x54), *(float *)(o + 0x58), *(float *)(o + 0x5C));
        } else {
            unsigned char *e = D_L00_001803C0 + (r << 5);
            qcopy(e + 0x10, v21);
            *(float *)(e + 0x1C) = *(float *)(o + 0x60);
            *(float *)(e + 0x0) = *(float *)(o + 0x54);
            *(float *)(e + 0x4) = *(float *)(o + 0x58);
            *(float *)(e + 0x8) = *(float *)(o + 0x5C);
            *(float *)(e + 0xC) = func_001FA888(o[0x24]) / 100.0f;
        }
    } else {
        if (*(int *)(o + 0x50) != -1) {
            func_L00_0023F1D0(*(int *)(o + 0x50));
            *(int *)(o + 0x50) = r;
        }
    }
}
