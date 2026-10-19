/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). corrected from the previous candidate in this file: the small-value test used the bits of the damped result and skipped the store; retail tests |diff| (before damping), stores the damped value when |diff| >= threshold and the undamped diff when it is below. Wave step over the 16x16 ripple grid (scratch a0+0x70, rows of 24 floats; 8 neighbours * 0.25 - old value, minus 1/64 of it) into grid a1 buffer a2 of base a3 (entries 0x1190 bytes, buffers 0x5C0), then copies the border rows/columns/corners into the eight neighbour grids named by bytes 0xC..0x13 of the entry (0xFF: none). */
extern void *memcpy(void *, const void *, unsigned long);
extern float D_L01_001CAF80[] __asm__("D_L01_001CAF80");

void func_L01_00262FE8(char *scratch, int idx, int buf_sel, char *base) {
    union { float f; int i; } v;
    float k_damp = 0.015625f;
    float k_wave = 0.25f;
    float k_one = 1.0f;
    int thresh;
    char *curr = scratch + 0x70;
    char *grid_entry = base + idx * 0x1190;
    char *next_buf = grid_entry + buf_sel * 0x5C0 + 0x50;
    char *src;
    char *target_base;
    int row, col, i;
    unsigned int b;

    v.f = D_L01_001CAF80[7];
    thresh = v.i;

    for (row = 0; row < 16; ++row) {
        for (col = 0; col < 16; ++col) {
            float n0 = *(float *)(curr - 0x64);
            float n1 = *(float *)(curr - 0x60);
            float n2 = *(float *)(curr - 0x5C);
            float n3 = *(float *)(curr - 0x4);
            float n4 = *(float *)(curr + 0x4);
            float n5 = *(float *)(curr + 0x5C);
            float n6 = *(float *)(curr + 0x60);
            float n7 = *(float *)(curr + 0x64);
            float cur_val = *(float *)next_buf;
            float acc, diff, next_val;
            int diff_bits;

            acc = n0 * k_wave;
            acc += n1 * k_wave;
            acc += n2 * k_wave;
            acc += n3 * k_wave;
            acc += n4 * k_wave;
            acc += n5 * k_wave;
            acc += n6 * k_wave;
            acc += n7 * k_wave;
            diff = acc - cur_val * k_one;
            v.f = diff;
            diff_bits = v.i;
            next_val = diff - diff * k_damp;

            if ((diff_bits & 0x7FFFFFFF) - thresh < 0)
                *(int *)next_buf = diff_bits;
            else
                *(float *)next_buf = next_val;

            curr += 4;
            next_buf += 4;
        }
        curr += 0x20;
    }

    src = grid_entry + buf_sel * 0x5C0 + 0x50;
    target_base = base + buf_sel * 0x5C0 + 0x50;

    b = (unsigned char)grid_entry[0xC];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        for (i = 0; i < 4; ++i) {
            memcpy(dst + 0x480 + i * 16, src + i * 16, 16);
            memcpy(dst + 0x500 + i * 16, src + 0x40 + i * 16, 16);
        }
    }

    b = (unsigned char)grid_entry[0xD];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        for (i = 0; i < 16; ++i) {
            *(int *)(dst + 0x4C0 + i * 4) = *(int *)(src + i * 0x40);
            *(int *)(dst + 0x540 + i * 4) = *(int *)(src + 0x4 + i * 0x40);
        }
    }

    b = (unsigned char)grid_entry[0xE];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        for (i = 0; i < 4; ++i)
            memcpy(dst + 0x400 + i * 16, src + 0x3C0 + i * 16, 16);
    }

    b = (unsigned char)grid_entry[0xF];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        for (i = 0; i < 16; ++i)
            *(int *)(dst + 0x440 + i * 4) = *(int *)(src + 0x3C + i * 0x40);
    }

    b = (unsigned char)grid_entry[0x10];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        *(int *)(dst + 0x58C) = *(int *)src;
        *(int *)(dst + 0x590) = *(int *)(src + 0x4);
        *(int *)(dst + 0x594) = *(int *)(src + 0x40);
    }

    b = (unsigned char)grid_entry[0x11];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        *(int *)(dst + 0x588) = *(int *)(src + 0x3C);
    }

    b = (unsigned char)grid_entry[0x12];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        *(int *)(dst + 0x584) = *(int *)(src + 0x3C0);
    }

    b = (unsigned char)grid_entry[0x13];
    if (b != 0xFF) {
        char *dst = target_base + b * 0x1190;
        *(int *)(dst + 0x580) = *(int *)(src + 0x3FC);
    }
}
