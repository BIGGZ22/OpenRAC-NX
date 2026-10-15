/*
 * GS packet builder for a display or draw environment (written from the assembly).
 * Writes five 64-bit words at out: out[0] = 0x66 (the GIF tag), out[1] = a mode
 * word 1..3, out[2] = a 0x3F-based field pair, out[3] = the sized rectangle
 * (by the mode and the values of the helper calls), out[4] = 0.
 * The helpers: func_00121D08 gives a record P (2 u16 words at +0 and +2 and
 * 16-bit words at +0 and +4), func_00121D18 and func_00121DB8 fill four
 * int locals when P+2 is not 2 or 3. Divisor y is never 0 on this path.
 */
extern char D_00153140[];
extern void *func_00121D08_x(void *) __asm__("func_00121D08");
extern int func_00121D18_x(int) __asm__("func_00121D18");
extern void func_00121DB8_x(int, int *, int *, int *, int *) __asm__("func_00121DB8");
extern void func_0011A6C8_x(void *) __asm__("func_0011A6C8");

void func_00121DC8(char *out, int a1, int a2, int a3, int a4, int a5) {
    int x;
    int y;
    int w;
    int h;
    int o;
    int l[4];
    char *P;
    int f2;
    int f0s;
    int f4s;
    int T;
    int T2;
    int q;
    int cc;
    int t;
    int d;
    int p1;
    int p2;
    int p4;
    int c5;
    int t2;
    unsigned long m3;
    unsigned long m10;
    unsigned long e7;
    unsigned long big;

    x = (short)a1;
    y = (short)a2;
    w = (short)a3;
    h = (short)a4;
    o = (short)a5;

    P = (char *)func_00121D08_x(out);
    f2 = *(unsigned short *)(P + 2);
    if ((unsigned int)(f2 - 2) < 2u) {
        l[3] = 0;
        l[2] = 0;
        l[1] = 0;
        l[0] = 0;
    } else {
        if (func_00121D18_x(f2) != 0) {
            func_00121DB8_x((short)f2, &l[0], &l[1], &l[2], &l[3]);
        } else {
            l[3] = 0;
            l[2] = 0;
            l[1] = 0;
            l[0] = 0;
        }
    }

    *(unsigned long *)(out + 0x00) = 0x66;
    f0s = *(short *)(P + 0);
    if (f0s == 0) {
        T = 2;
    } else {
        f4s = *(short *)(P + 4);
        T = (f4s != 0) ? 3 : 1;
    }
    *(unsigned long *)(out + 0x08) = (unsigned long)(long)T;

    T2 = (short)f2;
    *(unsigned long *)(out + 0x10) =
        ((unsigned long)(long)(x & 0xF) << 15) |
        ((unsigned long)(long)((((y + 0x3F) >> 6) & 0x3F)) << 9);

    if (T2 == 2 || T2 == 3) {
        /* Shared shape of A (T2 == 2) and B3 (T2 == 3); the constants differ. */
        int k1 = (T2 == 2) ? 0x32 : 0x48;
        int k2 = (T2 == 2) ? 0x27C : 0x290;
        int k3 = (T2 == 2) ? 0x19 : 0x24;
        if (f0s == 1) {
            q = (y + 0x9FF) / y;
            t = o + l[1] + k1;
            cc = l[0] + k2;
            d = t & 0xFFF;
            e7 = (unsigned long)(long)d << 12;
            p1 = q * y;
            p2 = h * q;
            m10 = (unsigned long)(long)(q - 1) << 23;
            p1 = p1 - 1;
            m3 = (unsigned long)(long)p1 << 32;
            c5 = (int)(((long)p2 + (long)cc) & 0xFFF);
            if (*(short *)(P + 4) == 0) {
                t2 = w - 1;
            } else {
                t2 = (w << 1) - 1;
            }
            m3 = m3 | m10;
            big = (unsigned long)(long)t2 << 44;
            big = (unsigned long)(long)c5 | big;
            m3 = m3 | big;
            *(unsigned long *)(out + 0x18) = m3 | e7;
        } else {
            q = f0s / y;
            t2 = w - 1;
            t = o + l[1] + k3;
            d = t & 0xFFF;
            p1 = h * q;
            p4 = q * y;
            cc = l[0] + k2;
            m10 = (unsigned long)(long)(q - 1) << 23;
            c5 = (int)(((long)p1 + (long)cc) & 0xFFF);
            p4 = p4 - 1;
            big = (unsigned long)(long)t2 << 44;
            big = (unsigned long)(long)c5 | big;
            m3 = (unsigned long)(long)p4 << 32;
            m3 = m3 | m10;
            m3 = m3 | big;
            m3 = m3 | ((unsigned long)(long)d << 12);
            *(unsigned long *)(out + 0x18) = m3;
        }
    } else if (T2 == 0x50) {
        t = 0x2D0 - y;
        t = t + (int)((unsigned int)t >> 31);
        t = (t >> 1) << 1;
        c5 = t + l[0];
        cc = (h << 1) + 0xE8;
        c5 = c5 + cc;
        m3 = (unsigned long)(long)(w - 1) << 44;
        m10 = (unsigned long)(long)((y << 1) - 1) << 32;
        c5 = c5 & 0xFFF;
        t2 = o + l[1] + 0x23;
        c5 = c5 | 0x800000;
        m3 = m3 | m10;
        t2 = t2 & 0xFFF;
        m3 = m3 | (unsigned long)(long)c5;
        m3 = m3 | ((unsigned long)(long)t2 << 12);
        *(unsigned long *)(out + 0x18) = m3;
    } else {
        func_0011A6C8_x(D_00153140);
    }

    *(unsigned long *)(out + 0x20) = 0;
}
