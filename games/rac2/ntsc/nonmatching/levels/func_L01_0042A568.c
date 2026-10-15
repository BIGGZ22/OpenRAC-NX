/* Near miss from src/levels/shared/L01_00397110.c: the build refuses this body at its
   place — 372 bytes, expected 380. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110.c. */
extern char *D_001B9510;

extern float func_2E3618(char *);

int func_L01_0042A568(void)
{
    char *s1 = *(char **)(D_001B9510);
    char *v1 = *(char **)(s1 + 0x70);
    unsigned char a = *(unsigned char *)(s1 + 0x88);
    int s0 = 0;
    float f;

    if (a == 2 || a == 3) {
        s0 = *(unsigned int *)(v1 + 0xE0) < 1;
    } else if (a != 1) {
        if (*(float *)(v1 + 0x0) < 0.001f && *(float *)(v1 + 0x4) < 0.001f &&
            *(float *)(v1 + 0x8) < 0.001f && *(float *)(v1 + 0x30) < 0.0001f &&
            *(float *)(v1 + 0x34) < 0.0001f && *(float *)(v1 + 0x38) < 0.0001f) {
            f = func_2E3618(s1 + 0x30);

            if (f < 0.01f)
                s0 = 1;
            else if (*(int *)(v1 + 0x120) == 0)
                return s0;
            else if (0.1f < f)
                s0 = 4;
        }
    }

    if (*(int *)(v1 + 0x120) != 0) {
        if (*(unsigned char *)(s1 + 0x89) != 0)
            s0 |= 2;
    }

    return s0;
}
