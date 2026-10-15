/* functional: high. Same reads, writes, calls and arguments as retail; the jump table is a switch, the shared exits are gotos, quad copies are four-float copies, the sin/cos/atan helpers keep their order, and the one float division (2*pi / random tick) is guarded so that a zero divisor gives FLT_MAX as on the PS2. No COP2/VU0 instructions in this function. */
/* Level 12 (Hoven) moby update, one call per frame per moby (a second enemy type of the same family as
 * func_L12_002E8640). data = moby+0x78 is its state block, data+0xAC the linked shadow/effect moby, data+0xC0 a
 * "scripted/spawned" flag. Nothing happens while D_L12_0015F6A8 == 2. It runs func_L00_00260D30 (which fills the
 * target position at buf+0 and a vector at buf+0x20), ticks the effect moby's timer, copies the definition's float at +0x24
 * into moby+0x2C (halved when D_0013E633+0x2EC1 is 2), claims or frees its slot (func_0022ED80 / func_L00_0028EBF0)
 * from the moby state, and deletes itself (func_0020D678) when its save byte is 0xFF or when it is a spawned moby
 * that no longer counts. Then a state machine on moby+0x20: 0 setup (copy a waypoint row's first point, random
 * delay), 1 wait for the trigger, 2 charge up and pick a hover target (func_L12_002EC2E8), 3 chase the target with
 * the angle springs and puff particles, 4 follow the waypoint row, 5 slow to a stop and fall, 6 die (explosion call
 * func_L00_0025F4A8, free the slot, delete or respawn). The tail (every state) bobs z by sin(data+0x90), copies pose
 * to the effect moby and, for a non-spawned moby in state >= 2 within 30 units of D_L12_001672C0, calls
 * func_L00_0025B178 and sets moby+0x7F = 0x18. */
