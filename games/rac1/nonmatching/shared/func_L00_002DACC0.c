/* NON_MATCHING func_L00_002DACC0 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 1460 / retail 1464, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_002DACC0: builds a 32x34 lit mesh: view basis from the moby's data, projects a vertex table, emits qu
 *   Best candidate p7.c/p8.c: same instruction sequence as retail but 1448-1456 bytes vs 1464 and different saved-
 *   Ours keeps data in a register and loses the shared lui (or merges the &v90 copy): pressure/allocation tie that
 */
typedef struct { float x, y, z, w; } Vq2 __attribute__((aligned(16)));
extern int func_001F4868(int);
extern void func_00234C98(int, long);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F7868(void);
extern void func_001F9EE8(void *, void *, void *);
extern void func_L00_001FDE48(int, int, int, void *, int);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_001F9C30(void *, void *, float);
extern void func_L00_001FD1D8(void *, void *, int);
extern int func_L00_002D95D8(unsigned char *m);
extern void func_L00_001FF500(float *, float *, float);
extern float func_L00_00200210(float, float);
extern char D_L00_001E40F0[];
extern char D_L00_00166EC0[] NOT_SDA;
extern char D_L00_00161BC0[] NOT_SDA;
extern char D_L00_001E1810[];
extern char D_L00_001E3340[];
extern char D_L00_001E2A30[];
extern int D_L00_00161BE0 MACRO_ADDR;
extern char D_L00_001E3DD0[];
extern char D_L00_001E37D0[];
extern char D_L00_001E3ED0[];
extern char D_L00_001E3FD0[];
extern short D_L00_00161AE0;
extern short D_L00_00161BA4;
extern short D_L00_00161B94;
extern short D_L00_00161B90;
extern short D_L00_00161B98;
extern short D_L00_00161B9C;
extern short D_L00_00161BA0;
extern short D_L00_00161BA8;
extern short D_L00_00161BB0;
extern short D_L00_00161BAC;
extern short D_L00_00161BBC;
extern short D_L00_00161BB4;
extern short D_L00_00161BB8;

