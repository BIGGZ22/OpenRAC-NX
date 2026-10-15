/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: the build refuses this body at its
   place — 184 bytes, expected 180. Kept as a starting point;
   not linked, not counted. */

/* Declaration recovered from nonmatching/levels/func_L20_0044AA88.c. */
extern void func_L20_0044AA88(char *arg0);


/* Standalone declarations recovered from src/levels/20_yeedil/L20_00449178-g8.c. */
typedef unsigned char u8;

typedef unsigned short u16;

typedef unsigned int u32;

typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B2410 __attribute__((sda));

extern float D_L20_001B2414 __attribute__((sda));

extern float D_L20_001B241C __attribute__((sda));

extern float func_00283B98(float, float);

extern float func_00283198(float);

extern void func_002A9288(char *, float);

void func_L20_0044A838(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    u32 timer;
    float r;

    timer = *(u16 *)(arg0 + 0x96);
    if (timer <= 0xFFFE) *(u16 *)(arg0 + 0x96) = (u16)(timer + 1);
    if (*(u8 *)(arg0 + 0x31) != 0) {
        *(TI128 *)(arg0 + 0x10) = *(TI128 *)(state + 0x180);
        r = func_00283198(func_00283B98(*(float *)(state + 0x1A0),
                                        D_L20_001B2414 * 0.000290888245f));
        *(float *)(state + 0x1A0) = r;
        *(float *)(arg0 + 0x18) = *(float *)(arg0 + 0x18) + D_L20_001B2410 * r;
        func_002A9288(arg0, D_L20_001B241C);
    }
    if ((*(u16 *)(arg0 + 0x34) & 1) == 0) func_L20_0044AA88(arg0);
}
