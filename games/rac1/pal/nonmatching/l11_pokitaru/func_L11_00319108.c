/* NON_MATCHING func_L11_00319108 -- src/overlays/l11_pokitaru/vendor_00312BD8.c
 * Best so far: SIZE ours 944 / retail 952, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Shifts a moby's list (d+0x70 pointer array, count at d+0x64): rescores the entry at d+0x90/0xC0, moves the tai
 *   Left: the prologue saves $f21 early and zeroes $s1 later than retail (scheduler order); the f2 test is bc1fl w
 *   Would unblock: knowing whether retail splits the second cur<n check off the float loop; that decides the branc
 */
extern char *func_L00_0025B478(void *, int, int);
extern float func_00214D28(float *p, float target, float maxstep);
extern void func_L11_003198F8(char *m);
extern int func_0022ED80_c(int, int, int) __asm__("func_0022ED80");
extern void func_001F9C30(void *, void *, float);
extern void func_L00_00260108(void *, void *, int, float, float);
extern int func_001F9850(int);
extern void *func_L00_00265050_s(char *, int, void *, void *, int, int, float *, float *, float, float *) __asm__("func_L00_00265050");
extern void func_0020D678(void *);
extern float D_0015EE60 MACRO_ADDR;
extern float D_L11_0015F660[] MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern short D_L11_001623C0;
extern char D_0014171B[];
typedef int u128_19108 __attribute__((mode(TI)));

// shifts the moby's list down by one, rescoring and moving its tail entry
void func_L11_00319108(char *moby) {
    char *d = *(char **)(moby + 0x78);
    int cur = -1;
    float sum = 0.0f;
    int i;
    void **pp = (void **)(d + 0x70);
    for (i = 0; i < *(int *)(d + 0x64); i++, pp++) {
        char *o = func_L00_0025B478(*pp, -1, 0);
        if (o != 0) {
            sum += *(float *)(o + 0x2C);
            cur = i;
            ((unsigned char *)*pp)[0xA4] = 0xFF;
        }
    }
    func_00214D28((float *)(d + 0xE0), 0.0f, sum);
    if (sum != 0.0f) {
        if (*(float *)(d + 0xE0) / 80.0f * 8.0f <= (float)(*(int *)(d + 0x64) - 1)) {
            *(unsigned short *)(D_0014171B + 0x5FD) = 0xFFFF;
            {
                void **ptr = (void **)(d + 0x70);
                void *src;
                void *dst;
                int n = *(int *)(d + 0x64) - 1;
                *(int *)(d + 0x64) = n;
                func_L11_003198F8(ptr[n]);
                src = ptr[cur];
                dst = ptr[*(int *)(d + 0x64)];
                qcopy((char *)dst + 0x10, (char *)src + 0x10);
                qcopy((char *)dst + 0x40, (char *)src + 0x40);
                func_0022ED80_c(1, 0, (int)dst);
            }
            if (cur < *(int *)(d + 0x64)) {
                short *s = (short *)(d + 0x90);
                float diff = (float)(s[cur] - s[cur + 1]);
                float *q = (float *)(d + 0xC0) + cur;
                int k = cur;
                do {
                    q[0] = q[1];
                    q++;
                    k++;
                } while (k < *(int *)(d + 0x64));
                {
                    float *base = (float *)(d + 0xC0 + cur * 4);
                    float f2 = *base + diff;
                    if (f2 < 0.0f) {
                        *base = f2;
                        *base = f2 + (float)**(int **)(d + 0xF0);
                    }
                }
            }
            if (cur < *(int *)(d + 0x64)) {
                unsigned short *p = (unsigned short *)(d + 0x90) + cur;
                int k = cur;
                do {
                    *p = p[1];
                    p++;
                    k++;
                } while (k < *(int *)(d + 0x64));
            }
            if (*(int *)(d + 0x64) == 0) {
                float v[4];
                func_001F9C30(v, moby + 0xC0, *(float *)&D_L11_001623C0 * D_0015EE60);
                v[2] = v[2] + D_0015EE60 * 0.08f;
                func_0022ED80_c(2, 0, (int)moby);
                func_L00_00260108(moby, moby + 0x10, -1, 3.0f, 13.0f);
                {
                    int r = func_001F9850(0x5A);
                    func_L00_00265050_s(moby, 0x602, moby + 0x10, moby + 0x40, r, 0, v, D_L11_0015F660, D_0015EE70 * 12.0f, D_L11_0015F660);
                }
                {
                    int r = func_001F9850(0x5A);
                    func_L00_00265050_s(moby, 0x603, moby + 0x10, moby + 0x40, r, 0, v, D_L11_0015F660, D_0015EE70 * 12.0f, D_L11_0015F660);
                }
                {
                    int r = func_001F9850(0x5A);
                    func_L00_00265050_s(moby, 0x784, moby + 0x10, moby + 0x40, r, 0, v, D_L11_0015F660, D_0015EE70 * 12.0f, D_L11_0015F660);
                }
                func_0020D678(moby);
            }
        }
    }
}
