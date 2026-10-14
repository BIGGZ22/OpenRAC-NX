/* NON_MATCHING func_L14_002E1570 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: BYTES 33/584 (94.3% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Spawns effect moby class 0x193 at owner+0x10 (copy via qcopy), then two setups: state 0 calls func_L00_0025EFC
 *   Best p4.c: registers all match, 588 vs 584 bytes. Left: order of the owner+0x48 float load and the 0xFF consta
 *   the 0xFF/1 constant scheduling, arg setup order in the state-0 call (retail: 5.0f/0 after the D_L14_0015F7EC l
 *   q28/t01: p9.c is closest (584 = retail size, 54 differing words). Fixes found: read owner+0x48 into a local be
 *   Left: register of the 0x31 byte store (ours $v1, retail $a0) and scheduling of the D_0015EE6C/D_L14_0015F7EC/2
 *   hq13/s08 (3 runs, p10-p12; best p11 at 33 of 584 bytes differing, from p9's 54): owner+0x48 is read after the 
 */
typedef int u128_E1570 __attribute__((mode(TI)));
extern char D_0013E633[];
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern int D_L14_0015F7EC MACRO_ADDR;
extern short D_L14_00161C00;
extern short D_L14_00161C04;
extern short D_L14_00161C08;
extern short D_L14_00161C0C;
extern unsigned char *func_0020D348(int);
extern int func_001F9850(int);
extern void func_L00_0025EFC0(int, void *, void *, void *, void *, int, float, float, float);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern float func_002140F8(float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern float func_L00_00258C80(float lo, float hi);
extern void func_002156E0(void *dst, void *vec, void *axis, float angle);
extern void func_L00_00251E30(void *);

/* Spawns an effect moby (class 0x193) at owner's position and sets up its motion. */
unsigned char *func_L14_002E1570(char *owner, float *a, int state) {
    unsigned char *moby = func_0020D348(0x193);
    if (moby != 0) {
        char *data = *(char **)(moby + 0x78);
        char *pos = (char *)moby + 0x10;
        float v[8];
        float f48;
        qcopy(pos, owner + 0x10);
        f48 = *(float *)(owner + 0x48);
        moby[0x20] = state;
        moby[0x30] = 0xFF;
        *(float *)(moby + 0x48) = f48;
        *(short *)(moby + 0x32) = 0xFF;
        moby[0x31] = 1;
        *(char **)data = owner;
        if (moby[0x20] == 0) {
            int t;
            char *tbl;
            *(int *)(data + 4) = func_001F9850(0x5A);
            tbl = (char *)D_L14_0015F7EC;
            *(float *)(data + 8) = a[0] * D_0015EE6C;
            t = *(int *)(*(char **)(owner + 0x78) + 0x60);
            *(int *)(data + 0x14) = 0;
            *(int *)(data + 0xC) = t;
            *(int *)(data + 0x10) = 0;
            func_L00_0025EFC0(*(int *)(tbl + t * 32 + 0x10), pos, v, data + 0x10, data + 0x14, 0, 20.0f, 5.0f, *(float *)(data + 0x14));
        } else {
            float z = 0.0f;
            char *p;
            char *q;
            float ang;
            *(int *)(data + 4) = func_001F9850(200);
            p = D_0013E633 + 0xE9D;
            func_001F9BF0(v, pos, p);
            q = p + 0x210;
            v[2] = z;
            func_L00_001FF4B0(v, v, func_002140F8(*(float *)&D_L14_00161C00 * D_0015EE60, *(float *)&D_L14_00161C04 * D_0015EE60));
            func_001F9CA0(v + 4, q, v);
            ang = func_L00_00258C80(0.57595867f, 1.3962634f);
            func_002156E0(data + 0x30, v, q, ang);
            *(float *)(data + 0x38) = func_002140F8(*(float *)&D_L14_00161C08 * D_0015EE60, *(float *)&D_L14_00161C0C * D_0015EE60);
            *(float *)(data + 0x20) = func_L00_00258C80(z, 0.008726646f);
            *(float *)(data + 0x24) = func_L00_00258C80(0.05235988f, 0.13962634f);
            *(float *)(data + 0x28) = func_L00_00258C80(z, 0.017453292f);
        }
        func_L00_00251E30(moby);
    }
    return moby;
}