extern int D_L12_0015F6A8_2EB2E8 __asm__("D_L12_0015F6A8") MACRO_ADDR;
extern int D_L12_00161F80_2EB2E8 __asm__("D_L12_00161F80") MACRO_ADDR;
extern int D_L12_00161FA0_2EB2E8 __asm__("D_L12_00161FA0") MACRO_ADDR;
extern int D_L12_00161A20_2EB2E8 __asm__("D_L12_00161A20") MACRO_ADDR;
extern char D_L12_00167180_2EB2E8[] __asm__("D_L12_00167180");
extern char D_L12_001672C0_2EB2E8[] __asm__("D_L12_001672C0");
extern float D_L12_0015F660_2EB2E8[] __asm__("D_L12_0015F660") MACRO_ADDR;
extern char *D_L12_001B0C30_2EB2E8[] __asm__("D_L12_001B0C30");
extern unsigned char D_0013E633_2EB2E8[] __asm__("D_0013E633");
extern unsigned char D_0013D5CA_2EB2E8[] __asm__("D_0013D5CA");
extern unsigned char D_0014171B_2EB2E8[] __asm__("D_0014171B");
extern int D_0015EE84_2EB2E8 __asm__("D_0015EE84") MACRO_ADDR;
extern float D_0015EE6C_2EB2E8 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_2EB2E8 __asm__("D_0015EE70") MACRO_ADDR;
extern int func_L00_00260D30_2EB2E8(void *, void *, float) __asm__("func_L00_00260D30");
extern void func_L00_00264B40_2EB2E8(float, void *, int, void *) __asm__("func_L00_00264B40");
extern void func_L12_002EC5A0_2EB2E8(void *) __asm__("func_L12_002EC5A0");
extern int func_0022ED80_2EB2E8(int, int, void *) __asm__("func_0022ED80");
extern void func_L00_0028EBF0_2EB2E8(int) __asm__("func_L00_0028EBF0");
extern void func_0020D678_2EB2E8(void *) __asm__("func_0020D678");
extern void func_L12_00309AE8_2EB2E8(void *, void *) __asm__("func_L12_00309AE8");
extern char *func_L12_00309A50_2EB2E8(void *) __asm__("func_L12_00309A50");
extern void func_L12_0027CA68_2EB2E8(int) __asm__("func_L12_0027CA68");
extern int func_001F9850_2EB2E8(int) __asm__("func_001F9850");
extern int func_L00_00258BC8_2EB2E8(int, int) __asm__("func_L00_00258BC8");
extern int func_001F9908_2EB2E8(void *) __asm__("func_001F9908");
extern int func_00215570_2EB2E8(void *, int) __asm__("func_00215570");
extern int func_L12_002EC2E8_2EB2E8(void *, int) __asm__("func_L12_002EC2E8");
extern void func_L00_00203F20_2EB2E8(int, int) __asm__("func_L00_00203F20");
extern float func_001F9D48_2EB2E8(void *, void *) __asm__("func_001F9D48");
extern float func_001F9D10_2EB2E8(void *, void *) __asm__("func_001F9D10");
extern float func_L00_001FF860_2EB2E8(float, float) __asm__("func_L00_001FF860");
extern float func_L00_00259148_2EB2E8(float *, float, float, float, float, float) __asm__("func_L00_00259148");
extern void func_001F9BC0_2EB2E8(void *) __asm__("func_001F9BC0");
extern void func_L12_002EC180_2EB2E8(void *) __asm__("func_L12_002EC180");
extern int func_001F9938_2EB2E8(void *) __asm__("func_001F9938");
extern void func_L00_00250800_2EB2E8(void *, int, void *) __asm__("func_L00_00250800");
extern float func_002140F8_2EB2E8(float, float) __asm__("func_002140F8");
extern float func_L00_00258C80_2EB2E8(float, float) __asm__("func_L00_00258C80");
extern void func_001F9BF0_2EB2E8(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_001F9BD8_2EB2E8(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FF4B0_2EB2E8(void *, void *, float) __asm__("func_L00_001FF4B0");
extern char *func_L12_002EE068_2EB2E8(void *, void *, void *, int, float, float, float) __asm__("func_L12_002EE068");
extern void func_L00_0026A7F8_2EB2E8(void *, void *, int, int, int, int, int, int) __asm__("func_L00_0026A7F8");
extern float func_00214D28_2EB2E8(float *, float, float) __asm__("func_00214D28");
extern float func_001F9F90_2EB2E8(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2EB2E8(float) __asm__("func_001F9FA8");
extern float func_001FA748_2EB2E8(float, float) __asm__("func_001FA748");
extern void func_L00_0025F4A8_2EB2E8(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L00_002584A8_2EB2E8(void *, int, int) __asm__("func_L00_002584A8");
extern void func_L00_002514B8_2EB2E8(void *) __asm__("func_L00_002514B8");
extern void func_L00_00251E30_2EB2E8(void *) __asm__("func_L00_00251E30");
extern void func_L00_0025B178_2EB2E8(void *) __asm__("func_L00_0025B178");
extern float func_001F9CB8_2EB2E8(void *) __asm__("func_001F9CB8");

#define M_U8(p, o) (*(unsigned char *)((char *)(p) + (o)))
#define M_S8(p, o) (*(signed char *)((char *)(p) + (o)))
#define M_U16(p, o) (*(unsigned short *)((char *)(p) + (o)))
#define M_S16(p, o) (*(short *)((char *)(p) + (o)))
#define M_I(p, o) (*(int *)((char *)(p) + (o)))
#define M_F(p, o) (*(float *)((char *)(p) + (o)))
#define M_P(p, o) (*(char **)((char *)(p) + (o)))

typedef int u128_2EB2E8 __attribute__((mode(TI)));

#define COPY4(dst, src) (*(u128_2EB2E8 *)(dst) = *(u128_2EB2E8 *)(src))

void func_L12_002EB2E8(char *moby) {
    char *data;
    char *obj;
    char *row;
    char *tgt;
    char *X;
    float buf[20] __attribute__((aligned(16)));
    float v70[4] __attribute__((aligned(16)));
    float v80[4] __attribute__((aligned(16)));
    float v90[4] __attribute__((aligned(16)));
    float vA0[4] __attribute__((aligned(16)));
    float f;
    float g;
    int st;
    int flagC0;
    int lvl;
    int r;
    int n;
    int t;
    int s;
    int h;
    int limit;
    int col;
    int i;
    int tmp;

    if (D_L12_0015F6A8_2EB2E8 == 2) {
        return;
    }
    data = M_P(moby, 0x78);
    func_L00_00260D30_2EB2E8(moby, buf, 70.0f);
    obj = M_P(data, 0xAC);
    if (obj != 0 && M_S8(obj, 0x20) >= 0) {
        func_L00_00264B40_2EB2E8(2.7f, obj, 0, data + 0xD0);
    }
    func_L12_002EC5A0_2EB2E8(moby);

    if (D_0013E633_2EB2E8[0x2EC1] == 2) {
        M_F(moby, 0x2C) = M_F(M_P(moby, 0x24), 0x24) * 0.5f;
        obj = M_P(data, 0xAC);
        if (obj != 0) {
            M_F(obj, 0x2C) = M_F(M_P(obj, 0x24), 0x24) * 0.5f;
        }
    } else {
        M_F(moby, 0x2C) = M_F(M_P(moby, 0x24), 0x24);
        obj = M_P(data, 0xAC);
        if (obj != 0) {
            M_F(obj, 0x2C) = M_F(M_P(obj, 0x24), 0x24);
        }
    }

    st = M_U8(moby, 0x20);
    if ((unsigned)(st - 2) < 4) {
        if (M_I(data, 0xB0) == -1) {
            M_I(data, 0xB0) = func_0022ED80_2EB2E8(3, 4, moby);
        }
    } else {
        r = M_I(data, 0xB0);
        if (r != -1) {
            char *ent = D_0013E633_2EB2E8 + 0x1D + r * 0x70;
            if (M_P(ent, 0x88) == moby && M_U8(ent, 0x74) != 0) {
                func_L00_0028EBF0_2EB2E8(r);
            }
        }
        M_I(data, 0xB0) = -1;
    }

    lvl = D_0015EE84_2EB2E8;
    flagC0 = M_I(data, 0xC0);
    if (lvl == 12 && D_L12_00161F80_2EB2E8 != 0) {
        if (flagC0 == 0) {
            X = D_L12_00167180_2EB2E8;
            if (M_P(X, 0x25C) == moby) {
                M_I(X, 0x25C) = 0;
            }
            if (M_U8(moby, 0x31) == 0) {
                obj = M_P(data, 0xAC);
                if (obj != 0) {
                    func_0020D678_2EB2E8(obj);
                    M_I(data, 0xAC) = 0;
                }
                goto kill;
            }
            flagC0 = M_I(data, 0xC0);
        }
    }
    if (flagC0 != 0) {
        if (D_0014171B_2EB2E8[0xAA35 + (lvl << 4) + M_U8(moby, 0xB0)] == 0xFF) {
            obj = M_P(data, 0xAC);
            if (obj != 0) {
                func_0020D678_2EB2E8(obj);
                M_I(data, 0xAC) = 0;
            }
            goto kill;
        }
        if (D_L12_00161A20_2EB2E8 != 0) {
            obj = M_P(data, 0xAC);
            if (obj != 0) {
                func_L12_00309AE8_2EB2E8(moby, obj);
                M_I(data, 0xAC) = 0;
            }
            goto kill;
        }
    }

    st = M_U8(moby, 0x20);
    if (st >= 7) {
        goto tail;
    }
    switch (st) {
    case 0:
        M_U8(data, 0x28) = 2;
        M_F(data, 0x20) = 6.0f;
        M_S16(data, 0x24) = 6;
        M_U8(data, 0x29) = 0;
        M_I(data, 0x94) = 0;
        M_I(data, 0x98) = 0;
        M_I(data, 0x9C) = 0;
        M_I(moby, 0x94) = 0;
        M_U16(moby, 0x34) = (M_U16(moby, 0x34) | 0x41) & 0xEFFF;
        func_L12_0027CA68_2EB2E8(M_I(data, 0xA0));
        func_L12_0027CA68_2EB2E8(M_I(data, 0xA0));
        func_L12_0027CA68_2EB2E8(M_I(data, 0xA4));
        if (M_S16(moby, 0x32) < 0x80) {
            M_S16(moby, 0x32) = 0x80;
        }
        if (M_P(data, 0xAC) == 0) {
            obj = func_L12_00309A50_2EB2E8(moby);
            M_P(data, 0xAC) = obj;
            M_U8(obj, 0x30) = 0;
            obj = M_P(data, 0xAC);
            M_U16(obj, 0x34) = M_U16(obj, 0x34) | 0x41;
        }
        row = D_L12_001B0C30_2EB2E8[M_I(data, 0xA0)];
        COPY4(moby + 0x10, row + 0x10);
        if (M_I(data, 0xC0) == 0 || M_I(data, 0xC4) != 0) {
            M_I(data, 0xB4) = 0;
        } else {
            t = func_001F9850_2EB2E8(0xF0);
            s = func_001F9850_2EB2E8(0x190);
            M_I(data, 0xC4) = func_L00_00258BC8_2EB2E8(t, s);
            n = D_L12_00161FA0_2EB2E8 - 1;
            if (n >= 11) {
                n = 10;
            }
            if (n <= -1) {
                n = 0;
            }
            t = func_001F9850_2EB2E8(0x23);
            M_I(data, 0xC4) = M_I(data, 0xC4) + n * t;
            M_I(data, 0xB4) = 0;
        }
        M_U8(moby, 0x20) = 1;
        goto tail;

    case 1:
        r = 0;
        if (M_I(data, 0xC0) != 0) {
            if (D_L12_00161F80_2EB2E8 != 0) {
                r = (func_001F9908_2EB2E8(data + 0xC4) != 0);
            }
        }
        n = M_I(data, 0x80);
        if (n == -1 || func_00215570_2EB2E8(D_0013E633_2EB2E8 + 0xE9D, n) == 0) {
            if (r == 0) {
                goto tail;
            }
        }
        M_U8(data, 0x58) = 0x16;
        M_U8(data, 0x59) = 1;
        M_U8(data, 0x5A) = 7;
        M_U8(moby, 0xA4) = 0xFF;
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        obj = M_P(data, 0xAC);
        if (obj != 0) {
            M_U16(obj, 0x34) = M_U16(obj, 0x34) & 0xFFBE;
        }
        M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x1000;
        M_I(moby, 0x94) = M_I(M_P(moby, 0x24), 0x10);
        M_U8(moby, 0x20) = 2;
        goto tail;

    case 2:
        if (M_I(data, 0xC0) == 0) {
            X = D_L12_00167180_2EB2E8;
            if (M_P(X, 0x25C) == 0) {
                M_I(X, 0x260) = 0;
                M_P(X, 0x25C) = moby;
            }
        }
        if (M_I(data, 0xC0) != 0) {
            n = D_L12_00161FA0_2EB2E8 - 3;
            if (n >= 6) {
                n = 5;
            }
            if (n <= -1) {
                n = 0;
            }
            h = M_S16(data, 0x8C);
            t = func_001F9850_2EB2E8(0xF);
            if (h < n * t) {
                t = func_001F9850_2EB2E8(0xF);
                M_S16(data, 0x8C) = (short)(n * t);
            }
        }
        if (func_L12_002EC2E8_2EB2E8(moby, M_I(data, 0xA0)) != 0) {
            M_U8(moby, 0x20) = 3;
        }
        goto tail;

    case 3:
        if (D_0015EE84_2EB2E8 == 12 && D_0013D5CA_2EB2E8[9] == 0 && *(unsigned short *)(D_0014171B_2EB2E8 + 0x635) == 0) {
            func_L00_00203F20_2EB2E8(0x2EE2, 0x5D);
        }
        tgt = D_0013E633_2EB2E8 + 0xE9D;
        f = func_001F9D48_2EB2E8(moby + 0x10, tgt);
        if (M_I(data, 0xC0) == 0) {
            X = D_L12_00167180_2EB2E8;
            if (f < 21.0f && M_F(moby, 0x18) - M_F(tgt, 8) > 1.0f) {
                if (M_P(X, 0x25C) == 0) {
                    M_I(X, 0x260) = 0;
                    M_P(X, 0x25C) = moby;
                }
            } else {
                if (M_P(X, 0x25C) == moby) {
                    M_I(X, 0x25C) = 0;
                }
            }
        }
        f = func_001F9D48_2EB2E8(moby + 0x10, buf);
        if ((f < 40.0f && M_F(moby, 0x18) - buf[2] > -2.0f) || M_I(data, 0xC0) != 0) {
            f = func_L00_001FF860_2EB2E8(buf[0] - M_F(moby, 0x10), buf[1] - M_F(moby, 0x14));
            M_F(moby, 0x48) = func_L00_00259148_2EB2E8((float *)(data + 0x88), M_F(moby, 0x48), f, 0.004f, 0.3f, D_0015EE6C_2EB2E8 * 0.78539819f);
            g = func_001F9D48_2EB2E8(buf, moby + 0x10);
            f = func_L00_001FF860_2EB2E8(g, M_F(moby, 0x18) - buf[2]);
            M_F(moby, 0x44) = func_L00_00259148_2EB2E8((float *)(data + 0x9C), M_F(moby, 0x44), f, 0.004f, 0.3f, D_0015EE6C_2EB2E8 * 0.78539819f);
            func_001F9BC0_2EB2E8(data + 0x70);
            func_L12_002EC180_2EB2E8(moby);
            if (func_001F9938_2EB2E8(data + 0x8C) != 0) {
                M_U16(data, 0xBC) = M_U16(data, 0xBC) + 1;
                func_L00_00250800_2EB2E8(moby, M_I(data, 0xB8), v70);
                M_I(data, 0xB8) = (M_I(data, 0xB8) == 0);
                COPY4(v90, buf + 8);
                if (M_I(data, 0xC0) != 0) {
                    COPY4(v90, D_L12_001672C0_2EB2E8);
                    f = func_002140F8_2EB2E8(-0.5f, 0.1f);
                    v90[2] = v90[2] + f;
                    f = func_L00_00258C80_2EB2E8(0.0f, 0.4f);
                    v90[0] = v90[0] + f;
                }
                func_001F9BF0_2EB2E8(v80, v90, v70);
                if (M_I(data, 0xC0) != 0) {
                    func_L00_001FF4B0_2EB2E8(v80, v80, D_0015EE6C_2EB2E8 * 22.0f);
                } else {
                    func_L00_001FF4B0_2EB2E8(v80, v80, D_0015EE6C_2EB2E8 * 15.0f);
                }
                f = func_L00_001FF860_2EB2E8(v80[0], v80[1]);
                t = func_001F9850_2EB2E8(0x82);
                func_L12_002EE068_2EB2E8(v80, v70, moby, t, f, D_0015EE6C_2EB2E8 * 15.0f, 0.0f);
                func_0022ED80_2EB2E8(0, 0, moby);
                func_L00_001FF4B0_2EB2E8(v80, v80, 0.5f);
                func_001F9BD8_2EB2E8(v70, v70, v80);
                for (i = 24; i >= 0; i--) {
                    vA0[0] = func_L00_00258C80_2EB2E8(0.0f, D_0015EE6C_2EB2E8 * 1.3f);
                    vA0[1] = func_L00_00258C80_2EB2E8(0.0f, D_0015EE6C_2EB2E8 * 1.3f);
                    vA0[2] = func_L00_00258C80_2EB2E8(0.0f, D_0015EE6C_2EB2E8 * 1.3f);
                    s = func_L00_00258BC8_2EB2E8(0xAA, 0xF0);
                    t = func_L00_00258BC8_2EB2E8(0x6E, 0xB4);
                    col = (t << 8) | (s | 0x7F000000);
                    n = func_L00_00258BC8_2EB2E8(0x1E, 0x46);
                    n = func_001F9850_2EB2E8(n);
                    t = func_L00_00258BC8_2EB2E8(0x78, 0xAA);
                    func_L00_0026A7F8_2EB2E8(v70, vA0, col, 0xFF, n, 0x1E, t, 1);
                }
                limit = 4;
                if (M_I(data, 0xC0) != 0) {
                    lvl = D_L12_00161FA0_2EB2E8;
                    if (lvl >= 11) {
                        limit = 2;
                    }
                    if (lvl >= 7) {
                        limit = 3;
                    }
                }
                if (M_S16(data, 0xBC) >= limit && M_I(data, 0xA4) != -1) {
                    M_S16(data, 0xBC) = 0;
                    M_U8(moby, 0x20) = 4;
                    goto tail;
                }
                M_S16(data, 0x8C) = (short)func_001F9850_2EB2E8(0x28);
            }
        }
        goto tail;

    case 4:
        f = func_L00_001FF860_2EB2E8(buf[0] - M_F(moby, 0x10), buf[1] - M_F(moby, 0x14));
        M_F(moby, 0x48) = func_L00_00259148_2EB2E8((float *)(data + 0x88), M_F(moby, 0x48), f, 0.004f, 0.3f, D_0015EE6C_2EB2E8 * 0.78539819f);
        row = D_L12_001B0C30_2EB2E8[M_I(data, 0xA4)];
        if (M_S16(data, 0xBE) != 0) {
            func_001F9BF0_2EB2E8(v70, row + 0x10, moby + 0x10);
        } else {
            func_001F9BF0_2EB2E8(v70, row + (M_I(row, 0) << 4), moby + 0x10);
        }
        f = func_001F9CB8_2EB2E8(v70);
        if (5.5f < f) {
            func_00214D28_2EB2E8((float *)(data + 0x94), D_0015EE6C_2EB2E8 * 10.0f, D_0015EE70_2EB2E8 * 7.0f);
        } else {
            func_00214D28_2EB2E8((float *)(data + 0x94), 0.0f, D_0015EE70_2EB2E8 * 9.0f);
            if (M_F(data, 0x94) == 0.0f) {
                M_S16(data, 0x8C) = (short)func_001F9850_2EB2E8(0x32);
                M_S16(data, 0xBE) = (M_S16(data, 0xBE) == 0);
                M_U8(moby, 0x20) = 3;
                goto tail;
            }
        }
        func_L00_001FF4B0_2EB2E8(data + 0x70, v70, M_F(data, 0x94));
        func_001F9BD8_2EB2E8(moby + 0x10, moby + 0x10, data + 0x70);
        func_L12_002EC180_2EB2E8(moby);
        goto tail;

    case 5:
        if (func_001F9938_2EB2E8(data + 0x8E) != 0) {
            M_U8(moby, 0x20) = M_U8(moby, 0xBC);
            goto tail;
        }
        f = M_F(data, 0x94) - D_0015EE70_2EB2E8 * 4.0f;
        M_F(data, 0x94) = f;
        if (f < 0.0f) {
            M_F(data, 0x94) = 0.0f;
        }
        v70[0] = func_001F9F90_2EB2E8(M_F(moby, 0x48)) * -M_F(data, 0x94);
        f = func_001F9FA8_2EB2E8(M_F(moby, 0x48));
        v70[2] = 0.0f;
        v70[1] = f * -M_F(data, 0x94);
        func_001F9BD8_2EB2E8(moby + 0x10, moby + 0x10, v70);
        goto tail;

    case 6:
        X = D_L12_00167180_2EB2E8;
        if (M_P(X, 0x25C) == moby) {
            M_I(X, 0x25C) = 0;
        }
        obj = M_P(data, 0xAC);
        if (obj != 0) {
            func_L12_00309AE8_2EB2E8(moby, obj);
            M_I(data, 0xAC) = 0;
        }
        func_L00_0025F4A8_2EB2E8(moby, D_L12_0015F660_2EB2E8, moby + 0x10, 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 2, 3.0f, 15.0f, 1, 1, -1, 0);
        if (M_I(data, 0xC0) == 0) {
            func_L00_002584A8_2EB2E8(moby, 0, -1);
        }
        r = M_I(data, 0xB0);
        if (r != -1) {
            char *ent = D_0013E633_2EB2E8 + 0x1D + r * 0x70;
            if (M_P(ent, 0x88) == moby && M_U8(ent, 0x74) != 0) {
                func_L00_0028EBF0_2EB2E8(r);
            }
        }
        M_I(data, 0xB0) = -1;
        if (M_I(data, 0xC0) == 0) {
            goto kill;
        }
        t = func_001F9850_2EB2E8(0xF0);
        s = func_001F9850_2EB2E8(0x190);
        M_I(data, 0xC4) = func_L00_00258BC8_2EB2E8(t, s);
        M_I(moby, 0x94) = 0;
        M_U8(moby, 0x20) = 0;
        M_U16(moby, 0x34) = (M_U16(moby, 0x34) | 0x41) & 0xEFFF;
        goto tail;
    }

tail:
    f = func_001F9F90_2EB2E8(M_F(data, 0x90));
    M_F(moby, 0x18) = M_F(moby, 0x18) - f * 0.17f;
    t = func_001F9850_2EB2E8(0x78);
    /* retail divides without a check; a zero tick count is x/0 = FLT_MAX on the PS2 */
    if (t != 0) {
        f = 6.2831855f / (float)t;
    } else {
        tmp = 0x7F7FFFFF; /* FLT_MAX bits (a literal would go to rodata) */
        f = *(float *)&tmp;
    }
    M_F(data, 0x90) = func_001FA748_2EB2E8(M_F(data, 0x90), f);
    f = func_001F9F90_2EB2E8(M_F(data, 0x90));
    M_F(moby, 0x18) = M_F(moby, 0x18) + f * 0.17f;
    obj = M_P(data, 0xAC);
    if (obj != 0) {
        COPY4(obj + 0x10, moby + 0x10);
        obj = M_P(data, 0xAC);
        COPY4(obj + 0x40, moby + 0x40);
        func_L00_002514B8_2EB2E8(M_P(data, 0xAC));
        func_L00_00251E30_2EB2E8(M_P(data, 0xAC));
        obj = M_P(data, 0xAC);
        M_U16(obj, 0x34) = M_U16(obj, 0x34) | 0x6;
    }
    if (M_I(data, 0xC0) != 0) {
        return;
    }
    if (M_U8(moby, 0x20) < 2) {
        return;
    }
    if (M_U8(moby, 0x31) == 0) {
        return;
    }
    f = func_001F9D10_2EB2E8(moby + 0x10, D_L12_001672C0_2EB2E8);
    if (f < 30.0f) {
        func_L00_0025B178_2EB2E8(moby);
        M_U8(moby, 0x7F) = 0x18;
    }
    return;

kill:
    func_0020D678_2EB2E8(moby);
}

#undef M_U8
#undef M_S8
#undef M_U16
#undef M_S16
#undef M_I
#undef M_F
#undef M_P
#undef COPY4
