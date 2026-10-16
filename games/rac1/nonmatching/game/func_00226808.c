/* Sets up the pause-menu helper moby h: spawns class 0x7A5 (func_00226720),
   places it at the D_00187040 position offsets (+0x140, +0x144, +0x148)
   plus 2.2, 0.0 and -1.6, sets its update hook to func_00226D48 and its
   +0x48 to pi, and points its descriptor back at h. Then asks
   func_00226EA8(1) for a buffer; h+0x34 becomes 3 when either the spawn or
   the buffer fails. Returns 0. */
extern void *func_00226720_fill(int) __asm__("func_00226720");
extern int func_00226EA8_fill(int) __asm__("func_00226EA8");
extern void func_00226D48_fill(void) __asm__("func_00226D48");
extern char D_00187040_fill[] __asm__("D_00187040");

int func_00226808(char *h) {
    char *m;
    int r;
    m = func_00226720_fill(0x7A5);
    *(int *)(h + 0x34) = 0;
    if (m != 0) {
        char *d;
        char *g = D_00187040_fill;
        *(char **)(h + 0x44) = m;
        *(short *)(m + 0x34) = 0;
        *(float *)(m + 0x10) = *(float *)(g + 0x140) + 2.2f;
        *(float *)(m + 0x14) = *(float *)(g + 0x144) + 0.0f;
        *(float *)(m + 0x18) = *(float *)(g + 0x148) + -1.6f;
        d = *(char **)(m + 0x78);
        *(void **)(m + 0x74) = (void *)func_00226D48_fill;
        *(float *)(m + 0x48) = 3.1415927f;
        *(char **)d = h;
        *(int *)(d + 4) = 0;
        *(int *)(d + 8) = 0;
    } else {
        *(int *)(h + 0x34) = 3;
    }
    r = func_00226EA8_fill(1);
    *(int *)(h + 0x3C) = r;
    if (r == 0) {
        *(int *)(h + 0x34) = 3;
    }
    return 0;
}
