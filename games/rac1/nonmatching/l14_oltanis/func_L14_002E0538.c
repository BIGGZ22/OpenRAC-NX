/* NON_MATCHING func_L14_002E0538 -- src/overlays/l14_oltanis/vendor_002E0538.c
 * Best so far: SIZE ours 788 / retail 796, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - What it does: per-frame update of the level 14 moby's 19-entry ring table (sin/cos-like helpers func_001FA74
 *   - Best candidate p3/p4/p5: 788 of 796 bytes. Loop 3 and the first two loops match in order, but: (1) retail co
 *   - Unblock: a way to keep the run-time `(4 << 4) + 8` in the source without folding, or a regalloc change that 
 *   - Note: run 3 was an accidental duplicate of p1 (same file, try_func run twice); no other run repeats.
 */
extern float func_001FA748(float, float);
extern float func_001FA790(float, float);
extern float func_001FA888(int);
extern float func_L00_0025F368(float);
extern float func_001F9FA8(float);
extern float func_L00_00258C80(float, float);
extern float func_001F9F90(float x);
extern void func_001F49B0(void (*)(void), void *);
void func_L14_002E0858(char *m);

// Per-frame update for the level 14 moby: refreshes its ring table and hands the table to func_001F49B0.
void func_L14_002E0538(char *moby)
{
    char *d = *(char **)(moby + 0x78);
    char *o;
    char *p;
    int i;
    int k;
    float c1;
    float c16;
    float one;
    float f20;
    float f21;
    float f22;
    float a;
    float b;
    float c;
    float e;
    float g;
    float one2;
    float c16b;
    float one3;
    float c39;
    float k99;
    float k20;
    float k75;
    float k50;

    c1 = 0.1f;
    c16 = 1.6f;
    one = 1.0f;
    *(float *)(d + 0x1E0) = func_001FA748(*(float *)(d + 0x1E0), 0.17453292f);
    *(float *)(d + 0x1E4) = func_001FA790(*(float *)(d + 0x1E4), 0.80285144f);
    *(float *)(d + 0x1E8) = func_001FA748(*(float *)(d + 0x1E8), 0.05236f);
    f22 = 360.0f / (func_001FA888(10) * 0.5f) * 0.017453292f;

    o = d;
    for (i = 0; i < 5; i++) {
        func_001FA888(i);
        func_001FA888(10);
        if (i == 0) {
            f20 = 0.0f;
            f21 = f20;
        } else {
            f20 = (float)i;
            f21 = func_001F9FA8(func_L00_0025F368(*(float *)(d + 0x1E0) + f20 * f22)) * c1;
            f21 = f21 + func_L00_00258C80(0.0f, 0.2f);
        }
        *(float *)(o + 0x8) = f21;
        *(int *)(o + 0x4) = 0;
        f21 = *(float *)(d + 0x1F0) * f20 + c16;
        *(float *)(o + 0xC) = one;
        *(float *)o = f21;
        o += 0x10;
    }

    c16b = 1.6f;
    p = d + 0x48;
    one2 = 1.0f;
    k = 5;
    o = d + 0x50;
    for (; k < 10; k++) {
        *(float *)(o + 0x8) = *(float *)p;
        *(int *)(o + 0x4) = 0;
        *(float *)(o + 0xC) = one2;
        *(float *)o = *(float *)(d + 0x1F0) * (float)k + c16b;
        p -= 0x10;
        o += 0x10;
    }

    k99 = 0.99483764f;
    k20 = 0.20944f;
    k75 = 0.75f;
    k50 = 0.5f;
    c39 = 0.39f;
    one3 = 1.0f;
    o = d;
    for (i = 0; i < 19; i++) {
        f22 = (float)i;
        *(int *)(o + 0xA8) = 0;
        a = func_L00_0025F368(*(float *)(d + 0x1E4) + f22 * k99);
        b = func_L00_0025F368(*(float *)(d + 0x1E8) + f22 * k20);
        c = func_001F9FA8(b) * k75;
        e = func_001F9FA8(a);
        g = func_L00_00258C80(0.0f, 0.2f);
        f20 = c * e + g;
        *(float *)(o + 0xA8) = *(float *)(o + 0xA8) + f20;
        f20 = func_001F9F90(a) * k50;
        *(float *)(o + 0xAC) = one3;
        f22 = f22 * c39;
        *(float *)(o + 0xA4) = f20;
        *(float *)(o + 0xA0) = f22;
        o += 0x10;
    }

    func_001F49B0((void (*)(void))func_L14_002E0858, moby);
}
