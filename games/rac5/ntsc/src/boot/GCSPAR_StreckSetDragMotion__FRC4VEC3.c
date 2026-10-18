typedef struct {
    float x;
    float y;
    float z;
} VEC3;

extern VEC3 gcspar_streck_drag;

void GCSPAR_StreckSetDragMotion__FRC4VEC3(const VEC3 *motion) {
    gcspar_streck_drag = *motion;
}
