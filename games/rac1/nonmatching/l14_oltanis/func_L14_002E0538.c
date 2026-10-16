/* NON_MATCHING func_L14_002E0538 -- src/overlays/l14_oltanis/vendor_002E0538.c
 * Best so far: BYTES 148/796 (81.4% of the bytes match), checked 2026-10-16.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Plain rewrite: d as float pts[29][4] plus angles and spacing, loops over element pointers p = d->pts[i].
 *   Loop 2's runtime (4 << 4) + 8 is a second counter j = 4 set before loop 1 (loop.c then uses its register).
 *   Products before a call are separate statements (GCC evaluates all calls of an expression first).
 *   Left: float register order (retail: step f22, fi f20, z f21, 1.0 f23) and loop 2 reading pts[j][2]
 *   as (j << 4) + (d + 8) where retail has ((j << 4) + 8) + d. Loop 3's pointer starts at d + 0xA0, retail d.
 */
typedef struct {
    float pts[29][4];
    char pad1D0[0x10];
    float ang[3];
    float pad1EC;
    float spacing;
} RingData_2E0538;
extern float FastAddRots(float, float);
extern float FastSubRots(float, float);
extern float func_001FA888(int);
extern float func_L00_0025F368(float);
extern float FastSin(float);
extern float randf_sym(float, float);
extern float FastCos(float);
extern void AddDrawCallback_2e0538(void (*)(char *), char *) __asm__("func_001F49B0");
void func_L14_002E0858(char *m);

/* turns the moby's three angles, lays out a ring of ten points (the second five mirror the first
   five's wobble) and a wave of nineteen, then queues func_L14_002E0858 to draw them */
void func_L14_002E0538(char *moby) {
    RingData_2E0538 *d = *(RingData_2E0538 **)(moby + 0x78);
    float wobble = 0.1f;
    float base = 1.6f;
    float one = 1.0f;
    float *p;
    float step, fi, z, a, b;
    int i, j, k;

    d->ang[0] = FastAddRots(d->ang[0], 0.17453292f);
    d->ang[1] = FastSubRots(d->ang[1], 0.80285144f);
    d->ang[2] = FastAddRots(d->ang[2], 0.05236f);
    step = 360.0f / (func_001FA888(10) * 0.5f) * 0.017453292f;
    j = 4;
    for (i = 0; i < 5; i++) {
        func_001FA888(i) / func_001FA888(10);
        if (i == 0) {
            fi = 0.0f;
            z = fi;
        } else {
            fi = i;
            z = FastSin(func_L00_0025F368(d->ang[0] + fi * step)) * wobble;
            z += randf_sym(0.0f, 0.2f);
        }
        p = d->pts[i];
        p[2] = z;
        p[1] = 0.0f;
        p[3] = one;
        p[0] = d->spacing * fi + base;
    }
    for (k = 5; k < 10; k++) {
        p = d->pts[k];
        p[2] = d->pts[j][2];
        p[1] = 0.0f;
        p[3] = 1.0f;
        p[0] = d->spacing * k + 1.6f;
        j--;
    }
    for (i = 0; i < 19; i++) {
        p = d->pts[i + 10];
        fi = i;
        p[2] = 0.0f;
        a = func_L00_0025F368(d->ang[1] + fi * 0.99483764f);
        b = func_L00_0025F368(d->ang[2] + fi * 0.20944f);
        z = FastSin(b) * 0.75f;
        p[2] += z * FastSin(a) + randf_sym(0.0f, 0.2f);
        p[3] = 1.0f;
        p[1] = FastCos(a) * 0.5f;
        p[0] = fi * 0.39f;
    }
    AddDrawCallback_2e0538(func_L14_002E0858, moby);
}
