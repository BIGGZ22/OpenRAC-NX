#include "include_asm.h"

extern int func_00282948(void *);
extern void func_002B9180(void *);
extern float func_00283B98(float, float);
extern int func_00283CF0(float);
extern int D_001AA304 __attribute__((sda));

void func_002BBA38(char *particle)
{
    char *state = particle + 32;
    int alpha;
    unsigned int shifted;
    int color;
    if (func_00282948(state)) {
        func_002B9180(particle);
    } else {
        *(float *)(state + 12) = func_00283B98(*(float *)(state + 12), *(float *)(state + 8));
        particle[8] = func_00283CF0(*(float *)(state + 12) * 40.7436637878418f) - 128;
        if (*(int *)(particle + 32) > D_001AA304 * 3 / 4)
            alpha = *(int *)(state + 16) * (D_001AA304 - *(int *)(particle + 32)) / (D_001AA304 / 4);
        else if (*(int *)(particle + 32) < D_001AA304 / 4)
            alpha = *(int *)(state + 16) * *(int *)(particle + 32) / (D_001AA304 / 4);
        else
            alpha = *(int *)(state + 16);
        shifted = (unsigned int)alpha << 24;
        switch ((unsigned char)particle[3]) {
        case 72:
            color = shifted + 0x004f4f4f;
            break;
        default:
            color = shifted + 0x007f7f7f;
            break;
        }
        *(int *)(particle + 4) = color;
    }
}
