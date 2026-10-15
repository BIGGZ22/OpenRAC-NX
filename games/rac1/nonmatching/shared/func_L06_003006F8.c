/* NON_MATCHING func_L06_003006F8 -- src/overlays/shared/vendor_002FF000.c
 * Best so far: SIZE ours 936 / retail 952, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Shared moby update (vendor_002FF000.c): steps the timer at data+0x21C, spaces 16 entries at data+0x230 against
 *   Where it differs: gcc hoists the flag address (lui/addiu of D_0013E633+0xE1D) above the x==20 block, where ret
 *   Unblock: a source shape that keeps the flag address inside the loop and the counter ascending (perhaps the loo
 */
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_001F9BF0(void *dst, void *a, void *b);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(float *, float *, float *);
extern float func_001F9C78(void *a, void *b);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern float func_00214D28(float *, float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F49B0(void *, void *);
extern void func_L06_00300AB0(char *moby);
extern void func_L06_00300DA8(char *moby);
extern short D_L06_00162060;
extern float D_0015EE6C MACRO_ADDR;
extern unsigned char D_0013E633[] MACRO_ADDR;

/* Shared moby update: steps the timer at 0x21C, spaces 16 entries, and runs the pair checks. */
void func_L06_003006F8(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int x;
    float A[4];
    float W[4];
    char rec[0x20];
    float scale;
    float t;
    float *P;
    float *Q;
    int i, j, k, m;
    int n;

    x = func_001FA898_r(*(float *)(data + 0x21C));
    func_001F9908(&x);
    *(float *)(data + 0x21C) = (float)x;
    if (x == 0) {
        return;
    }
    if (x == 20) {
        char *p = data;
        for (n = 15; n >= 0; n--) {
            if (1.0f <= *(float *)(p + 0x23C)) {
                *(float *)(p + 0x23C) = 0.98f;
            }
            p += 0x10;
        }
    }

    P = (float *)(data + 0x230);
    Q = (float *)(data + 0x210);
    for (i = 0; i < 16; i++) {
        if (((unsigned char *)D_0013E633)[0xE1D + 0x20A4] == 1) {
            scale = (*(float *)&D_L06_00162060 * D_0015EE6C) * 0.5f;
        } else {
            scale = *(float *)&D_L06_00162060 * D_0015EE6C;
        }
        func_001F9BF0(A, P, Q);
        A[2] = 0.0f;
        func_L00_001FF4B0(A, A, scale);
        func_001F9BD8(A, P, A);
        func_001F9BF0(W, A, moby + 0x10);
        t = func_001F9C78(W, data + 0x220);
        if (0.4f < t) {
            A[2] = A[2] - (D_0015EE6C + D_0015EE6C);
        } else if (t < 0.3f) {
            A[2] = A[2] + (D_0015EE6C + D_0015EE6C);
        }
        if (1.0f <= P[3] && func_L00_001EFFF0(data + (i * 16 + 0x230), A, 2, 0, 0)) {
            A[3] = 0.98f;
        } else if (A[3] < 1.0f) {
            func_00214D28(&A[3], 0.0f, D_0015EE6C + D_0015EE6C);
        }
        qcopy(P, A);
        P += 4;
    }

    scale = 0.6f;
    j = 0;
    do {
        k = j + 1;
        if (0.6f <= *(float *)(data + j * 16 + 0x23C) && 0.6f <= *(float *)(data + k * 16 + 0x23C)) {
            A[0] = func_001F9F90(*(float *)(moby + 0x48));
            A[1] = func_001F9FA8(*(float *)(moby + 0x48));
            A[2] = 1.0f;
            A[3] = 5620.9248046875f;
            m = 0x10000;
            if (1.0f <= *(float *)(data + j * 16 + 0x23C) && 1.0f <= *(float *)(data + k * 16 + 0x23C)) {
                m = 0x10001;
            }
            func_L00_0025A8C0(rec, moby, m, 1.0f, A);
            *(unsigned short *)(rec + 0x1A) = *(unsigned short *)(moby + 0xA6);
            rec[0x18] = 0;
            rec[0x19] = 1;
            func_L00_001EFFF0(data + 0x230 + j * 16, data + 0x240 + j * 16, 0, moby, rec);
        }
        j = k;
    } while (j < 15);

    func_001F49B0((void *)func_L06_00300AB0, moby);
    func_L06_00300DA8(moby);
}
