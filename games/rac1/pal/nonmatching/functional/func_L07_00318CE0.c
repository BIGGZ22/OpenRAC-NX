/* functional: medium. Draws the Umbris rope/sheet: for rows 1..9 and 20 columns builds a quad packet on the stack (four points from the tables at D_L07_0020DEA0 and the row above it 0x140 lower, alpha from the shorts at D_L07_0020DE88, uv scrolling with the two phase floats in sdata) and draws it with func_L00_001FD1D8, wrapped in two GS register writes. Same function as the NTSC decomp's FUN_L07_00317910. Not checked on real calls. */
typedef int U128_18CE0 __attribute__((mode(TI)));
extern int D_0015EF88_18CE0 __asm__("D_0015EF88");
extern float D_L07_00161C04_18CE0 __asm__("D_L07_00161C04");
extern float D_L07_00161C08_18CE0 __asm__("D_L07_00161C08");
extern float D_L07_00161C0C_18CE0 __asm__("D_L07_00161C0C");
extern U128_18CE0 D_L07_0020DEA0_18CE0[] __asm__("D_L07_0020DEA0");
extern short D_L07_0020DE88_18CE0[] __asm__("D_L07_0020DE88");
extern long func_001F4868_18CE0(int) __asm__("func_001F4868");
extern void func_00234C98_18CE0(int, long) __asm__("func_00234C98");
extern void func_L00_001FD1D8_18CE0(void *, void *, int) __asm__("func_L00_001FD1D8");

void func_L07_00318CE0(void) {
    unsigned char pk[0x90] __attribute__((aligned(16)));
    float phase, row_phase, end_phase, low, high;
    int row, col, nextcol, j, alpha, cur;
    unsigned int prev;
    U128_18CE0 *base = D_L07_0020DEA0_18CE0;
    int *color = (int *)(pk + 0x40);
    float *uv = (float *)(pk + 0x50);
    int depth;

    *(long *)(pk + 0x78) = func_001F4868_18CE0(0x15);
    *(long *)(pk + 0x80) = 0xFF9000000260L;
    *(long *)(pk + 0x88) = 0x8000000048L;
    *(long *)(pk + 0x70) = 0;
    phase = D_L07_00161C04_18CE0;
    D_L07_00161C04_18CE0 = phase + 0.001f;
    if (7.0f < D_L07_00161C04_18CE0)
        D_L07_00161C04_18CE0 = D_L07_00161C04_18CE0 - 7.0f;
    func_00234C98_18CE0(0x47, 0x5301BL);
    depth = D_0015EF88_18CE0 >> 13;
    func_00234C98_18CE0(0x4E, (long)depth | 0x1000000L | 0x100000000L);
    phase = phase + 0.3f;
    for (row = 1; row < 10; row++) {
        row_phase = D_L07_00161C08_18CE0;
        D_L07_00161C08_18CE0 = row_phase + D_L07_00161C0C_18CE0;
        if (7.0f < D_L07_00161C08_18CE0)
            D_L07_00161C08_18CE0 = D_L07_00161C08_18CE0 - 7.0f;
        high = phase + 0.3f;
        low = phase - 0.3f;
        for (col = 0; col < 20; col++) {
            nextcol = (col + 1) % 20;
            *(U128_18CE0 *)(pk + 0x00) = *(U128_18CE0 *)((char *)base + row * 0x140 + col * 16);
            *(U128_18CE0 *)(pk + 0x10) = *(U128_18CE0 *)((char *)base + row * 0x140 + nextcol * 16);
            *(U128_18CE0 *)(pk + 0x20) = *(U128_18CE0 *)((char *)base + row * 0x140 - 0x140 + col * 16);
            *(U128_18CE0 *)(pk + 0x30) = *(U128_18CE0 *)((char *)base + row * 0x140 - 0x140 + nextcol * 16);
            prev = (unsigned int)(unsigned short)D_L07_0020DE88_18CE0[row - 1] << 16;
            cur = D_L07_0020DE88_18CE0[row];
            for (j = 1; j >= 0; j--) {
                alpha = cur;
                if (alpha < 0) alpha = 0;
                if (alpha > 0x40) alpha = 0x40;
                color[2 * (1 - j) + 1] = (alpha << 24) | 0x404040;
                color[2 * (1 - j)] = (alpha << 24) | 0x404040;
                cur = (int)prev >> 16;
            }
            if (row == 1) {
                color[3] = 0x20404040;
                color[2] = 0x20404040;
            } else if (row == 9) {
                color[1] = 0x404040;
                color[0] = 0x404040;
            }
            end_phase = row_phase + 0.05f;
            uv[1] = row_phase;
            uv[5] = row_phase;
            uv[0] = phase;
            uv[2] = phase;
            uv[3] = end_phase;
            uv[4] = low;
            uv[6] = low;
            uv[7] = end_phase;
            row_phase = end_phase;
            func_L00_001FD1D8_18CE0(pk, 0, 0);
        }
        phase = high;
    }
    func_00234C98_18CE0(0x4E, (long)(D_0015EF88_18CE0 >> 13) | 0x1000000L);
    func_00234C98_18CE0(0x47, 0x5360BL);
}
