/* functional: medium. Draw callback of a lens-flare style effect: builds four orientation matrices pointing from the moby toward the camera (the last two rotated by pvars angles), fills four textured quads (colour of the first tweened by pvars scale) and draws each. Statically checked only. */
typedef u32 u128_EB5E8 __attribute__((mode(TI), aligned(16)));
typedef struct {
    float corner[4][4];
    unsigned int color[4];
    float uv[4][2];
    long zero, texture, state, flags;
} Packet_EB5E8;
extern void f_clear_EB5E8(void *) __asm__("func_001F9BC0");
extern float f_len_EB5E8(float, float) __asm__("func_L00_001FF860");
extern float f_dist_EB5E8(void *, void *) __asm__("func_001F9D48");
extern void f_copy_EB5E8(void *, void *) __asm__("func_001FA218");
extern void f_scale_EB5E8(void *, void *, float) __asm__("func_001F9C30");
extern long f_tex_EB5E8(int) __asm__("func_001F4868");
extern int f_tween_EB5E8(int, int, float) __asm__("func_001FA8A8");
extern void f_draw_EB5E8(void *, void *, int) __asm__("func_L00_001FD1D8");
extern char D_L18_00167700_EB5E8[] __asm__("D_L18_00167700");
extern float D_L18_001D9E20_EB5E8[][2] __asm__("D_L18_001D9E20");
extern float D_L18_001D9E40_EB5E8[][4] __asm__("D_L18_001D9E40");
extern int D_L18_00161F74_EB5E8 __asm__("D_L18_00161F74") MACRO_ADDR;
extern int D_L18_00161F78_EB5E8 __asm__("D_L18_00161F78") MACRO_ADDR;
extern int D_L18_00161F7C_EB5E8 __asm__("D_L18_00161F7C") MACRO_ADDR;
extern int D_L18_00161F80_EB5E8 __asm__("D_L18_00161F80") MACRO_ADDR;
extern int D_L18_00161F88_EB5E8[] __asm__("D_L18_00161F88") MACRO_ADDR;
extern int D_L18_00161F98_EB5E8[] __asm__("D_L18_00161F98") MACRO_ADDR;
void func_L18_002EB5E8_impl(unsigned char *moby) __asm__("func_L18_002EB5E8");

void func_L18_002EB5E8_impl(unsigned char *moby) {
    Packet_EB5E8 packet[4];
    float axis[4];
    float basis[4][16];
    float *data = *(float **)(moby + 0x78);
    float *view = (float *)(D_L18_00167700_EB5E8 + 0x140);
    void *pos = moby + 0x10;
    int i;
    int j;
    float *corner;
    float x, y;

    f_clear_EB5E8(axis);
    x = *(float *)(moby + 0x10);
    y = *(float *)(moby + 0x14);
    x = view[0] - x;
    y = view[1] - y;
    axis[2] = f_len_EB5E8(x, y);
    axis[1] = -f_len_EB5E8(f_dist_EB5E8(pos, view), view[2] - *(float *)(moby + 0x18));
    f_copy_EB5E8(&basis[0][0], axis);
    f_copy_EB5E8(&basis[1][0], axis);
    f_scale_EB5E8(&basis[1][4], &basis[1][4], data[9]);
    f_scale_EB5E8(&basis[1][8], &basis[1][8], data[9]);
    *(u128_EB5E8 *)&basis[0][12] = *(u128_EB5E8 *)pos;
    *(u128_EB5E8 *)&basis[1][12] = *(u128_EB5E8 *)pos;
    axis[0] = data[11];
    f_copy_EB5E8(&basis[2][0], axis);
    f_scale_EB5E8(&basis[2][4], &basis[2][4], data[9]);
    f_scale_EB5E8(&basis[2][8], &basis[2][8], data[9]);
    *(u128_EB5E8 *)&basis[2][12] = *(u128_EB5E8 *)pos;
    axis[0] = data[12];
    f_copy_EB5E8(&basis[3][0], axis);
    f_scale_EB5E8(&basis[3][4], &basis[3][4], data[9]);
    f_scale_EB5E8(&basis[3][8], &basis[3][8], data[9]);
    *(u128_EB5E8 *)&basis[3][12] = *(u128_EB5E8 *)pos;
    i = 0;
    do {
        packet[i].texture = f_tex_EB5E8(D_L18_00161F98_EB5E8[i]);
        packet[i].state = 0xFF9000000260L;
        packet[i].flags = (long)D_L18_00161F74_EB5E8 | ((long)D_L18_00161F78_EB5E8 << 2) |
                          ((long)D_L18_00161F7C_EB5E8 << 4) | ((long)D_L18_00161F80_EB5E8 << 6) |
                          0x8000000000L;
        packet[i].zero = 0;
        corner = D_L18_001D9E40_EB5E8[i * 4];
        j = 0;
        do {
            packet[i].uv[j][0] = D_L18_001D9E20_EB5E8[j][0];
            packet[i].uv[j][1] = D_L18_001D9E20_EB5E8[j][1];
            if (i == 0) {
                packet[i].color[j] = f_tween_EB5E8(D_L18_00161F88_EB5E8[i] & 0xFFFFFF,
                                                   D_L18_00161F88_EB5E8[i], data[9]);
            } else {
                packet[i].color[j] = D_L18_00161F88_EB5E8[i];
            }
            *(u128_EB5E8 *)packet[i].corner[j] = *(u128_EB5E8 *)corner;
            j++;
            corner += 4;
        } while (j < 4);
        i++;
    } while (i < 4);
    f_draw_EB5E8(&packet[0], &basis[0][0], 0);
    f_draw_EB5E8(&packet[1], &basis[1][0], 0);
    f_draw_EB5E8(&packet[2], &basis[2][0], 0);
    f_draw_EB5E8(&packet[3], &basis[3][0], 0);
}
