/* functional: high. Only registers, scheduling, the frame layout and the order of independent stores differ from retail; the reads, writes, calls, their arguments and order, and the float multiply order are retail's (the unset w lanes of the two state-3 vectors are never written, as in retail). */
/* Level 3 (Kerwan) Robot Qwark (moby class 914) update. A hero close enough
 * (< 26) while m[0x31] is set triggers func_L00_0025B178 and m[0x7F] = 21.
 * If its saved bit (level table or per-save bitfield) is set it goes to state
 * 3. A hit of type 3 while the bit is clear plays the first-time cutscene
 * flag/sound, throws debris (func_L00_0025F4A8), enters state 3 and sets both
 * bits. Then a state machine: 0 copies the hero's 0x38 word and starts the
 * talk, 1 waits for the talk and plays its animation, 2 handles the talk
 * camera, 3 emits a random puff (func_L00_0026DD70) around it. */
typedef unsigned int u128_DD158 __attribute__((mode(TI), aligned(16)));

extern float func_001F9D10_DD158(void *, void *) __asm__("func_001F9D10");
extern void func_L00_0025B178_DD158(unsigned char *) __asm__("func_L00_0025B178");
extern void func_00213D28_DD158(unsigned char *, int, int) __asm__("func_00213D28");
extern unsigned char *func_L00_0025B478_DD158(unsigned char *, int, int) __asm__("func_L00_0025B478");
extern void func_0022EE28_DD158(int, int, int) __asm__("func_0022EE28");
extern void func_L00_00264DB8_DD158(int, int) __asm__("func_L00_00264DB8");
extern void func_L00_0025F4A8_DD158(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern int func_L00_002676E8_DD158(unsigned char *, unsigned char *) __asm__("func_L00_002676E8");
extern int func_L00_00267290_DD158(unsigned char *, unsigned char *) __asm__("func_L00_00267290");
extern void func_L01_00279398_DD158(float, unsigned char *) __asm__("func_L01_00279398");
extern void func_L00_00286128_DD158(unsigned char *, unsigned char *) __asm__("func_L00_00286128");
extern float func_00214158_DD158(void) __asm__("func_00214158");
extern float func_001F9F90_DD158(float) __asm__("func_001F9F90");
extern float func_001F9FA8_DD158(float) __asm__("func_001F9FA8");
extern float func_002140F8_DD158(float, float) __asm__("func_002140F8");
extern void func_L00_001FF240_DD158(void *, void *, void *) __asm__("func_L00_001FF240");
extern int func_002140B0_DD158(int) __asm__("func_002140B0");
extern int func_L00_00258BC8_DD158(int, int) __asm__("func_L00_00258BC8");
extern int func_001F9850_DD158(int) __asm__("func_001F9850");
extern void func_L00_0026DD70_DD158(void *, void *, int, int, float, int) __asm__("func_L00_0026DD70");

extern unsigned char D_L03_00166F40_DD158[] __asm__("D_L03_00166F40");
extern unsigned char D_L03_001BB640_DD158[] __asm__("D_L03_001BB640");
extern int D_L03_001BA8E0_DD158[] __asm__("D_L03_001BA8E0");
extern unsigned char D_0014171B_DD158[] __asm__("D_0014171B");
extern int D_0015EE84_DD158 __asm__("D_0015EE84");
extern unsigned char D_0013D50F_DD158[] __asm__("D_0013D50F");
extern unsigned char D_0013D5CA_DD158[] __asm__("D_0013D5CA");
extern unsigned char D_0013D4A5_DD158[] __asm__("D_0013D4A5");
extern unsigned char D_0013E633_DD158[] __asm__("D_0013E633");
extern int D_L03_0015F6A8_DD158 __asm__("D_L03_0015F6A8");
extern unsigned char *D_L03_0016016C_DD158 __asm__("D_L03_0016016C");
extern float D_0015EE6C_DD158 __asm__("D_0015EE6C");

/* the per-save bitfield word holding bit h (0x14C290 + (h >> 5) * 4 + save * 256) */
static int *save_word_DD158(unsigned short h) {
    return (int *)(D_0014171B_DD158 + 0xAB75 + ((short)h >> 5) * 4 + (D_0015EE84_DD158 << 8));
}

void func_L03_002DD158(unsigned char *m) {
    unsigned char *d = *(unsigned char **)(m + 0x78);
    unsigned char *it;
    unsigned short h;
    unsigned char itype;

    if (m[0x31] != 0) {
        if (func_001F9D10_DD158(m + 0x10, D_L03_00166F40_DD158) < 26.0f) {
            func_L00_0025B178_DD158(m);
            m[0x7F] = 21;
        }
    }

    h = *(unsigned short *)(m + 0xB2);
    if (D_L03_001BB640_DD158[0x454 + (short)h] != 0 || ((*save_word_DD158(h) >> (h & 0x1F)) & 1)) {
        if (m[0x20] != 3) {
            m[0x20] = 3;
            func_00213D28_DD158(m, 1, 0);
        }
    }

    it = func_L00_0025B478_DD158(m, -1, 0);
    if (it != 0 && *(int *)(it + 0x20) != 0) {
        itype = it[0x28];
        if (itype == 3) {
            h = *(unsigned short *)(m + 0xB2);
            if (D_L03_001BB640_DD158[0x454 + (short)h] == 0 && (((*save_word_DD158(h) >> (h & 0x1F)) ^ 1) & 1)) {
                unsigned char *p5 = D_0013D50F_DD158 + 1; /* 0x13D510 */
                float vec[4] __attribute__((aligned(16)));
                float blk[4] __attribute__((aligned(16)));

                if (p5[5] == 0) {
                    p5[5] = 1;
                    func_0022EE28_DD158(1, 0, 0);
                    func_L00_00264DB8_DD158(0x53DB, -1);
                }
                *(u128_DD158 *)blk = *(u128_DD158 *)(m + 0x10);
                *(u128_DD158 *)vec = 0;
                blk[2] = blk[2] + 1.0f;
                func_L00_0025F4A8_DD158(m, vec, blk, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 0, 15.0f, 1, 1, -1,
                                        0);
                m[0x20] = itype;
                func_00213D28_DD158(m, 1, 0);
                h = *(unsigned short *)(m + 0xB2);
                {
                    int *w = save_word_DD158(h);
                    *w = *w | (1 << (h & 0x1F));
                }
                h = *(unsigned short *)(m + 0xB2);
                D_L03_001BA8E0_DD158[(short)h >> 5] = D_L03_001BA8E0_DD158[(short)h >> 5] | (1 << (h & 0x1F));
            }
        }
    }
    m[0xA4] = 0xFF;

    if (D_0013D5CA_DD158[0xA] != 0 && m[0x20] != 3) {
        return;
    }

    switch (m[0x20]) {
    case 0: {
        unsigned char *p = *(unsigned char **)(D_0013E633_DD158 + 0x2E9D); /* 0x1414D0 */
        long long t = *(long long *)(p + 0x38);
        m[0x20] = 1;
        *(long long *)(m + 0x38) = t;
        func_L00_002676E8_DD158(m, d);
        if (D_0013D4A5_DD158[4] != 0) {
            *(short *)(d + 4) = 1;
            *(short *)(d + 0x36) = 2;
        }
        break;
    }
    case 1:
        if (func_L00_00267290_DD158(m, d) != 0) {
            D_0013D4A5_DD158[4] = 1;
            func_L01_00279398_DD158(2.7f, m);
            m[0x20] = 2;
        }
        break;
    case 2: {
        int v;
        unsigned char *p;
        if (D_L03_0015F6A8_DD158 == 2) {
            break;
        }
        m[0x20] = 1;
        if (*(short *)(d + 4) != 2) {
            break;
        }
        v = *(int *)(d + 0x4C);
        if (v == -1) {
            break;
        }
        p = D_L03_0016016C_DD158 + v * 128;
        func_L00_00286128_DD158(p + 0x30, p + 0x70);
        break;
    }
    case 3: {
        float a[4] __attribute__((aligned(16)));
        float b[4] __attribute__((aligned(16)));
        float c[4] __attribute__((aligned(16)));
        float f20, r1, r2, t;

        t = func_00214158_DD158();
        f20 = func_001F9F90_DD158(t);
        r1 = func_002140F8_DD158(0.0f, 0.25f);
        a[0] = f20 * (r1 * D_0015EE6C_DD158);

        t = func_00214158_DD158();
        f20 = func_001F9FA8_DD158(t);
        r1 = func_002140F8_DD158(0.0f, 0.25f);
        a[2] = 0.0f;
        a[1] = f20 * (r1 * D_0015EE6C_DD158);

        t = func_00214158_DD158();
        f20 = func_001F9F90_DD158(t);
        r1 = func_002140F8_DD158(0.0f, 0.5f);
        r2 = func_002140F8_DD158(0.0f, 1.0f);
        b[0] = (f20 * r1) * r2;

        t = func_00214158_DD158();
        f20 = func_001F9FA8_DD158(t);
        r1 = func_002140F8_DD158(0.0f, 0.5f);
        r2 = func_002140F8_DD158(0.0f, 1.0f);
        b[2] = 0.0f;
        b[1] = (f20 * r1) * r2;

        func_L00_001FF240_DD158(c, b, m + 0x10);
        if (func_002140B0_DD158(3) == 0) {
            int r = func_001F9850_DD158(func_L00_00258BC8_DD158(0x3C, 0x78));
            func_L00_0026DD70_DD158(b, a, 0x20202020, 0x7F7F7F, 157500.0f, r);
        }
        break;
    }
    }
}
