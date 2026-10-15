/* NON_MATCHING func_L05_0030F358 -- src/overlays/l05_rilgar/vendor_0030EB68.c
 * Best so far: SIZE ours 860 / retail 868, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Builds a 2x5 array of 0x90-byte entries from the float tables D_L05_0020BA80/BAA0/BAE0 (rows get r1/r2 from fu
 *   Best: p7.c, SIZE 860/868, frame 0x6A0 and one spill as retail. Left: retail hoists tmp+0x30 into $22 before th
 *   Unblock: a source form that keeps the copy pointer hoisted without the extra spill, and reproduces the second 
 */
typedef int u128 __attribute__((mode(TI)));
extern int D_L05_0015F6B0 MACRO_ADDR;
extern short D_L05_00160098;
extern char *D_L05_00160098_m __asm__("D_L05_00160098") MACRO_ADDR;
extern short *D_L05_001AC040[];
extern char D_L05_0020BAE0[];
extern char D_L05_0020BAA0[];
extern char D_L05_0020BA80[];
extern short D_L05_00161DFC;
extern short D_L05_00161E00;
extern short D_L05_00161E04;
extern short D_L05_00161E08;
extern short D_L05_00161E30;
extern short D_L05_00161E34;
extern short D_L05_00161E38;
extern short D_L05_00161E3C;
extern short D_L05_00161E58;
extern float func_001FA888(int);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern float func_001FA748(float, float);
extern int func_001F4868(int);
extern void func_001FA460(void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);

/* Fills ten 0x90-byte entries from the float tables, then runs the id list at a[0x21] against them. */
extern void func_L05_0030F358_2(char *a) __asm__("func_L05_0030F358");
void func_L05_0030F358_2(char *a) {
    char arr[2][5][0x90];
    char tmp[0x40];
    int i, j, n, nx, r1, r2, off;
    float ang, t;
    unsigned short *p;
    unsigned short *nxt;
    short cur;
    char *e, *base, *pa, *qa, *xb, *yb, *ib;

    ang = func_001FA888(D_L05_0015F6B0 & 0x3F) / 63.0f * 6.28318f;
    xb = arr[0][0] + 0x50;
    yb = arr[0][0] + 0x54;
    ib = arr[0][0] + 0x40;
    i = 0;
    do {
        nx = i + 1;
        t = (func_001F9FA8(ang) + 1.0f) * 0.5f;
        r1 = func_001FA8A8(*(int *)&D_L05_00161DFC, *(int *)&D_L05_00161E00, t);
        r2 = func_001FA8A8(*(int *)&D_L05_00161E04, *(int *)&D_L05_00161E08, t);
        ang = func_001FA748(ang, 1.2566371f);
        j = 0;
        do {
            func_001F4868(*(int *)&D_L05_00161E58);
            e = arr[j][i];
            off = j * 0x2D0 + i * 0x90;
            *(s64 *)(e + 0x88) = (s64)*(int *)&D_L05_00161E30 | ((s64)*(int *)&D_L05_00161E34 << 2) | ((s64)*(int *)&D_L05_00161E38 << 4) | ((s64)*(int *)&D_L05_00161E3C << 6) | 0x8000000000LL;
            *(s64 *)(e + 0x78) = 0x90;
            *(s64 *)(e + 0x80) = 0xFF9000000260LL;
            *(s64 *)(e + 0x70) = 0;
            for (n = 0; n < 4; n++) {
                *(float *)(xb + off + n * 8) = *(float *)(D_L05_0020BA80 + n * 8);
                *(float *)(yb + off + n * 8) = *(float *)(D_L05_0020BA80 + n * 8 + 4);
                if (j != 0) {
                    *(int *)(ib + off + n * 4) = r1;
                    *(u128 *)(e + n * 16) = *(u128 *)(D_L05_0020BAA0 + n * 16);
                    *(float *)(e + n * 16 + 8) = *(float *)(e + n * 16 + 8) + (float)(i + 1);
                } else {
                    *(int *)(ib + i * 0x90 + n * 4) = r2;
                    *(u128 *)(arr[0][i] + n * 16) = *(u128 *)(D_L05_0020BAE0 + n * 16);
                    *(float *)(arr[0][i] + n * 16 + 8) = *(float *)(arr[0][i] + n * 16 + 8) + (float)(i + 1);
                }
            }
        } while (++j < 2);
        i = nx;
    } while (nx < 5);

    p = (unsigned short *)D_L05_001AC040[*(unsigned char *)(a + 0x21)];
    do {
        off = (*p & 0x7FFF) << 8;
        base = D_L05_00160098_m + off;
        nxt = p + 1;
        if (*(short *)(base + 0xA6) == 0x346 && *(unsigned char *)(base + 0x20) == 1) {
            char *tq = tmp + 0x30;
            func_001FA460(tmp, base + 0xC0);
            *(u128 *)tq = *(u128 *)(D_L05_00160098_m + off + 0x10);
            *(float *)(tmp + 0x3C) = 1.0f;
            pa = arr[0][0];
            qa = arr[1][0];
            do {
                func_L00_001FD1D8(pa, tmp, 0);
                pa += 0x90;
                func_L00_001FD1D8(qa, tmp, 0);
                qa += 0x90;
            } while (pa < arr[1][0]);
        }
        cur = *(short *)p;
        p = nxt;
    } while (cur >= 0);
}
