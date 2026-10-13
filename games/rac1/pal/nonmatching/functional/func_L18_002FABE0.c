/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L18_002f9780), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: PAL's compiler gives our 784 bytes for retail's 792 bytes. */
extern short D_L18_00162528;
extern float D_L18_001625B0 MACRO_ADDR;
extern char D_L18_00167A50[];
extern int func_001F9850(int);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float D_L18_001625E0;
extern volatile float D_L18_001625E4 MACRO_ADDR;
extern volatile float D_L18_001625E8 MACRO_ADDR;
extern float D_L18_001625EC MACRO_ADDR;
extern short D_L18_00162588;
extern short D_L18_0016258C;
typedef unsigned int u128_9780 __attribute__((mode(TI)));
typedef struct {
    float m[4][4];
    int col[4];
    float uv[8];
    unsigned long a, b, c, d;
} Pk_9780;
typedef struct { char pad[0x30]; int state; int n; } ML_9780;
extern ML_9780 D_L18_0016D2E0_FABE0 __asm__("D_L18_0016D2E0");
extern float D_L18_001DFE50[4][4];
extern unsigned long func_001F4868(int);
extern void func_00234C98(int, long);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9EE8(void *, void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l18/gameplay/vendor/002f88e8.c, FUN_L18_002f9780. */
void func_L18_002FABE0(void) {
    Pk_9780 pk;
    float mat[4][4];
    float up[4];
    int i;
    int color;
    float tilt;
    char *target;

    if (D_L18_0016D2E0_FABE0.n == func_001F9850((*(int *)&D_L18_00162528) + 1)) {
        D_L18_001625E0 = 1.0f;
        D_L18_001625E4 = 0.0f;
        if (D_L18_0016D2E0_FABE0.state == 7) {
            tilt = -0.15f;
        } else {
            tilt = 0.15f;
        }
        D_L18_001625E8 = tilt;
        D_L18_001625EC = 1.0f;
    }
    pk.b = func_001F4868(0xB);
    pk.c = 0x0000FF9000000260ULL;
    pk.d = 0x8000000048ULL;
    pk.a = 0;
    func_00234C98(0x4A, 0);
    func_00234C98(0x47, 0x51001);
    func_001F9EC0(mat[3], &D_L18_001625E0, D_L18_00167A50);
    target = D_L18_00167A50;
    target -= 0x210;
    func_001F9BD8(mat[3], mat[3], target);
    func_001F9BF0(mat[0], mat[3], target);
    func_L00_001FF4B0(mat[0], mat[0], 1.0f);
    *(u128_9780 *)up = 0;
    up[2] = 1.0f;
    up[3] = 1.0f;
    func_001F9CA0(mat[1], mat[0], up);
    func_L00_001FF4B0(mat[1], mat[1], 1.0f);
    func_001F9CA0(mat[2], mat[1], mat[0]);
    pk.uv[0] = 1.0f;
    pk.uv[1] = 1.0f;
    pk.uv[2] = 0.0f;
    pk.uv[3] = 1.0f;
    pk.uv[4] = 1.0f;
    pk.uv[5] = 0.0f;
    pk.uv[6] = 0.0f;
    pk.uv[7] = 0.0f;
    color = (*(int *)&D_L18_00162588);
    pk.col[3] = color;
    pk.col[2] = color;
    pk.col[1] = color;
    pk.col[0] = color;
    for (i = 0; i < 4; i++) {
        func_001F9C30(pk.m[i], D_L18_001DFE50[i], D_L18_001625B0);
        func_001F9EE8(pk.m[i], pk.m[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);
    color = (*(int *)&D_L18_0016258C);
    pk.col[3] = color;
    pk.col[2] = color;
    pk.col[1] = color;
    pk.col[0] = color;
    for (i = 0; i < 4; i++) {
        func_001F9C30(pk.m[i], D_L18_001DFE50[i], D_L18_001625B0 * 0.75f);
        func_001F9EE8(pk.m[i], pk.m[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);
    for (i = 0; i < 4; i++) {
        func_001F9C30(pk.m[i], D_L18_001DFE50[i], D_L18_001625B0 * 0.5f);
        func_001F9EE8(pk.m[i], pk.m[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);
}
