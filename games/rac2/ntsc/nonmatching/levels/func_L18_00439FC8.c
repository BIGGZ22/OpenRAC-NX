/* Near miss from src/levels/18_damosel/L18_00433460.c: the build refuses this body at its
   place — instruction at +0xec differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_00433460.c. */
typedef int s32;

typedef unsigned char u8;

extern float func_00283B98(float, float);

extern int D_L18_001C9D40;

extern s32 func_002E59B0(void *, s32);

extern s32 func_002E5CF8(s32, s32, void *);

extern void *func_002A9640(void *, s32, s32);

extern float func_002A96A0(void *, void *);

extern void func_002AB800(void *, void *);

extern void func_002AB8C0(void *, void *);

extern int func_002A7750(void *, int, int);

void func_L18_00439FC8(u8 *obj) {
    u8 *state;
    u8 *p;
    float f;

    if (D_L18_001C9D40 != 0) return;
    state = *(u8 **)(obj + 0x68);
    switch (*(u8 *)(obj + 0x20)) {
    case 0:
        *(short *)(state + 0x24) = 0x12;
        *(float *)(state + 0x20) = 18.0f;
        func_002A7750(obj, 1, -1);
        return;
    case 1:
        if (func_002E59B0(obj, *(s32 *)(state + 0x68)) == 0) {
            *(s32 *)(state + 0x68) = func_002E5CF8(0, 4, obj);
        }
        p = state + 0x50;
        *(float *)(obj + 0xF8) = func_00283B98(*(float *)(obj + 0xF8),
                                               *(float *)(state + 0x64) * 0.000290888245f);
        f = func_002A96A0(obj, func_002A9640(obj, 0x230000, 0));
        if (f > 0.0f) {
            *(u8 *)(state + 0x57) = 0xFF;
            *(u8 *)(state + 0x58) = 0x40;
            *(u8 *)(state + 0x59) = 0;
            func_002AB800(obj, p);
            *(float *)(state + 0x20) = *(float *)(state + 0x20) - f;
            func_002AB8C0(obj, p);
        }
        *(u8 *)(obj + 0xA8) = 0xFF;
        return;
    }
}
