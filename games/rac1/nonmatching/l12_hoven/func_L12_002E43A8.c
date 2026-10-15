/* NON_MATCHING func_L12_002E43A8 -- src/overlays/l12_hoven/vendor_002C0310.c
 * Best so far: SIZE ours 924 / retail 900, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 12 hoven entry update: a count from D_L12_001618BC (scaled by 1.5 when state 15), then per entry a 3-vec
 *   Best candidates p0.c and p2.c: 924 bytes against retail 900, every call and branch shape is in place. Differen
 *   Unblock: the constant hold (f20-f24) and the spill order of the moby pointer; the 0.55 constant is 0x3F0CCCCD.
 *   2026-10-15: compiles again. The file now declares it with a struct Moby * parameter, so the candidate
 *   takes that and casts it to the char * it used.
 */
extern short D_L12_001618BC;
extern short D_L12_001618C0;
extern short D_L12_001618C4;
extern short D_L12_001618C8;
extern short D_L12_001618CC;
extern short D_L12_001618D0;
extern short D_L12_001618D4;
extern short D_L12_001618D8;
extern short D_L12_001618DC;
extern short D_L12_001618E0;
extern short D_L12_001618E4;
extern short D_L12_001618E8;
extern short D_L12_001618EC;
extern short D_L12_001618F0;
extern short D_L12_001618F4;
extern short D_L12_001618F8;
extern short D_L12_001618FC;
extern short D_L12_00161900;
extern short D_L12_00161904;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9850(int);
extern int func_001FA8A8(int, int, float);
extern float func_001F9878(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern char *func_00219780(void *, void *, void *, int, int, int, int, int, int);
extern void func_L00_00260958(float *v, float s);

/* Level 12 hoven: per-entry update that places and blends a set of points around the moby; fills a 0x50-byte scratch area. */
void func_L12_002E43A8(struct Moby *moby) {
    char *m = (char *)moby;
    u128 vv[5];
    float *fv;
    int cnt;
    int i;
    int k;
    int n;
    int a19;
    int a18;
    int a17;
    int a16;
    int a2;
    int j;
    float f20;
    float f21;
    float f22;
    float t;
    float t2;
    float t3;
    float t4;
    float tmp;
    float r0;
    float k55;
    float k0;

    fv = (float *)vv;
    k55 = 0.55f;
    k0 = 0.0f;
    cnt = *(int *)&D_L12_001618BC;
    if (*(unsigned char *)(m + 0x20) == 15) cnt = func_001FA898_r((float)cnt * 1.5f);
    for (i = 0; i < cnt; i++) {
        qcopy(vv + 1, m + 0x10);
        func_L00_00260958((float *)(vv + 1), *(float *)&D_L12_001618C4);
        fv[6] = fv[6] + k55;
        f22 = func_00214158();
        f20 = func_002140F8(*(float *)&D_L12_001618D0, *(float *)&D_L12_001618D4);
        f21 = func_002140F8(*(float *)&D_L12_001618D8, *(float *)&D_L12_001618DC);
        if (*(unsigned char *)(m + 0x20) == 15) {
            f20 = f20 * k55;
            f21 = f21 * k55;
            fv[6] = fv[6] - 0.2f;
        }
        tmp = func_001F9F90(f22);
        fv[8] = tmp * (f20 * D_0015EE6C);
        r0 = func_001F9FA8(f22);
        fv[10] = f21 * D_0015EE6C + k0;
        fv[9] = r0 * (f20 * D_0015EE6C);
        func_001F9C30(vv + 4, *(char **)(m + 0x78) + 0x120, 0.5f);
        fv[18] = k0;
        func_001F9BD8(vv + 2, vv + 2, vv + 4);
        qcopy(vv + 3, vv + 2);
        j = func_001F9850(*(int *)&D_L12_00161900);
        fv[14] = fv[14] - *(float *)&D_L12_001618E0 * D_0015EE70 * (float)j;
        n = *(int *)&D_L12_001618C0;
        if (n > 0) {
            f22 = 0.5f;
            f21 = 0.0f;
            f20 = 1.0f;
            for (k = 0; k < *(int *)&D_L12_001618C0; k++) {
                t = func_002140F8(f22, 1.5f);
                fv[11] = t * *(float *)&D_L12_001618E4;
                fv[15] = t * *(float *)&D_L12_001618E8;
                func_L00_00260958((float *)(vv + 1), *(float *)&D_L12_001618C8);
                func_L00_00260958((float *)(vv + 3), *(float *)&D_L12_001618CC * D_0015EE6C);
                t2 = func_002140F8(f21, f20);
                a19 = func_001FA8A8(*(int *)&D_L12_001618EC, *(int *)&D_L12_001618F4, t2);
                t3 = func_002140F8(f21, f20);
                a18 = func_001FA8A8(*(int *)&D_L12_001618F0, *(int *)&D_L12_001618F8, t3);
                a17 = func_001F9850(*(int *)&D_L12_001618FC);
                a16 = func_001F9850(*(int *)&D_L12_00161900);
                t4 = func_002140F8((float)*(int *)&D_L12_00161904 * f22, (float)*(int *)&D_L12_00161904 * 2.5f);
                a2 = func_001FA898_r(func_001F9878(t4));
                func_00219780(vv + 1, vv + 2, vv + 3, a19, a18, a17, a16, a2, -1);
            }
        }
    }
}
