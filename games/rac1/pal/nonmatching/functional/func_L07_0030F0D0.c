/* functional: medium. Draw callback of the class 0x419 effect: ten expanding rings of 10 arc segments each, drawn as two quads per step (second one lowered by 5); ported from the asm with the NTSC decomp draft as a guide, checked statically only. */
typedef u32 u128_0F0D0 __attribute__((mode(TI), aligned(16)));
extern long func_001F4868(int);
extern float func_L00_002001D8(void *, float);
extern void func_L00_001FD1D8(void *, int, int);
extern float func_001FA888(int);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern int D_L07_0015F6B0;

void func_L07_0030F0D0_impl(unsigned char *moby) __asm__("func_L07_0030F0D0");
void func_L07_0030F0D0_impl(unsigned char *moby) {
    char packet[0x90] __attribute__((aligned(16)));
    char second[0x90] __attribute__((aligned(16)));
    char phase_work[0x10] __attribute__((aligned(16)));
    int index = 0;
    unsigned int green = 0x80;
    float band = 0.0f;
    unsigned int alpha;
    char *data = *(char **)(moby + 0x78);
    unsigned int green_step = (green / 11) & 0xFF;
    float radius_threshold = 0.9f;
    float inner_scale = 0.96f;
    alpha = (unsigned int)moby[0x23] / 5u;
    *(long *)(packet + 0x78) = func_001F4868(*(int *)(data + 0x18));
    *(long *)(packet + 0x80) = 0xFF9000000260L;
    *(long *)(packet + 0x70) = 4;
    *(long *)(packet + 0x88) = 0x8000000048L;
    do {
        float radius, side, center, angle, inner_radius, width, phase, ft;
        int color = ((alpha & 0xFF) << 24) | ((green << 8) | 0x80) | (green << 16);
        *(int *)(packet + 0x40) = color;
        *(int *)(packet + 0x4C) = color;
        *(int *)(packet + 0x48) = color;
        *(int *)(packet + 0x44) = color;
        if (*(float *)(data + 8) < radius_threshold)
            radius = *(float *)(data + 8) * band;
        else
            radius = *(float *)(data + 8) - band * radius_threshold;
        side = (1.0f - band * 0.5f) * *(float *)(data + 0x10);
        ft = func_001FA888(D_L07_0015F6B0);
        phase = func_L00_002001D8(phase_work, ft / (band * 40.0f + 40.0f));
        if ((index & 1) == 0)
            phase = -phase;
        *(float *)(packet + 0x60) = phase;
        *(float *)(packet + 0x50) = phase;
        width = func_001FA888((index + 1) < 6 ? index + 1 : 5);
        green = green - green_step;
        *(float *)(packet + 0x6C) = 1.0f;
        *(float *)(packet + 0x68) = *(float *)(packet + 0x58) = *(float *)(packet + 0x50) + width;
        center = *(float *)(moby + 0x10);
        *(float *)(packet + 0x34) = *(float *)(packet + 0x14) = *(float *)(moby + 0x14);
        *(float *)(packet + 0x30) = center - radius;
        *(float *)(packet + 0x5C) = 0.0f;
        *(float *)(packet + 0x08) = side + *(float *)(moby + 0x18);
        *(float *)(packet + 0x10) = center - radius * (inner_scale - band * 0.03f);
        *(float *)(packet + 0x38) = *(float *)(moby + 0x18);
        *(float *)(packet + 0x28) = *(float *)(moby + 0x18);
        *(float *)(packet + 0x18) = side + *(float *)(moby + 0x18);
        *(float *)(packet + 0x54) = 0.0f;
        *(float *)(packet + 0x64) = 1.0f;
        {
            char *sp = packet;
            char *dp = second;
            do {
                *(u128_0F0D0 *)dp = *(u128_0F0D0 *)sp;
                *(u128_0F0D0 *)(dp + 16) = *(u128_0F0D0 *)(sp + 16);
                sp += 32;
                dp += 32;
            } while (sp != packet + 0x80);
            *(u128_0F0D0 *)dp = *(u128_0F0D0 *)sp;
        }
        *(float *)(second + 8) = *(float *)(moby + 0x18) - 5.0f;
        *(float *)(second + 0x18) = *(float *)(second + 8);
        angle = -2.8274336f;
        do {
            float x, y, rr;
            *(u128_0F0D0 *)(packet + 0) = *(u128_0F0D0 *)(packet + 0x10);
            *(u128_0F0D0 *)(packet + 0x20) = *(u128_0F0D0 *)(packet + 0x30);
            x = func_001F9F90(angle);
            rr = band * 0.03f;
            inner_radius = radius * (inner_scale - rr);
            *(float *)(packet + 0x10) = *(float *)(moby + 0x10) + inner_radius * x;
            x = func_001F9F90(angle);
            *(float *)(packet + 0x30) = *(float *)(moby + 0x10) + radius * x;
            y = func_001F9FA8(angle);
            *(float *)(packet + 0x14) = *(float *)(moby + 0x14) + inner_radius * y;
            y = func_001F9FA8(angle);
            angle = angle + 0.31415927f;
            *(float *)(packet + 0x34) = *(float *)(moby + 0x14) + radius * y;
            func_L00_001FD1D8(packet, 0, 1);
            *(u128_0F0D0 *)(second + 0) = *(u128_0F0D0 *)(second + 0x10);
            *(u128_0F0D0 *)(second + 0x20) = *(u128_0F0D0 *)(second + 0x30);
            *(float *)(second + 0x30) = *(float *)(packet + 0x30);
            *(float *)(second + 0x34) = *(float *)(packet + 0x34);
            *(float *)(second + 0x10) = *(float *)(packet + 0x10);
            *(float *)(second + 0x14) = *(float *)(packet + 0x14);
            func_L00_001FD1D8(second, 0, 1);
        } while (angle < 3.1415927f);
        index++;
        band = band + 0.1f;
        green &= 0xFF;
    } while (band < 1.0f);
}
