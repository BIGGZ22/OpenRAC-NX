/* 72 bytes, exact in 23 of 27 places: the four others (16_snivelak,
   22_dobbo_orbit, 23_damosel_orbit, 26_jamming_array) hold other constants, so no one
   source form reaches them - retail uses q+0x20 with fields +0x1C/+0x20 there, and
   q+0x60 with +0xE4/+0xE8 in the 23 others. The placement then drops the body
   everywhere and every program still prints OK, so the loss only shows as
   "1 do not match" on the build line. ref_LNN_XXXXXXXX_kk cannot name these:
   it gives a symbol value, not an immediate constant. */
extern char *D_L00_001B8E10;

void func_L00_00424BB8(float a, float b)
{
    char *p = D_L00_001B8E10;

    if (*(short *)(p + 134) == 0) {
        char *q = *(char **)(p + 112);
        char *r = q + 0x60;

        if (a != 0.0f) {
            *(float *)(r + 228) = a;
        }
        if (b != 0.0f) {
            *(float *)(r + 232) = b;
        }
    }
}

