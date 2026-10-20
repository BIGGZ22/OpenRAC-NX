/* Correct pointer argument changes allocation at +0x60; original unit 0 of 1 places.
 * Requires declarations/types from src/levels/26_jamming_array/L26_002EC840.c. */
void func_L26_003DC048(unsigned char *a0) {
    void *s0 = *(void **)(a0 + 0x68);
    unsigned short v;
    float f;

    func_00282978((short *)(s0 + 0x220));
    func_00282978((short *)(s0 + 0x222));
    func_00325DA0(a0, s0 + 0x278);
    if (a0[0x95] == 2) {
        f = func_00283B98(*(float *)(s0 + 0x27C), 0.20943955f);
        *(float *)(s0 + 0x27C) = f;
        func_00325C50(f, a0, s0 + 0x270, 1, D_L26_001A8F20);
    } else {
        f = func_00283B98(*(float *)(s0 + 0x27C), 0.011635531f);
        *(float *)(s0 + 0x27C) = f;
        func_00325C50(f, a0, s0 + 0x270, 0, D_L26_001A8F20);
    }
    v = *(unsigned short *)(a0 + 0x96);
    if (v < 0xFFFF)
        *(unsigned short *)(a0 + 0x96) = v + 1;
}
