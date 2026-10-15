/* Near miss from src/levels/shared/L01_00397110-g8.c: the build refuses this body at its
   place — 196 bytes, expected 204. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110-g8.c. */
extern float func_002E4460(float, float);

extern void func_002E3470(void *, void *, void *);

extern float D_L01_001B1B24 __attribute__((sda));

extern void func_00306250(char *);

void func_L01_0041C890(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    *(float *)(a0 + 0x2C) = *(float *)(*(char **)(a0 + 0x24) + 0x24) * D_L01_001B1B24;
    *(float *)(s0 + 0x8) = *(float *)(s0 + 0x8) - 0.004000000189989805f;
    func_002E3470(a0 + 0x10, a0 + 0x10, s0);
    func_002E3470(a0 + 0xF0, a0 + 0xF0, s0 + 0x10);
    *(float *)(a0 + 0xF0) = func_002E4460(*(float *)(a0 + 0xF0), *(float *)(s0 + 0x10));
    *(float *)(a0 + 0xF4) = func_002E4460(*(float *)(a0 + 0xF4), *(float *)(s0 + 0x14));
    *(float *)(a0 + 0xF8) = func_002E4460(*(float *)(a0 + 0xF8), *(float *)(s0 + 0x18));

    if (--*(unsigned char *)(a0 + 0xBC) == 0)
        func_00306250(a0);
}
