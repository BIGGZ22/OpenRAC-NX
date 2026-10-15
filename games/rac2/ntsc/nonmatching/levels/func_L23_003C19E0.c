/* Near miss from src/levels/23_damosel_orbit/L23_0035CE90-g8.c: the build refuses this body at its
   place — 200 bytes, expected 204. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/23_damosel_orbit/L23_0035CE90-g8.c. */
extern int D_L23_001B36B0;

extern float D_L23_001AFA34 __attribute__((sda));

extern char D_00189EA0[];

extern void func_2F8190(char *out, char *in, char *base);

extern void func_2F8440(char *out, char *in, float scale);

extern float func_2F8268(char *out, char *in);

int func_L23_003C19E0(char *arg0, float *arg1) {
    char buf[16];

    if (*(unsigned char *)(arg0 + 0x20) != 8)
        return 0;
    if (D_L23_001B36B0)
        return 0;
    if (*(unsigned char *)(arg0 + 0x31) == 0)
        return 0;
    if (*(short *)(*(char **)(arg0 + 0x68) + 0x324) == 0)
        return 0;
    func_2F8190(buf, arg0 + 0x10, D_00189EA0);
    func_2F8440(buf, buf, 1.0f);
    if (func_2F8268(buf, (char *)(*(int *)(D_00189EA0 + 0x2210) + 0xC0)) <= D_L23_001AFA34) {
        *arg1 = *(float *)(*(char **)(arg0 + 0x68) + 0x27C);
        return 1;
    }
    return 0;
}
