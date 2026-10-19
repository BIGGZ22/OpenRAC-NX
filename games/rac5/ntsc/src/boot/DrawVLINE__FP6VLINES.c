typedef struct { float x, y, z; } VEC3;
typedef struct { VEC3 start; VEC3 end; unsigned int color; } VLINES;

extern void UTIL_Draw2DLine__FP4VEC3UiUi(VEC3 *p, unsigned int color, unsigned int mode);

void DrawVLINE__FP6VLINES(VLINES *v) {
    UTIL_Draw2DLine__FP4VEC3UiUi(&v->start, v->color, 0xFFFFFF);
}
