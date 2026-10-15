/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: the build refuses this body at its
   place — 300 bytes, expected 308. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_00449178-g8.c. */
typedef unsigned char u8;

typedef unsigned short u16;

typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B23F8 __attribute__((sda));

extern void *func_0029EFD0(int);

extern void func_002B37E8(u8 *, u8 *);

extern float func_002A7940(void);

extern void func_002833F0(void *, void *);

extern float func_002A7878(float a, float b);

void *func_L20_00449FD0(char *arg0, void *arg1, void *arg2, int arg3, float f12,
                        float f13) {
    char *obj = func_0029EFD0(0x12EE);
    char *state;

    if (obj != 0) {
        func_002B37E8((u8 *)obj, (u8 *)arg0);
        *(TI128 *)(obj + 0x10) = *(TI128 *)arg1;
        *(float *)(obj + 0xF0) = func_002A7940();
        *(float *)(obj + 0xF4) = func_002A7940();
        func_002833F0(obj + 0xC0, obj + 0xF0);
        *(u8 *)(obj + 0x20) = 1;
        *(u16 *)(obj + 0x34) |= 0x100;
        *(float *)(obj + 0x2C) = *(float *)(*(char **)(obj + 0x24) + 0x24)
            * D_L20_001B23F8;
        state = *(char **)(obj + 0x68);
        *(TI128 *)state = *(TI128 *)arg2;
        *(int *)(state + 0x18) = arg3;
        *(float *)(state + 0x14) = f12 * 0.016666668f;
        *(float *)(state + 0x1C) = f13;
        *(int *)(state + 0x20) = 0x3C;
        *(float *)(state + 0x10) = func_002A7878(20.0f, 20.0f) * 0.00027777778f;
    }
    return obj;
}
