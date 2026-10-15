/* NON_MATCHING func_L14_00316E60 -- src/overlays/l14_oltanis/vendor_002FF358.c
 * Best so far: SIZE ours 864 / retail 880, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Camera 21 activate on level 14: three keyframe tables summed from records at cam+0x34/0x38/0x3C, a view setup 
 *   Left: retail re-materialises the float table base (lui/addiu) after each of the three summing loops and reload
 *   Unblock: a way to keep the table base as a symbol across the loops without changing the frame size.
 */
extern char *D_L14_0015F050 MACRO_ADDR;
extern char *D_L14_0015F7EC_p __asm__("D_L14_0015F7EC") MACRO_ADDR;
extern short D_L14_00162578;
extern char D_0013E633[];
extern float func_L00_001FF860(float, float);
extern void func_L00_001ED900(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);

/* Activate function of camera 21 on level 14: builds the three keyframe sums and sets up the view. */
void func_L14_00316E60(char *moby) {
    float v0[4];
    float v20[4];
    float v30[4];
    char *q40;
    char *cam;
    char *rec;
    char *w;
    char *p;
    char *q;
    char *r;
    float *F;
    float acc;
    float k2;
    int k;

    cam = *(char **)(D_L14_0015F050 + (*(short *)(moby + 0x84) << 5) + 0x1C);
    *(int *)(cam + 0x20) = 2;
    *(short *)(cam + 0x30) = 0;
    *(short *)(cam + 0x32) = 0;

    F = D_L14_001F6FC0;
    rec = *(char **)(D_L14_0015F7EC_p + (*(int *)(cam + 0x34) << 5) + 0x10);
    acc = 0.0f;
    for (k = 0; k < *(int *)rec - 1; k++) {
        F[k] = acc;
        acc = acc + *(float *)(rec + 0x1C + k * 16);
    }
    F[k] = acc;

    rec = *(char **)(D_L14_0015F7EC_p + (*(int *)(cam + 0x38) << 5) + 0x10);
    q40 = moby + 0x20;
    acc = 0.0f;
    for (k = 0; k < *(int *)rec - 1; k++) {
        F[k + 1000] = acc;
        acc = acc + *(float *)(rec + 0x1C + k * 16);
    }
    F[k + 1000] = acc;

    rec = *(char **)(D_L14_0015F7EC_p + (*(int *)(cam + 0x3C) << 5) + 0x10);
    acc = 0.0f;
    for (k = 0; k < *(int *)rec - 1; k++) {
        F[k + 2000] = acc;
        acc = acc + *(float *)(rec + 0x1C + k * 16);
    }
    F[k + 2000] = acc;

    w = D_0013E633 + 0xE1D;
    k2 = 0.0174532925f;
    *(float *)&D_L14_00162578 = 1.0f;
    q = *(char **)(moby + 0x70);
    *(int *)(q + 0x88) = 1;
    q += 0x88;
    *(int *)(q + 0x10) = 0;
    *(int *)(q + 0x4) = 0;
    *(int *)(q + 0x8) = 0;
    *(int *)(q + 0xC) = 0;
    p = *(char **)(moby + 0x70);
    *(float *)(p + 0x60) = D_L14_001F67C8[0] * k2;
    *(float *)(p + 0x64) = D_L14_001F67C8[1] * k2;
    *(float *)(p + 0x68) = D_L14_001F67C8[2];
    *(float *)(p + 0x6C) = D_L14_001F67C8[3] * k2;
    *(float *)(p + 0x70) = D_L14_001F67C8[4] * k2;
    q = *(char **)(moby + 0x70);
    *(int *)(q + 0x74) = 0;
    q += 0x74;
    *(int *)(q + 0x10) = 0;
    *(int *)(q + 0x4) = 0;
    *(int *)(q + 0x8) = 0;
    *(int *)(q + 0xC) = 0;

    v0[0] = func_L00_001FF860(*(float *)(*(char **)(w + 0x2080) + 0xD0), *(float *)(*(char **)(w + 0x2080) + 0xD4));
    v0[1] = *(float *)(p + 0x64);
    v0[2] = *(float *)(p + 0x68);
    func_L00_001ED900(moby + 0x30, v0, w + 0x80);

    func_001F9BF0(v20, w + 0x80, moby + 0x30);
    func_001F9C30(v30, w + 0x290, -1.5f);
    func_001F9BD8(v20, v20, v30);
    func_L00_001FF4B0(moby, v20, 1.0f);
    func_001F9CA0(moby + 0x10, moby, v30);
    func_L00_001FF4B0(moby + 0x10, moby + 0x10, 1.0f);
    func_001F9CA0(q40, moby + 0x10, moby);

    r = (char *)D_L14_001F67C8 + 0x44;
    k = 38;
    while (k >= 0) {
        if (*(int *)r != 0) {
        } else {
            *(int *)r = 10000;
        }
        k--;
        r += 0x30;
    }
}
