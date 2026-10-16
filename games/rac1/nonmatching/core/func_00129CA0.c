extern long func_0011EEC8(long a, long b);

/* Transcribed from the assembly, one operation at a time. func_0011EEC8 is a
   64-bit multiply (the low 64 bits of a*b). s is the object (flag at +0x70,
   values at +0x78..+0xF8), t the source record (+0x18 .. +0x40), o1 and o3 the
   two 64-bit outputs, o4 the final output; returns the packed value. */
long func_00129CA0(char *s, char *t, long *o1, long *o3, long *o4) {
    long v2;
    long w23;
    long v16;
    long a5;
    long r1;
    long r2;
    long r3;
    long r4;
    long w22;
    long x;
    long w3;
    long x2;
    long x3;
    long x4;
    long x6;
    long x7;

    if (*(int *)(s + 0x70) == 0) {
        *o1 = *(long *)(t + 0x18);
    } else {
        v2 = *(long *)(t + 0x18);
        w23 = 0;
        if (v2 >= 0 || (w23 = (long)*(int *)(s + 0x80)) < 0) {
            *o1 = v2;
        } else {
            v16 = (long)*(int *)(s + 0x88);
            a5 = *(long *)(s + 0x78) & 1;
            r1 = func_0011EEC8(v16 & 1, a5);
            w22 = (long)*(int *)(s + 0x90);
            r2 = func_0011EEC8(r1, w22 & 1);
            r3 = func_0011EEC8(*(long *)(s + 0x78), v16);
            x = (long)((unsigned long)r3 << 31) >> 32;
            x = (long)(int)((unsigned int)x + (unsigned int)r2);
            x = (long)(int)((unsigned int)w23 + (unsigned int)x);
            *o1 = x;
            a5 = *(long *)(s + 0x78) & 1;
            r4 = func_0011EEC8(v16 & 1, a5);
            if (r4 != 0) {
                *(int *)(s + 0x90) = (int)(w22 + 1);
            }
        }
    }

    w3 = (long)*(int *)(s + 0xF8);
    if (w3 == 2) {
        v2 = *(long *)(s + 0xF0);
        if (v2 >= 0) {
            *o1 = v2;
            *(int *)(s + 0xF8) = 0;
            *(long *)(s + 0xF0) = -1;
        }
    }

    a5 = (long)*(int *)(t + 0x40);
    x4 = (long)*(int *)(t + 0x3C);
    x2 = (long)*(int *)(t + 0x34);
    a5 = (long)((unsigned long)a5 << 5);
    x4 = (long)((unsigned long)x4 << 6);
    x6 = (long)*(int *)(t + 0x30);
    x7 = (long)*(int *)(t + 0x2C);
    a5 = a5 | x4;
    x3 = (long)*(int *)(t + 0x38);
    x2 = (long)((unsigned long)x2 << 8);
    x4 = *(long *)(t + 0x20);
    x2 = x2 | x7;
    x6 = (long)((unsigned long)x6 << 3);
    x3 = (long)((unsigned long)x3 << 7);
    *o3 = x4;
    x3 = x3 | x6;
    x2 = x2 | a5;
    x2 = x2 | x3;
    *o4 = x2;
    return x2;
}
