# func_001EE3B0 (effects.c) — glow sprite layouts, size matches, ~165-184/796 bytes

Levers found: tex as `long` from func_001F4868 (avoids an int->long copy), block-scoped loop i.
Remaining: the load order of func_001F5E60 args. In retail, x/y load before the ints,
rot loads right after y in the case-1 calls, and the 0xFFFFF3 lui is hoisted early.
The source parameter order of func_001F5E60 is unknown (Ghidra groups float params).
Decompiling func_001F5E60 itself (draw.c) would pin it down.

```c
/* A 2D glow effect: texture id +0x18, colour +0x14, size +0x10, angle
   +0x1C, spin step +0x28, copy count +0x26, layout +0x2C. */
typedef struct {
    char pad00[0x10];
    float size;
    int color;
    int tex;
    float angle;
    char pad20[6];
    short count;
    float spin;
    int layout;
} Glow;

extern long func_001F4868_l(int) __asm__("func_001F4868");
extern void func_001F5E60(float, float, int, int, long, long, int, float, float,
                          int, int, float, float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA748(float, float);

void func_001EE3B0(Glow *e, float x, float y) {
    float k = 40.0f;
    long tex = func_001F4868_l(e->tex);
    float ang = e->angle;
    float a[4];
    float b[4];

    switch (e->layout) {
    case 0: {
        int i;

        for (i = 0; i < e->count; i++) {
            func_001F5E60(x, y, 0x3F, 0x3F, tex, 0xFFFFF3, e->color, k * e->size, k * e->size, 0, 0, ang, 0.0f, 0.0f);
            ang = func_001FA748(ang, e->spin);
        }
        break;
    }
    case 1:
        a[0] = func_001F9FA8(ang) * k * e->size;
        a[1] = func_001F9F90(ang) * k * e->size;
        b[0] = func_001F9F90(ang) * k * e->size;
        b[1] = func_001F9FA8(ang) * -k * e->size;
        func_001F5E60(x, y, 0x3F, 0x3F, tex, 0xFFFFF3, e->color, e->size * k, e->size * k, 0, 0, ang, 0.0f, 0.0f);
        func_001F5E60(x + b[0], y + b[1], 0x3F, 0x3F, tex, 0xFFFFF3, e->color, e->size * k, e->size * k, 1, 0, ang, 0.0f, 0.0f);
        func_001F5E60(x - a[0], y - a[1], 0x3F, 0x3F, tex, 0xFFFFF3, e->color, e->size * k, e->size * k, 0, 1, ang, 0.0f, 0.0f);
        func_001F5E60(x + b[0] - a[0], y + b[1] - a[1], 0x3F, 0x3F, tex, 0xFFFFF3, e->color, e->size * k, e->size * k, 1, 1, ang, 0.0f, 0.0f);
        break;
    case 2:
        func_001F5E60(x, y, 0x3F, 0x3F, tex, 0xFFFFF3, e->color, e->size * k, e->size * k, 0, 0, ang, 0.5f, 0.5f);
        break;
    }
}
```
