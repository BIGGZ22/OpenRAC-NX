typedef struct { float x, y, z; } VEC3;
typedef struct { int count; char pad[0x0C]; } PUBE;

extern void Draw2DLineStrip__FP4VEC3UiUi(VEC3 *p, unsigned int count, unsigned int color);

void DrawPube__FP4PUBE(PUBE *p) {
    if (p->count) {
        Draw2DLineStrip__FP4VEC3UiUi((VEC3 *)((char *)p + 0x10), p->count, 0x2F000000);
    }
}
