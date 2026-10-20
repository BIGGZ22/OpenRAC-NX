/* 0 of 6 places match - last line: 00_aranos_tutorial: instruction at +0x50 differs.
   Exact size (136 bytes) and 30 of 34 words. Body understood: the value is clamped into
   [1024.0, 4096.0], (int)value - 64 is published to two resident words (0x001B1EB4, 0x001B255C),
   value * 1024.0 to the resident float 0x001BD364, (value - 64.0) * 1024.0 to the small-data
   float, then func_2E4D28 runs; the branch-likely max clamp is reproduced exactly.
   The four words left are the float store's address: retail computes `lui $v1,0x1C` one slot
   before `sub.s` and stores `swc1 $f1,-0x2C9C($v1)`, this cc1 fuses the address into
   `lui $at` at the store point. Statement order, a pointer local and literal addresses all
   give the same four words (four variants measured in one object).
   Lives in src/levels/shared/L00_002E3B38.c (the -O2 -G8 file; draft removed). */

extern float D_001BD364;

extern int D_001B1EB4;

extern int D_001B255C;

extern float D_L00_001ABAE0 __attribute__((sda));

extern void func_0027AB40(void);

void func_L00_002E54E0(float value)
{
    int quantized;

    if (value < 1024.0f)
        value = 1024.0f;
    if (value > 4096.0f)
        value = 4096.0f;
    quantized = (int)value - 0x40;
    D_001BD364 = value * 1024.0f;
    D_001B1EB4 = quantized;
    D_001B255C = quantized;
    D_L00_001ABAE0 = (value - 64.0f) * 1024.0f;
    func_0027AB40();
}
