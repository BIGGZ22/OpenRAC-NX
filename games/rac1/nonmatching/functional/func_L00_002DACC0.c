typedef struct { float x, y, z, w; } Vq2 __attribute__((aligned(16)));
extern int func_001F4868_2DACC0(int) __asm__("func_001F4868");
extern void func_00234C98_2DACC0(int, long) __asm__("func_00234C98");
extern void func_001F9BD8_2DACC0(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0_2DACC0(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_001FF4B0_2DACC0(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9CA0_2DACC0(void *, void *, void *) __asm__("func_001F9CA0");
extern void func_001F7868_2DACC0(void) __asm__("func_001F7868");
extern void func_001F9EE8_2DACC0(void *, void *, void *) __asm__("func_001F9EE8");
extern void func_L00_001FDE48_2DACC0(int, int, int, void *, int) __asm__("func_L00_001FDE48");
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_001F9C30_2DACC0(void *, void *, float) __asm__("func_001F9C30");
extern void func_L00_001FD1D8_2DACC0(void *, void *, int) __asm__("func_L00_001FD1D8");
extern int func_L00_002D95D8_2DACC0(unsigned char *m) __asm__("func_L00_002D95D8");
extern void func_L00_001FF500_2DACC0(float *, float *, float) __asm__("func_L00_001FF500");
extern float func_L00_00200210_2DACC0(float, float) __asm__("func_L00_00200210");
extern char D_L00_001E40F0_2DACC0[] __asm__("D_L00_001E40F0");
extern char D_L00_00166EC0_2DACC0[] __asm__("D_L00_00166EC0") NOT_SDA;
extern char D_L00_00161BC0_2DACC0[] __asm__("D_L00_00161BC0") NOT_SDA;
extern char D_L00_001E1810_2DACC0[] __asm__("D_L00_001E1810");
extern char D_L00_001E3340_2DACC0[] __asm__("D_L00_001E3340");
extern char D_L00_001E2A30_2DACC0[] __asm__("D_L00_001E2A30");
extern int D_L00_00161BE0_2DACC0 __asm__("D_L00_00161BE0") MACRO_ADDR;
typedef struct { short v[4]; } Row_2DACC0;
extern Row_2DACC0 D_L00_001E3DD0_r[] __asm__("D_L00_001E3DD0");
extern char D_L00_001E37D0_2DACC0[] __asm__("D_L00_001E37D0");
extern char D_L00_001E3ED0_2DACC0[] __asm__("D_L00_001E3ED0");
extern Row_2DACC0 D_L00_001E3FD0_r[] __asm__("D_L00_001E3FD0");
extern short D_L00_00161AE0_2DACC0 __asm__("D_L00_00161AE0");
extern short D_L00_00161BA4_2DACC0 __asm__("D_L00_00161BA4");
extern short D_L00_00161B94_2DACC0 __asm__("D_L00_00161B94");
extern short D_L00_00161B90_2DACC0 __asm__("D_L00_00161B90");
extern short D_L00_00161B98_2DACC0 __asm__("D_L00_00161B98");
extern short D_L00_00161B9C_2DACC0 __asm__("D_L00_00161B9C");
extern short D_L00_00161BA0_2DACC0 __asm__("D_L00_00161BA0");
extern short D_L00_00161BA8_2DACC0 __asm__("D_L00_00161BA8");
extern short D_L00_00161BB0_2DACC0 __asm__("D_L00_00161BB0");
extern short D_L00_00161BAC_2DACC0 __asm__("D_L00_00161BAC");
extern short D_L00_00161BBC_2DACC0 __asm__("D_L00_00161BBC");
extern short D_L00_00161BB4_2DACC0 __asm__("D_L00_00161BB4");
extern short D_L00_00161BB8_2DACC0 __asm__("D_L00_00161BB8");

/* func_L00_002DACC0 -- src/overlays/shared/vendor_002D9438.c (functional C for the port, not a match)
 * Draw callback of a glowing orb: builds a camera-facing basis at the moby (lifted by its size),
 * transforms the 0x122-point mesh (D_L00_001E1810) and draws its three strips (func_L00_001FDE48),
 * then 32 glow quads from the D_L00_001E3DD0 rows (alpha by size), and when func_L00_002D95D8 says
 * so, four reflection quads with uvs from the camera direction.
 * From the staged near miss (nonmatching/shared/func_L00_002DACC0.c); fixed: the glow quads' first
 * two corners use the plain scale D_L00_00161BAC and the last two the scale times D_L00_00161BBC
 * (the near miss had them the other way round, which the equivalence tool cannot see).
 * equiv: DIFFERENT by shape only: retail keeps the row tables' full addresses in registers and
 * indexes them (sll 3, addu base); this adds the index to the high half and the low half after. */
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
    float f22 = *(float *)data / *(float *)&D_L00_00161AE0_2DACC0;
    int i, j, k, n2;
    float *out;
    char *in;

    q.w[1] = func_001F4868_2DACC0(*(int *)&D_L00_00161BA4_2DACC0);
    q.w[0] = 0;
    q.w[2] = 0xFF9000000260L;
    q.w[3] = (long)*(int *)&D_L00_00161B90_2DACC0 | ((long)*(int *)&D_L00_00161B94_2DACC0 << 2)
           | ((long)*(int *)&D_L00_00161B98_2DACC0 << 4) | ((long)*(int *)&D_L00_00161B9C_2DACC0 << 6)
           | ((long)*(int *)&D_L00_00161BA0_2DACC0 << 32);
    func_00234C98_2DACC0(0x47, 0x53001);
    vc0.x = *(float *)(m + 0x10);
    vc0.y = *(float *)(m + 0x14);
    vc0.z = *(float *)(m + 0x18) + 0.5f;
    vc0.w = 1.0f;
    vc0.z += *(float *)data;
    func_001F9BD8_2DACC0(&vc0, &vc0, D_L00_00161BC0_2DACC0);
    func_001F9BF0_2DACC0(&ve0, D_L00_00166EC0_2DACC0, &vc0);
    func_L00_001FF4B0_2DACC0(&v90, &ve0, 1.0f);
    func_001F9CA0_2DACC0(&va0, &v90, *(char **)(data + 0xC) + 0xE0);
    func_L00_001FF4B0_2DACC0(&va0, &va0, 1.0f);
    func_001F9CA0_2DACC0(&vb0, &va0, &v90);
    q.col[0] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[3] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[2] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[1] = *(int *)&D_L00_00161BA8_2DACC0;
    func_00234C98_2DACC0(6, func_001F4868_2DACC0(*(int *)&D_L00_00161BA4_2DACC0));
    func_00234C98_2DACC0(0x42, 0x8000000044L);
    func_00234C98_2DACC0(8, 0);
    func_00234C98_2DACC0(0x14, 0xFF9000000260L);
    func_00234C98_2DACC0(0x4A, 0);
    func_001F7868_2DACC0();
    out = (float *)D_L00_001E40F0_2DACC0;
    in = D_L00_001E1810_2DACC0;
    for (i = 0; i < 0x122; i++) {
        func_001F9EE8_2DACC0(&vf0, in, &v90);
        in += 0x10;
        out[0] = vf0.x;
        out[1] = vf0.y;
        out[2] = vf0.z;
        out += 3;
    }
    func_L00_001FDE48_2DACC0(0x94, (int)D_L00_001E40F0_2DACC0, (int)D_L00_001E3340_2DACC0, D_L00_001E2A30_2DACC0, 1);
    func_L00_001FDE48_2DACC0(0x6D, (int)(D_L00_001E40F0_2DACC0 + 0x6D8), (int)(D_L00_001E3340_2DACC0 + 0x248), D_L00_001E2A30_2DACC0 + 0x490, 1);
    func_L00_001FDE48_2DACC0(0x24, (int)(D_L00_001E40F0_2DACC0 + 0xBE8), (int)(D_L00_001E3340_2DACC0 + 0x3F8), D_L00_001E2A30_2DACC0 + 0x7F0, 1);
    q.w[1] = func_001F4868_2DACC0(0xB);
    q.w[3] = 0x8000000048L;
    {
        int n = func_001FA898_r(f22 * 64.0f + 128.0f);
        int c = ((n + D_L00_00161BE0_2DACC0) << 24) | *(int *)&D_L00_00161BB0_2DACC0;
        q.col[3] = 0;
        q.col[2] = 0;
        q.col[0] = c;
        q.col[1] = c;
    }
    i = 0;
    do {
        short *row;
        n2 = i + 1;
        row = D_L00_001E3DD0_r[i].v;
        for (j = 0; j < 4; j++) {
            if (!(j < 2)) {
                func_001F9C30_2DACC0(&q.pts[j], D_L00_001E37D0_2DACC0 + row[j] * 16, *(float *)&D_L00_00161BAC_2DACC0 * *(float *)&D_L00_00161BBC_2DACC0);
            } else {
                func_001F9C30_2DACC0(&q.pts[j], D_L00_001E37D0_2DACC0 + row[j] * 16, *(float *)&D_L00_00161BAC_2DACC0);
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
        func_L00_001FD1D8_2DACC0(&q, &v90, 0);
        i = n2;
    } while (i < 0x20);
    if (func_L00_002D95D8_2DACC0((unsigned char *)m)) {
        q.w[1] = func_001F4868_2DACC0(*(int *)&D_L00_00161BB4_2DACC0);
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
        i = 0;
        do {
            short *row;
            float *u0 = &q.uv[0];
            float *u1 = &q.uv[1];
            n2 = i + 1;
            row = D_L00_001E3FD0_r[i].v;
            for (k = 0; k < 4; k++) {
                func_001F9EE8_2DACC0(&q.pts[k], D_L00_001E3ED0_2DACC0 + row[k] * 16, &v90);
                func_001F9BF0_2DACC0(&vd0, &q.pts[k], D_L00_00166EC0_2DACC0);
                vd0.y = vd0.z;
                func_L00_001FF500_2DACC0((float *)&vd0, (float *)&vd0, *(float *)&D_L00_00161BB8_2DACC0);
                *u0 = vd0.x * 0.5f + 0.5f;
                *u0 = func_L00_00200210_2DACC0(*u0, 8.0f);
                *u1 = vd0.y * 0.5f + 0.5f;
                *u1 = func_L00_00200210_2DACC0(*u1, 8.0f);
                u0 += 2;
                u1 += 2;
            }
            func_L00_001FD1D8_2DACC0(&q, 0, 0);
            i = n2;
        } while (i < 4);
    }
    func_00234C98_2DACC0(0x47, 0x5360B);
}
