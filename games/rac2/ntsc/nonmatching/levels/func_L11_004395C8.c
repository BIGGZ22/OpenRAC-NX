/* Near miss from src/levels/11_joba/L11_00432628-g8.c: the build refuses this body at its
   place — instruction at +0x4 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/11_joba/L11_00432628-g8.c. */
typedef unsigned char      u8;

extern float D_L11_001B12C8 __attribute__((sda));

void func_L11_004395C8(char *p)
{
    char *q = *(char **)(p + 0x68);
    u8 *armed = (u8 *)(q + 0x380);
    float *timer = (float *)(q + 0x2F0);
    int i;

    for (i = 9; i >= 0; i--) {
        if (*armed == 1) {
            float v = *timer - D_L11_001B12C8 * 0.016666668f;

            *timer = v;
            if (v < 0.0f) {
                *timer = 0.0f;
                *armed = 2;
            }
        }
        timer++;
        armed++;
    }
}
