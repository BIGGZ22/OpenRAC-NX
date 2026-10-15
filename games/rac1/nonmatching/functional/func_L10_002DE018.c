/* functional: medium. Update of a rippling cloth/water grid moby (4 groups of 10 columns of 20 rows): state 0 sets up per-column timers (0x2D per step) unless on level 10 with flag set (then deletes itself); state 1 advances a phase, and for each column whose timer function says so rebuilds the wave rows and relaxes neighbouring rows, otherwise just relaxes. Statically checked only. */
extern void f_delete_DE018(void *) __asm__("func_0020D678");
extern int f_timer_DE018(void *) __asm__("func_001F9908");
extern void f_clear_DE018(void *) __asm__("func_001F9BC0");
extern void f_wave_DE018(float, float, void *) __asm__("func_L00_00258DB0");
extern void f_sub_DE018(void *, void *, void *) __asm__("func_001F9BD8");
extern void f_scale_DE018(void *, void *, float) __asm__("func_001F9C30");
extern unsigned char D_0013E633_DE018[] __asm__("D_0013E633");
extern int D_0015EE84_DE018 __asm__("D_0015EE84") MACRO_ADDR;
extern float D_0015EE6C_DE018 __asm__("D_0015EE6C") MACRO_ADDR;
extern int D_L10_00161DE4_DE018 __asm__("D_L10_00161DE4");
extern int D_L10_00161DE8_DE018 __asm__("D_L10_00161DE8");
extern float D_L10_00161DD4_DE018 __asm__("D_L10_00161DD4") MACRO_ADDR;
extern float D_L10_00161DD8_DE018 __asm__("D_L10_00161DD8") MACRO_ADDR;
extern float D_L10_00161DDC_DE018 __asm__("D_L10_00161DDC") MACRO_ADDR;
void func_L10_002DE018_impl(unsigned char *moby) __asm__("func_L10_002DE018");

void func_L10_002DE018_impl(unsigned char *moby) {
    char *data = *(char **)(moby + 0x78);
    float scratch[0xC80 / 4];
    int group, item;
    float phase;

    switch (moby[0x20]) {
    case 0:
        if (D_0015EE84_DE018 == 10 && D_0013E633_DE018[0x2EC1] == 1) {
            f_delete_DE018(moby);
        } else {
            int value = 0;
            long increment = 45;
            D_L10_00161DE4_DE018 = 0x2D;
            D_L10_00161DE8_DE018 = 0x16;
            for (item = 0; item < 10; item++) {
                int quotient = value / 10;
                *(int *)(data + 0x12C00 + item * 4) = quotient;
                *(int *)(data + 0x12C78 + item * 4) = quotient + 11;
                *(int *)(data + 0x12CF0 + item * 4) = quotient + 23;
                *(int *)(data + 0x12D68 + item * 4) = quotient + 34;
                value += increment;
            }
            moby[0x20] = 1;
        }
        break;
    case 1:
        phase = *(float *)(data + 0x12DE0) + D_L10_00161DDC_DE018 * D_0015EE6C_DE018;
        *(float *)(data + 0x12DE0) = phase;
        if (phase > 1.0f)
            *(float *)(data + 0x12DE0) = phase - 1.0f;
        {
            int group_table = 0x12C00;
            for (group = 0; group < 4; group++) {
                int group_offset = group * 0x2580;
                int timer_offset = group * 0x78;
                for (item = 0; item < 10; item++) {
                    char *entry = data + (timer_offset + group_table) + item * 4;
                    char *base = data + group_offset;
                    char *other = data + (group_offset + 0x9600);
                    char *buffer = (char *)scratch + item * 0x10;
                    if (f_timer_DE018(entry) != 0) {
                        char *dst = data + (item * 0x10 + group_offset);
                        char *current = base + item * 0x10;
                        char *out = buffer;
                        int row;
                        for (row = 0; row < 20; row++) {
                            float ph, value, quotient, half;
                            f_clear_DE018(current);
                            current += 0x140;
                            ph = D_L10_00161DD8_DE018;
                            value = D_L10_00161DD4_DE018 * D_0015EE6C_DE018;
                            quotient = ph / 20.0f;
                            half = ph * 0.5f;
                            *(float *)(dst + 0xC) = 1.0f;
                            *(float *)(dst + 4) = (float)row * quotient - half;
                            f_wave_DE018(0.0f, value, out);
                            dst += 0x140;
                            out += 0xA0;
                        }
                        {
                            char *dst2 = other + item * 0x10 + 0x140;
                            char *cur2 = buffer;
                            char *out2 = buffer + 0x140;
                            for (row = 17; row >= 0; row--) {
                                f_sub_DE018(dst2, cur2, cur2 + 0xA0);
                                f_sub_DE018(dst2, dst2, out2);
                                f_scale_DE018(dst2, dst2, 0.333f);
                                dst2 += 0x140;
                                cur2 += 0xA0;
                                out2 += 0xA0;
                            }
                        }
                        *(int *)entry = D_L10_00161DE4_DE018;
                    } else {
                        char *dst = base + item * 0x10 + 0x140;
                        char *out = other + item * 0x10 + 0x140;
                        int row;
                        for (row = 17; row >= 0; row--) {
                            f_sub_DE018(dst, dst, out);
                            dst += 0x140;
                            out += 0x140;
                        }
                    }
                }
            }
        }
        break;
    }
}