/* Draw a lit sphere-like billboard mesh for the moby: builds the view basis, projects the vertex grid and emits quads. */
void func_L00_002DACC0(char *m) {
    struct { Vq2 pts[4]; int col[4]; float uv[8]; long w[4]; } q;
    Vq2 v90;
    Vq2 va0;
    Vq2 vb0;
    Vq2 vc0;
    Vq2 vd0;
    Vq2 ve0;
    Vq2 vf0;
    char *data = *(char **)(m + 0x78);
    float f22 = *(float *)data / *(float *)&D_L00_00161AE0;
    int i, j, k;
    float *out;
    char *in;

    q.w[1] = func_001F4868(*(int *)&D_L00_00161BA4);
    q.w[0] = 0;
    q.w[2] = 0xFF9000000260L;
    q.w[3] = (long)*(int *)&D_L00_00161B90 | ((long)*(int *)&D_L00_00161B94 << 2)
           | ((long)*(int *)&D_L00_00161B98 << 4) | ((long)*(int *)&D_L00_00161B9C << 6)
           | ((long)*(int *)&D_L00_00161BA0 << 32);
    func_00234C98(0x47, 0x53001);
    vc0.x = *(float *)(m + 0x10);
    vc0.y = *(float *)(m + 0x14);
    vc0.z = *(float *)(m + 0x18) + 0.5f;
    vc0.w = 1.0f;
    vc0.z += *(float *)data;
    func_001F9BD8(&vc0, &vc0, D_L00_00161BC0);
    func_001F9BF0(&ve0, D_L00_00166EC0, &vc0);
    func_L00_001FF4B0(&v90, &ve0, 1.0f);
    func_001F9CA0(&va0, &v90, *(char **)(data + 0xC) + 0xE0);
    func_L00_001FF4B0(&va0, &va0, 1.0f);
    func_001F9CA0(&vb0, &va0, &v90);
    q.col[0] = *(int *)&D_L00_00161BA8;
    q.col[3] = *(int *)&D_L00_00161BA8;
    q.col[2] = *(int *)&D_L00_00161BA8;
    q.col[1] = *(int *)&D_L00_00161BA8;
    func_00234C98(6, func_001F4868(*(int *)&D_L00_00161BA4));
    func_00234C98(0x42, 0x8000000044L);
    func_00234C98(8, 0);
    func_00234C98(0x14, 0xFF9000000260L);
    func_00234C98(0x4A, 0);
    func_001F7868();
    out = (float *)D_L00_001E40F0;
    in = D_L00_001E1810;
    for (i = 0; i < 0x122; i++) {
        func_001F9EE8(&vf0, in, &v90);
        in += 0x10;
        out[0] = vf0.x;
        out[1] = vf0.y;
        out[2] = vf0.z;
        out += 3;
    }
    func_L00_001FDE48(0x94, (int)D_L00_001E40F0, (int)D_L00_001E3340, D_L00_001E2A30, 1);
    func_L00_001FDE48(0x6D, (int)(D_L00_001E40F0 + 0x6D8), (int)(D_L00_001E3340 + 0x248), D_L00_001E2A30 + 0x490, 1);
    func_L00_001FDE48(0x24, (int)(D_L00_001E40F0 + 0xBE8), (int)(D_L00_001E3340 + 0x3F8), D_L00_001E2A30 + 0x7F0, 1);
    q.w[1] = func_001F4868(0xB);
    q.w[3] = 0x8000000048L;
    {
        int n = func_001FA898_r(f22 * 64.0f + 128.0f);
        int c = ((n + D_L00_00161BE0) << 24) | *(int *)&D_L00_00161BB0;
        q.col[3] = 0;
        q.col[2] = 0;
        q.col[0] = c;
        q.col[1] = c;
    }
    for (i = 0; i < 0x20; i++) {
        short *row = (short *)(D_L00_001E3DD0 + i * 8);
        for (j = 0; j < 4; j++) {
            if (j < 2) {
                func_001F9C30(&q.pts[j], D_L00_001E37D0 + row[j] * 16, *(float *)&D_L00_00161BAC * *(float *)&D_L00_00161BBC);
            } else {
                func_001F9C30(&q.pts[j], D_L00_001E37D0 + row[j] * 16, *(float *)&D_L00_00161BAC);
            }
        }
        q.uv[0] = 0.5f;
        q.uv[1] = 0.5f;
        q.uv[2] = 0.5f;
        q.uv[3] = 0.5f;
        q.uv[4] = 0.5f;
        q.uv[5] = 1.0f;
        q.uv[6] = 0.5f;
        q.uv[7] = 1.0f;
        func_L00_001FD1D8(&q, &v90, 0);
    }
    if (func_L00_002D95D8((unsigned char *)m)) {
        q.w[1] = func_001F4868(*(int *)&D_L00_00161BB4);
        q.col[0] = 0x107F7F7F;
        q.col[3] = 0x107F7F7F;
        q.col[2] = 0x107F7F7F;
        q.col[1] = 0x107F7F7F;
        q.w[3] = 0x8000000044L;
        {
            char *p = *(char **)(data + 0xC);
            qcopy(&v90, p + 0xC0);
            qcopy(&va0, p + 0xD0);
            qcopy(&vb0, p + 0xE0);
        }
        vc0.z = *(float *)(m + 0x18);
        for (i = 0; i < 4; i++) {
            short *row = (short *)(D_L00_001E3FD0 + i * 8);
            for (k = 0; k < 4; k++) {
                func_001F9EE8(&q.pts[k], D_L00_001E3ED0 + row[k] * 16, &v90);
                func_001F9BF0(&vd0, &q.pts[k], D_L00_00166EC0);
                vd0.y = vd0.z;
                func_L00_001FF500((float *)&vd0, (float *)&vd0, *(float *)&D_L00_00161BB8);
                q.uv[k * 2] = vd0.x * 0.5f + 0.5f;
                q.uv[k * 2] = func_L00_00200210(q.uv[k * 2], 8.0f);
                q.uv[k * 2 + 1] = vd0.y * 0.5f + 0.5f;
                q.uv[k * 2 + 1] = func_L00_00200210(q.uv[k * 2 + 1], 8.0f);
            }
            func_L00_001FD1D8(&q, 0, 0);
        }
    }
    func_00234C98(0x47, 0x5360B);
}
