/* For each 0x10-byte record of the table at D_L00_0016CE80 (count at D_L00_0015F518), writes one or
 * two 0x70-byte blocks at the output pointer D_L00_00161280, then tail-calls func_00218A78 when the
 * count runs out. The five quadwords come from D_L00_001608E0..D_L00_00160920 (gp-relative). */
typedef struct { unsigned int w[4]; } __attribute__((aligned(16))) Quad16;
extern Quad16 D_L00_001608E0 MACRO_ADDR;
extern Quad16 D_L00_001608F0 MACRO_ADDR;
extern Quad16 D_L00_00160900 MACRO_ADDR;
extern Quad16 D_L00_00160910 MACRO_ADDR;
extern Quad16 D_L00_00160920 MACRO_ADDR;
extern int D_L00_0015F518 MACRO_ADDR;
extern char *D_L00_00161280 MACRO_ADDR;
extern unsigned char D_L00_0016CE80[];
extern void func_00218A78(void);

void func_L00_0024E798(void) {
    Quad16 q11, q12, q13, q14, q15;
    unsigned char *rec;
    char *p;
    int count;
    unsigned int a, w8, h4, h6, hc, he, lo, hi, s, t;

    q11 = D_L00_001608E0;
    q12 = D_L00_001608F0;
    q13 = D_L00_00160900;
    q14 = D_L00_00160910;
    q15 = D_L00_00160920;
    count = D_L00_0015F518;
    p = D_L00_00161280;
    rec = D_L00_0016CE80;

    for (;;) {
        if (count == 0) {
            func_00218A78();
            return;
        }
        count = count - 1;
        a = *(unsigned int *)(rec + 0);
        h4 = *(unsigned short *)(rec + 4);
        h6 = *(unsigned short *)(rec + 6);

        if (a != 0) {
            *(Quad16 *)(p + 0x00) = q11;
            *(Quad16 *)(p + 0x10) = q12;
            *(Quad16 *)(p + 0x20) = q13;
            *(Quad16 *)(p + 0x30) = q14;
            *(Quad16 *)(p + 0x40) = (Quad16){{0, 0, 0, 0}};
            *(unsigned int *)(p + 0x48) = 0x53;
            *(Quad16 *)(p + 0x50) = q15;
            *(unsigned char *)(p + 0x27) = (unsigned char)h4;
            h4 = h4 >> 1;
            *(unsigned short *)(p + 0x24) = (unsigned short)h6;
            *(unsigned int *)(p + 0x30) = 0x10;
            *(unsigned int *)(p + 0x34) = 0x10;
            s = 0x40u >> (h4);
            *(unsigned int *)(p + 0x50) = s | 0x8000u;
            *(Quad16 *)(p + 0x60) = (Quad16){{0, 0, 0, 0}};
            *(unsigned int *)(p + 0x64) = a;
            *(unsigned int *)(p + 0x60) = 0x30000000u + s;
            *(unsigned int *)(p + 0x6C) = 0x50000000u + s;
            p += 0x70;
        }

        w8 = *(unsigned int *)(rec + 8);
        hc = *(unsigned short *)(rec + 0xC);
        he = *(unsigned short *)(rec + 0xE);
        *(Quad16 *)(p + 0x00) = q11;
        *(Quad16 *)(p + 0x10) = q12;
        *(Quad16 *)(p + 0x20) = q13;
        *(Quad16 *)(p + 0x30) = q14;
        *(Quad16 *)(p + 0x40) = (Quad16){{0, 0, 0, 0}};
        *(unsigned int *)(p + 0x48) = 0x53;
        *(Quad16 *)(p + 0x50) = q15;
        lo = hc & 0xFF;
        hi = hc >> 8;
        *(unsigned short *)(p + 0x24) = (unsigned short)he;
        if ((int)(lo - 6) > 0) {
            *(unsigned char *)(p + 0x26) = (unsigned char)(1u << ((lo - 6)));
        }
        t = 1u << (hi);
        s = 1u << (lo);
        *(unsigned int *)(p + 0x34) = t;
        t = t << (lo);
        *(unsigned int *)(p + 0x30) = s;
        t = t >> 4;
        *(unsigned int *)(p + 0x50) = t | 0x8000u;
        *(Quad16 *)(p + 0x60) = (Quad16){{0, 0, 0, 0}};
        *(unsigned int *)(p + 0x64) = w8;
        *(unsigned int *)(p + 0x60) = 0x30000000u + t;
        *(unsigned int *)(p + 0x6C) = 0x50000000u + t;
        p += 0x70;

        rec += 0x10;
    }
}
