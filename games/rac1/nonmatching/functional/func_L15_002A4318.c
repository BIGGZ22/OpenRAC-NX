/* functional: medium. Draws six passes of two glow quads for the moby: per pass picks a colour (eased by data[5]/60), animates the first quad's UVs with a sine/cosine of a frame counter, derives the second quad's UVs from them, draws both with the moby's rotation and shifts all corners by 0.1. From the NTSC decomp's draft; UV copy indices and gp tables were re-read from the assembly. */
typedef int Q_2A4318 __attribute__((mode(TI)));
typedef struct {
    Q_2A4318 corner[4];
    int col[4];
    float uv[4][2];
    long unk70, tex, unk80, unk88;
} Quad_2A4318;
extern int A0_2A4318[] __asm__("D_L15_00161558");
extern int A1_2A4318[] __asm__("D_L15_00161560");
extern int A2_2A4318[] __asm__("D_L15_00161568");
extern int A3_2A4318[] __asm__("D_L15_00161570");
extern int D_L15_0015F4F8_2A4318 __asm__("D_L15_0015F4F8");
extern int D_L15_001CE4B0_2A4318[] __asm__("D_L15_001CE4B0");
extern float D_L15_001CE4D0_2A4318[][2] __asm__("D_L15_001CE4D0");
extern int D_L15_001CE510_2A4318[][2] __asm__("D_L15_001CE510");
extern float D_L15_001CE550_2A4318[][2] __asm__("D_L15_001CE550");
extern Q_2A4318 D_L15_001CE570_2A4318[] __asm__("D_L15_001CE570");
extern Q_2A4318 D_L15_001CE5B0_2A4318[] __asm__("D_L15_001CE5B0");
extern void rot_2A4318(void *, void *) __asm__("func_001FA460");
extern long tex_2A4318(int) __asm__("func_001F4868");
extern int ease_2A4318(float, int, int) __asm__("func_001FA8A8");
extern int ftoi_2A4318(float) __asm__("func_001FA898");
extern float itof_2A4318(int) __asm__("func_001FA888");
extern float cos_2A4318(float) __asm__("func_001F9F90");
extern float sin_2A4318(float) __asm__("func_001F9FA8");
extern void draw_2A4318(void *, void *, int) __asm__("func_L00_001FD1D8");

void func_L15_002A4318(unsigned char *moby) {
    Quad_2A4318 first, second;
    float transform[12] __attribute__((aligned(16)));
    float position[4] __attribute__((aligned(16)));
    int *data = *(int **)(moby + 0x78);
    int pass, corner, axis, component, i;

    rot_2A4318(transform, moby + 0xC0);
    *(Q_2A4318 *)position = *(Q_2A4318 *)(moby + 0x10);
    position[3] = 1.0f;
    first.tex = tex_2A4318(0x2C);
    first.unk80 = 0xFF9000000260L;
    first.unk70 = 0;
    second.tex = tex_2A4318(0x2C);
    second.unk70 = 0;
    second.unk80 = 0xFF9000000260L;
    second.col[3] = 0;
    second.col[1] = 0;
    for (i = 0; i < 4; i++) {
        first.corner[i] = D_L15_001CE5B0_2A4318[i];
        second.corner[i] = D_L15_001CE570_2A4318[i];
    }
    for (pass = 0; pass < 6; pass++) {
        int parity = pass & 1;
        int level;
        long alpha;
        alpha = (long)A0_2A4318[parity] | ((long)A1_2A4318[parity] << 2) | ((long)A2_2A4318[parity] << 4) |
                ((long)A3_2A4318[parity] << 6) | 0x8000000000L;
        first.unk88 = alpha;
        second.unk88 = alpha;
        if (data[5] != 0) {
            level = ease_2A4318((float)data[5] / 60.0f, D_L15_001CE4B0_2A4318[pass], 0);
        } else {
            level = D_L15_001CE4B0_2A4318[pass];
        }
        first.col[0] = level;
        first.col[1] = level;
        first.col[2] = level;
        first.col[3] = level;
        second.col[0] = level;
        second.col[2] = level;
        for (corner = 0; corner < 4; corner++) {
            float *freq = D_L15_001CE4D0_2A4318[pass];
            int *wave = D_L15_001CE510_2A4318[pass];
            float *src = D_L15_001CE550_2A4318[corner];
            float *uv = first.uv[corner];
            for (axis = 0; axis < 2; axis++) {
                int limit = ftoi_2A4318(*freq * 360.0f);
                float angle = itof_2A4318(D_L15_0015F4F8_2A4318 % limit) / *freq - 180.0f;
                float delta;
                freq++;
                if (*wave != 0) {
                    delta = cos_2A4318(angle * 0.017453292f) * 0.125f;
                } else {
                    delta = sin_2A4318(angle * 0.017453292f) * 0.125f;
                }
                wave++;
                *uv = *src + delta;
                src++;
                uv++;
            }
        }
        for (component = 0; component < 2; component++) {
            float step = (float)component * 0.125f;
            second.uv[0][component] = first.uv[1][component];
            second.uv[1][component] = first.uv[1][component] + step;
            second.uv[2][component] = first.uv[3][component];
            second.uv[3][component] = first.uv[3][component] + step;
        }
        draw_2A4318(&first, transform, 0);
        draw_2A4318(&second, transform, 0);
        for (i = 0; i < 4; i++) {
            ((float *)&first.corner[i])[0] += 0.1f;
            ((float *)&second.corner[i])[0] += 0.1f;
        }
    }
}
