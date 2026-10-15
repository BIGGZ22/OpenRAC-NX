/* NON_MATCHING func_L08_002DF8A0 -- src/overlays/l08_batalia/vendor_002B9438.c
 * Best so far: SIZE ours 784 / retail 800, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Spawns n effect mobys: func_L00_0025F4A8 with 18 args (two variants by flag), three func_002140F8 samples into
 *   Difference: retail spills the batch offset (a << 7) to sp+0x50, so its frame is 0x110 with the saves at 0x60..
 *   Would unblock: a C form that gcc2.95 spills the offset for (10 saved registers in retail), or a different regi
 */
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern int func_002140B0(int);
extern char *func_0020D348(int);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_001F9EE8(void *, void *, void *);
extern void func_001F9BC0(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern char *D_L08_0016016C MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;

// Spawns n effect mobys for the moby's batch at index a, flag picks the id set.
void func_L08_002DF8A0(char *moby, int a, int n, int flag) {
    float vec[4];
    float vec2[4];
    float vec3[4];
    int off = a << 7;
    char *data = *(char **)(moby + 0x78);
    char *pos = moby + 0x10;
    int k;
    int c = 0x40;
    if (!flag) {
        func_L00_0025F4A8(moby, data + 0x20, pos, 0.0f, 0.0f, 0x2D, 0x1E, 0x1E, 10.0f, 6.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
    } else {
        func_L00_0025F4A8(moby, data + 0x20, pos, 0.0f, 0.0f, 0x1E, 0x14, 0x14, 6.0f, 4.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
    }
    for (k = n; k > 0; k--) {
        int id;
        int i;
        char *m;
        float d;
        if (flag) {
            id = (func_002140B0(0xFF) & 1) ? 0x30E : 0x30F;
        } else {
            id = (func_002140B0(0xFF) & 1) ? 0x310 : 0x311;
        }
        m = func_0020D348(id);
        *(unsigned char *)(m + 0x30) = c;
        m[0x31] = 1;
        *(short *)(m + 0x32) = c;
        *(s64 *)(m + 0x38) = *(s64 *)(moby + 0x38);
        *(float *)(m + 0x40) = func_00214158();
        *(float *)(m + 0x44) = func_00214158();
        *(float *)(m + 0x48) = func_00214158();
        vec[0] = func_002140F8(-1.0f, 1.0f);
        vec[1] = func_002140F8(-1.0f, 1.0f);
        vec[2] = func_002140F8(-1.0f, 1.0f);
        vec[3] = 1.0f;
        func_001F9EE8(vec, vec, D_L08_0016016C + off);
        qcopy(m + 0x10, vec);
        func_001F9BC0(vec2);
        func_001F9BF0(vec3, vec, pos);
        d = D_0015EE6C;
        func_L00_001FF500(vec3, vec3, func_002140F8(d * 12.0f, d * 15.0f));
        d = D_0015EE6C;
        vec3[2] = func_002140F8(d * 6.0f, d * 18.0f);
        i = func_001FA898_r(func_002140F8(60.0f, 90.0f));
        func_L05_00319B58(m, (int)moby, vec3, vec2, i);
    }
}
