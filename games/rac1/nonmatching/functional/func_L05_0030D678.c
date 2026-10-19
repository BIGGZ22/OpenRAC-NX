/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Draw callback of the class 0x337 wobbling effect: builds a basis from the moby (rotation from its pvars phase, direction toward the camera position) and draws ten pairs of textured quads that widen with each step. Statically checked only. */
typedef u32 u128_0D678 __attribute__((mode(TI), aligned(16)));
typedef struct {
    float vertex[4][4];
    int color[4];
    float texcoord[8];
    long control[4];
} Quad_0D678;
extern void f_pos_0D678(void *, int, float *) __asm__("func_L00_00250800");
extern void f_clear_0D678(float *) __asm__("func_001F9BC0");
extern float f_sin_0D678(float) __asm__("func_001F9FA8");
extern void f_rot_0D678(float *, float *) __asm__("func_001FA218");
extern void f_sub_0D678(float *, void *, float *) __asm__("func_001F9BF0");
extern void f_norm_0D678(float *, float *, float) __asm__("func_L00_001FF4B0");
extern void f_cross_0D678(float *, float *, float *) __asm__("func_001F9CA0");
extern long f_tex_0D678(int) __asm__("func_001F4868");
extern int f_tween_0D678(int, int, float) __asm__("func_001FA8A8");
extern void f_draw_0D678(void *, float *, int) __asm__("func_L00_001FD1D8");
extern char D_L05_001672C0_0D678[] __asm__("D_L05_001672C0");
extern float D_L05_001D6B00_0D678[] __asm__("D_L05_001D6B00");
extern float D_L05_001D6B20_0D678[][4] __asm__("D_L05_001D6B20");
extern float D_L05_001D6B60_0D678[][4] __asm__("D_L05_001D6B60");
extern int D_L05_00161DB4_0D678 __asm__("D_L05_00161DB4") MACRO_ADDR;
extern int D_L05_00161DB8_0D678 __asm__("D_L05_00161DB8") MACRO_ADDR;
extern int D_L05_00161DBC_0D678 __asm__("D_L05_00161DBC") MACRO_ADDR;
extern int D_L05_00161DC0_0D678 __asm__("D_L05_00161DC0") MACRO_ADDR;
extern int D_L05_00161DC4_0D678 __asm__("D_L05_00161DC4") MACRO_ADDR;
extern int D_L05_00161DC8_0D678 __asm__("D_L05_00161DC8") MACRO_ADDR;
extern int D_L05_00161DCC_0D678 __asm__("D_L05_00161DCC") MACRO_ADDR;
void func_L05_0030D678_impl(unsigned char *m) __asm__("func_L05_0030D678");

void func_L05_0030D678_impl(unsigned char *m) {
    Quad_0D678 a, b;
    float rotation[4];
    float position[4];
    float matrix[16] __attribute__((aligned(16)));
    long texture, flags;
    float phase, sign, pass;
    int i, j, color;
    float *vertex_a, *vertex_b, *ta_src, *tb_src;
    float *mp;
    int *color_a, *color_b;
    char *vars = *(char **)(m + 0x78);

    f_pos_0D678(m, 0, position);
    f_clear_0D678(rotation);
    rotation[2] = *(float *)(m + 0x48);
    rotation[1] = f_sin_0D678(*(float *)(vars + 0x48)) * 0.5235988f + 0.5235988f;
    f_rot_0D678(matrix, rotation);
    f_sub_0D678(matrix + 4, D_L05_001672C0_0D678, position);
    f_norm_0D678(matrix + 4, matrix + 4, 1.0f);
    f_cross_0D678(matrix, matrix + 4, matrix + 8);
    f_cross_0D678(matrix + 4, matrix + 8, matrix);
    mp = matrix + 12;
    *(u128_0D678 *)mp = *(u128_0D678 *)position;

    texture = f_tex_0D678(D_L05_00161DCC_0D678);
    flags = (long)D_L05_00161DB4_0D678 | ((long)D_L05_00161DB8_0D678 << 2) |
            ((long)D_L05_00161DBC_0D678 << 4) | ((long)D_L05_00161DC0_0D678 << 6) |
            0x8000000000L;
    b.control[1] = texture;
    a.control[1] = texture;
    b.control[2] = 0xFF9000000260L;
    b.control[3] = flags;
    a.control[2] = 0xFF9000000260L;
    a.control[3] = flags;
    b.control[0] = 0;
    a.control[0] = 0;
    {
        float *tb = b.texcoord;
        float *tbv = b.texcoord + 1;
        float *tav = a.texcoord + 1;
        float *ta = a.texcoord;
        float *t = D_L05_001D6B00_0D678;
        int count = 3;
        do {
            float u = t[0];
            float v = t[1];
            *ta = u;
            *tav = v;
            *tb = u;
            *tbv = v;
            t += 2;
            ta += 2;
            tav += 2;
            tb += 2;
            tbv += 2;
        } while (--count >= 0);
    }
    for (i = 0; i < 10; i++) {
        pass = (float)i;
        j = 0;
        ta_src = D_L05_001D6B20_0D678[0];
        tb_src = D_L05_001D6B60_0D678[0];
        vertex_a = a.vertex[0];
        vertex_b = b.vertex[0];
        color_b = b.color;
        color_a = a.color;
        do {
            *(u128_0D678 *)vertex_a = *(u128_0D678 *)ta_src;
            *(u128_0D678 *)vertex_b = *(u128_0D678 *)tb_src;
            ta_src += 4;
            tb_src += 4;
            phase = (float)(i + (j & 1)) * 0.1f;
            sign = 1.0f;
            if (j > 1)
                sign = -1.0f;
            color = f_tween_0D678(D_L05_00161DC4_0D678, D_L05_00161DC8_0D678, phase);
            *color_b++ = color;
            *color_a++ = color;
            vertex_a[0] = vertex_a[0] + phase * 8.0f * sign;
            vertex_a[2] = vertex_a[2] + pass * 10.0f;
            vertex_b[0] += phase * 6.0f * sign;
            vertex_b[2] += pass * 12.0f;
            vertex_a += 4;
            vertex_b += 4;
        } while (++j < 4);
        f_draw_0D678(&a, matrix, 0);
        f_draw_0D678(&b, matrix, 0);
    }
}
