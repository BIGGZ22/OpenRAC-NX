/*
 * tie_ad_gif_convert (written from the assembly). Records the tie ad self in
 * the table D_001E1A00 at the count D_0016104C, stores the count and the ad's
 * index, scales the float at +0x48 by 1024 for func_001FA898, and relocates the
 * three pointers at +0 .. +8 and the pair at +0xC and +0x2C. Then, for each of
 * the self's entries (count at +0x23, 0x50 bytes each, from the pointer at +0x2C),
 * writes four 64-bit GIF words from the record at a1 (index idx in D_001E3900)
 * or, when a1 is 0, from D_0019E640.
 */
extern int D_0016104C NOT_SDA;
extern char D_001E1C00[] NOT_SDA;
extern char D_001E1D00[] NOT_SDA;
extern char D_001E1A00[] NOT_SDA;
extern char D_001E3100[] NOT_SDA;
extern char D_001E3900[] NOT_SDA;
extern long D_0019E640[];
extern int D_0015EF8C MACRO_ADDR;
extern int func_001F9968(int);
extern int func_001FA898(float);

void func_00203F68(char *self, int a1, char *src, int a3) {
    int N;
    int N2;
    int R;
    float f;
    int k;
    int j;
    int P;
    int v;
    char *m;
    char *cur;
    char *dest;
    int idx;
    int local4;
    int local8;
    int local12;
    int t23;
    int F1;
    int F2;
    int H;
    int W;
    int t4;
    int K;
    int c4;
    int c7;
    int c9;
    int S8;
    int i;
    unsigned long D0;
    unsigned long D1;
    unsigned long D2;
    unsigned long D3;
    unsigned long dw4;
    unsigned long dw7;
    unsigned long dw8;
    unsigned long c4p;
    unsigned long c7p;
    char *rec;
    char *E;

    N = D_0016104C;
    *(short *)(D_001E1C00 + 2 * N) = (short)a3;
    *(unsigned char *)(D_001E1D00 + a3) = (unsigned char)N;
    *(char **)(D_001E1A00 + 4 * N) = self;
    f = *(float *)(self + 0x48) * 1024.0f;
    *(short *)(self + 0x46) = (short)N;
    R = func_001FA898(f);
    N2 = D_0016104C;
    D_0016104C = N2 + 1;
    *(int *)(D_001E3100 + 4 * N2) = R;

    *(int *)(self + 0x28) = 0;
    *(short *)(self + 0x26) = 0;

    for (k = 0; k < 3; k++) {
        P = *(int *)(self + 4 * k);
        if (P == 0) {
            continue;
        }
        P = P + (int)self;
        *(int *)(self + 4 * k) = P;
        if (*(unsigned char *)(self + 0x20 + k) != 0) {
            j = 0;
            do {
                *(int *)(P + 16 * j) = *(int *)(P + 16 * j) + P;
                j++;
            } while (j < *(unsigned char *)(self + 0x20 + k));
        }
    }

    v = *(int *)(self + 0x0C) + (int)self;
    m = (char *)(*(int *)(self + 0x2C) + (int)self);
    *(int *)(self + 0x0C) = v;
    *(int *)(self + 0x2C) = (int)m;

    dest = D_001E3900 + ((*(unsigned char *)(D_001E1D00 + a3)) << 4);
    for (i = 0; i < 4; i++) {
        ((int *)dest)[i] = ((int *)src)[i];
    }

    if (*(unsigned char *)(self + 0x23) == 0) {
        return;
    }

    cur = m;
    j = 0;
    do {
        local4 = *(int *)(cur + 0x10);
        idx = *(unsigned char *)(dest + j);
        local8 = *(int *)(cur + 0x30);
        t23 = *(int *)(cur + 0x14);
        local12 = *(int *)(cur + 0x34);

        if (a1 == 0) {
            E = (char *)D_0019E640;
            dw7 = *(unsigned long *)(E + 24 * idx);
            dw4 = *(unsigned long *)(E + 8 * (3 * idx + 1));
            dw8 = *(unsigned long *)(E + 8 * (3 * idx + 2));
            D1 = (dw4 & 0x1CUL) | (((unsigned long)(long)t23 << 6) | 0x20UL) | ((unsigned long)(long)local4 << 32);
            D3 = (unsigned long)(long)local8 | ((unsigned long)(long)local12 << 2) | ((unsigned long)(long)idx << 24);
            *(unsigned long *)(cur + 0x00) = dw7;
            *(unsigned long *)(cur + 0x10) = D1;
            *(unsigned long *)(cur + 0x20) = dw8;
            *(unsigned long *)(cur + 0x30) = D3;
        } else {
            rec = (char *)a1 + (idx << 4);
            t4 = (int)((unsigned int)*(unsigned short *)(rec + 4) << 16);
            H = t4 >> 22;
            W = t4 >> 23;
            if (!(0 < H)) {
                H = 1;
            }
            F1 = func_001F9968(t4 >> 16);
            if (!(0 < W)) {
                W = 1;
            }
            F2 = func_001F9968(*(short *)(rec + 6));
            K = D_0015EF8C >> 8;
            c7 = *(short *)(rec + 0x0E) + K;
            c9 = *(short *)(rec + 0x0C) + K;
            c4 = *(short *)(rec + 0x0A) + K;
            S8 = *(short *)(rec + 0x08);

            D0 = ((unsigned long)(long)H << 14)
               | (((unsigned long)(long)F1 << 26) | 0x01300000UL)
               | ((unsigned long)(long)F2 << 30);
            c4p = ((unsigned long)(long)c4 << 37) | (1UL << 34);
            D0 = D0 | c4p;
            D0 = D0 | (1UL << 63);

            c7p = ((unsigned long)(long)c7 << 40) | (1UL << 34);
            D2 = ((unsigned long)(long)W << 14) | ((unsigned long)(long)c9 << 20) | c7p;
            D2 = D2 | (1UL << 54);

            D1 = ((unsigned long)(long)(S8 - 1) << 2)
               | (((unsigned long)(long)t23 << 6) | 0x20UL)
               | ((unsigned long)(long)local4 << 32);

            D3 = (unsigned long)(long)local8 | ((unsigned long)(long)local12 << 2) | ((unsigned long)(long)idx << 24);

            *(unsigned long *)(cur + 0x00) = D0;
            *(unsigned long *)(cur + 0x10) = D1;
            *(unsigned long *)(cur + 0x20) = D2;
            *(unsigned long *)(cur + 0x30) = D3;
        }
        *(unsigned long *)(cur + 0x40) = 0;
        j++;
        cur = cur + 0x50;
    } while (j < *(unsigned char *)(self + 0x23));
}
