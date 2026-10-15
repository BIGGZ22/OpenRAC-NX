/* Near miss from src/levels/18_damosel/L18_003F80A0-g8.c: the build refuses this body at its
   place — 216 bytes, expected 220. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_003F80A0-g8.c. */
typedef unsigned char u8;

typedef int s32;

extern void func_0029FC20(s32 arg0, s32 arg1, void *arg2);

typedef int TI128 __attribute__((mode(TI)));

extern void func_00308DE0(void *arg0, void *arg1, void *arg2);

extern void func_003088F0(void *arg0, void *arg1, void *arg2);

extern void func_00308920(void *arg0, void *arg1, void *arg2);

extern void func_00308BD8(void *arg0, void *arg1, float arg2);

extern float func_003089E0(void *arg0, void *arg1);

extern s32 func_00337AD8(s32 arg0, s32 arg1, float arg2);

extern void func_00340828(void *arg0, s32 arg1, float arg2, float arg3);

extern int D_L18_001B15F0 __attribute__((sda));

extern float D_L18_001B15F4 __attribute__((sda));

extern float D_L18_001B15F8 __attribute__((sda));

extern u8 D_L18_001B1600[] __attribute__((sda));

extern float D_L18_001B1610 __attribute__((sda));

extern u8 D_001C97C0[];

void func_L18_00433088(u8 *a0, s32 a1) {
    u8 buf[0x70];
    float t;

    func_0029FC20(*(s32 *)(*(u8 **)(a0 + 0x68) + 0x7C4), a1, buf + 0x10);
    func_00308DE0(buf, D_L18_001B1600, buf + 0x10);
    func_003088F0(buf, buf, buf + 0x40);
    func_00308920(buf + 0x50, D_001C97C0, buf);
    func_00308BD8(buf + 0x50, buf + 0x50, 1.0f);
    *(TI128 *)(buf + 0x60) = *(TI128 *)(buf + 0x10);
    t = func_003089E0(buf + 0x50, buf + 0x60);
    if (D_L18_001B1610 < t) {
        func_00340828(buf, func_00337AD8(D_L18_001B15F0 & 0xFFFFFF,
                                         D_L18_001B15F0, t * t),
                      D_L18_001B15F8, D_L18_001B15F4);
    }
}
