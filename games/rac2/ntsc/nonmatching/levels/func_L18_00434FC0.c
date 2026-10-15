/* Near miss from src/levels/18_damosel/L18_00433460.c: the build refuses this body at its
   place — instruction at +0xa8 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_00433460.c. */
typedef int s32;

typedef unsigned char u8;

extern float D_L18_001B97C0[];

extern float func_002A7878(float, float);

extern void func_0029FCC0(void *, int, void *);

extern void func_00282CF0(float *, const float *, const float *);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern int func_002A7820(int, int);

extern float func_002A7940(void);

extern void func_L02_003FFC48(void *, int, int, int, int, float, float);

void func_L18_00434FC0(u8 *obj) {
    float v[4];
    float w[4];
    float a;
    s32 id;

    a = func_002A7878(0.5f, 0.8f);
    func_0029FCC0(obj, 0, v);
    a = a * 0.5f;
    func_00282CF0(w, D_L18_001B97C0, v);
    w[2] = 0.0f;
    func_00282F20((char *)w, (char *)w, 0.2f);
    func_00282CC0((char *)v, (char *)v, (char *)w);
    id = func_002A7820(0x50, 0x60);
    func_L02_003FFC48(v, 0x807070, id, 0x31, 2, a, func_002A7940());
    id = func_002A7820(0x16, 0x32);
    func_L02_003FFC48(v, 0x802020, id, 0xB, 2, 0.5600000023841858f, func_002A7940());
}
