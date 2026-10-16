/* functional: medium. Draw callback of an electric-arc/ribbon moby: builds a view matrix from the table entry, then draws a ribbon quad per segment (31 segments of the main strand, then three further strands of 3 segments for each of up to 4 branches whose count byte is < 0x1F) with func_L00_001FD1D8. Segment corners are the strand points +- the previous/current segment normal (cross product, scaled by 0.1); u coordinates add (i&3)*0.25. Packet field meanings are guessed from the layout. */
typedef struct Pkt_03430 {
    float corner[4][4];
    unsigned int color[4];
    float uv[4][2];
    long zero;
    long tex;
    long state;
    long flags;
} Pkt_03430;
typedef struct Vq_03430 { float x, y, z, w; } __attribute__((aligned(16))) Vq_03430;

extern void mat_a_03430(void *, void *) __asm__("func_001FA218");
extern void mat_b_03430(void *, void *) __asm__("func_001FA4A0");
extern void sub_03430(void *, void *, void *) __asm__("func_001F9BF0");
extern void add_03430(void *, void *, void *) __asm__("func_001F9BD8");
extern void xform_03430(void *, void *, void *) __asm__("func_001F9EC0");
extern void cross_03430(void *, void *, void *) __asm__("func_001F9CA0");
extern void zero_03430(void *) __asm__("func_001F9BC0");
extern void setlen_03430(void *, void *, float) __asm__("func_L00_001FF4B0");
extern float jit_03430(int) __asm__("func_001FA888");
extern long tex_03430(int) __asm__("func_001F4868");
extern void draw_03430(void *, void *, int) __asm__("func_L00_001FD1D8");
extern char *D_L14_001601AC_03430 __asm__("D_L14_001601AC");
extern float D_L14_001675C0_03430[] __asm__("D_L14_001675C0");
extern float D_L14_001E6320_03430[][2] __asm__("D_L14_001E6320");
extern int D_L14_001621AC_03430 __asm__("D_L14_001621AC");
extern int D_L14_001621B0_03430 __asm__("D_L14_001621B0");
extern int D_L14_001621B4_03430 __asm__("D_L14_001621B4");
extern int D_L14_001621B8_03430 __asm__("D_L14_001621B8");
extern int D_L14_001621BC_03430 __asm__("D_L14_001621BC");

void func_L14_00303430_impl(char *m) __asm__("func_L14_00303430");
void func_L14_00303430_impl(char *m) {
    char *d = *(char **)(m + 0x78);
    Pkt_03430 P[3] __attribute__((aligned(16)));
    float M1[16] __attribute__((aligned(16)));
    float M2[16] __attribute__((aligned(16)));
    float v230[4] __attribute__((aligned(16)));
    float v240[4] __attribute__((aligned(16)));
    float v250[4] __attribute__((aligned(16)));
    float v260[4] __attribute__((aligned(16)));
    float v270[4] __attribute__((aligned(16)));
    char *ent;
    long tex;
    long flags;
    int i, j, n, k;

    ent = D_L14_001601AC_03430 + (*(int *)d << 7);
    mat_a_03430(M1, ent + 0x70);
    mat_b_03430(M2, M1);
    sub_03430(v230, D_L14_001675C0_03430, ent + 0x30);
    xform_03430(v230, v230, M2);
    tex = tex_03430(D_L14_001621AC_03430);
    flags = (long)D_L14_001621B0_03430 | ((long)D_L14_001621B4_03430 << 2) | ((long)D_L14_001621B8_03430 << 4)
            | ((long)D_L14_001621BC_03430 << 6) | (0x8000L << 24);
    for (k = 0; k < 3; k++) {
        P[k].zero = 0;
        P[k].tex = tex;
        P[k].state = 0xFF9000000260L;
        P[k].flags = flags;
    }
    for (j = 0; j < 4; j++) {
        float v = D_L14_001E6320_03430[j][1];
        int c = *(int *)(d + 0x14);
        P[2].uv[j][1] = v;
        P[1].uv[j][1] = v;
        P[0].uv[j][1] = v;
        P[2].color[j] = c;
        P[1].color[j] = c;
        P[0].color[j] = c;
    }
    zero_03430(v250);
    for (i = 0; i < 31; i++) {
        char *p16;
        char *p20;
        *(Vq_03430 *)v240 = *(Vq_03430 *)v250;
        if (i < 30) {
            sub_03430(v270, d + (i << 4) + 0x20, d + (i << 4) + 0x40);
            sub_03430(v260, d + (i << 4) + 0x30, v230);
            cross_03430(v250, v260, v270);
            setlen_03430(v250, v250, 0.1f);
        } else {
            zero_03430(v250);
        }
        p16 = d + (i << 4) + 0x20;
        p20 = d + (i << 4) + 0x30;
        for (k = 0; k < 4; k++) {
            float f;
            if (k < 2) {
                *(Vq_03430 *)P[0].corner[k] = *(Vq_03430 *)p16;
                if (k & 1) add_03430(P[0].corner[k], P[0].corner[k], v240);
                else sub_03430(P[0].corner[k], P[0].corner[k], v240);
            } else {
                *(Vq_03430 *)P[0].corner[k] = *(Vq_03430 *)p20;
                if (k & 1) add_03430(P[0].corner[k], P[0].corner[k], v250);
                else sub_03430(P[0].corner[k], P[0].corner[k], v250);
            }
            f = jit_03430(i - ((i >> 2) << 2)) * 0.25f;
            P[0].uv[k][0] = D_L14_001E6320_03430[k][0] + f;
        }
        draw_03430(&P[0], M1, 0);
    }
    for (j = 0; j < 4; j++) {
        unsigned char b;
        zero_03430(v250);
        b = *(unsigned char *)(d + 0x18 + j);
        if (b >= 0x1F) continue;
        for (n = 0; n < 3; n++) {
            char *base = d + (n << 6) + (j << 4);
            *(Vq_03430 *)v240 = *(Vq_03430 *)v250;
            if (n < 2) {
                sub_03430(v270, base + 0x420, base + 0x4A0);
                sub_03430(v260, base + 0x460, v230);
                cross_03430(v250, v260, v270);
                setlen_03430(v250, v250, 0.1f);
            } else {
                zero_03430(v250);
            }
            if (n == 0) *(Vq_03430 *)v240 = *(Vq_03430 *)v250;
            for (k = 0; k < 4; k++) {
                float f;
                if (k < 2) {
                    *(Vq_03430 *)P[0].corner[k] = *(Vq_03430 *)(base + 0x420);
                    if (k & 1) add_03430(P[0].corner[k], P[0].corner[k], v240);
                    else sub_03430(P[0].corner[k], P[0].corner[k], v240);
                } else {
                    *(Vq_03430 *)P[0].corner[k] = *(Vq_03430 *)(base + 0x460);
                    if (k & 1) add_03430(P[0].corner[k], P[0].corner[k], v250);
                    else sub_03430(P[0].corner[k], P[0].corner[k], v250);
                }
                f = jit_03430(n - ((n >> 2) << 2)) * 0.25f;
                P[0].uv[k][0] = D_L14_001E6320_03430[k][0] + f;
            }
            draw_03430(&P[0], M1, 0);
        }
    }
}
