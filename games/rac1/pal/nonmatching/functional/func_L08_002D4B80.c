/* functional: medium. Draw callback of a Scoutbot's beam: builds a textured quad packet (colour from four sdata words, UVs of a unit square), orients it towards the target moby (data+0x218) with a rotation matrix from two angles, and draws four pairs of quads (lit core and two flaring side strips, each drawn twice with the +-0.25 jitter). Not run against captured calls. */
typedef struct Vec_2D4B80 {
    float v[4];
} Vec_2D4B80 __attribute__((aligned(16)));
typedef struct Packet_2D4B80 {
    Vec_2D4B80 vertices[4];
    unsigned int color[4];
    float uv[8];
    long tag[4];
} Packet_2D4B80;

extern long func_001F4868_4B80(int) __asm__("func_001F4868");
extern void func_L00_00250800_4B80(void *, int, void *) __asm__("func_L00_00250800");
extern void func_001F9BF0_4B80(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_002607A8_4B80(void *, float) __asm__("func_L00_002607A8");
extern float func_001F9CB8_4B80(void *) __asm__("func_001F9CB8");
extern void func_001F9BC0_4B80(void *) __asm__("func_001F9BC0");
extern float func_001F9CE8_4B80(void *) __asm__("func_001F9CE8");
extern float func_L00_001FF860_4B80(float, float) __asm__("func_L00_001FF860");
extern void func_001FA218_4B80(void *, void *) __asm__("func_001FA218");
extern void func_L00_001FD1D8_4B80(void *, void *, int) __asm__("func_L00_001FD1D8");
extern int G_C_4B80[4] __asm__("D_L08_00161908");
extern float D_L08_00161924_4B80 __asm__("D_L08_00161924");

void func_L08_002D4B80_impl(char *m) __asm__("func_L08_002D4B80");
void func_L08_002D4B80_impl(char *m) {
    char *data = *(char **)(m + 0x78);
    Packet_2D4B80 pk;
    Vec_2D4B80 matrix[3], direction, pos, delta, angles;
    float length;
    long tag;
    float *v;
    int i;

    tag = func_001F4868_4B80(14);
    pk.color[0] = G_C_4B80[0];
    pk.color[1] = G_C_4B80[1];
    pk.color[2] = G_C_4B80[2];
    pk.color[3] = G_C_4B80[3];
    pk.tag[1] = tag;
    pk.tag[2] = 0xFF9000000260L;
    pk.tag[0] = 5;
    pk.tag[3] = 0x8000000048L;
    pk.uv[0] = 0.0f; pk.uv[1] = 0.0f;
    pk.uv[2] = 1.0f; pk.uv[3] = 0.0f;
    pk.uv[4] = 0.0f; pk.uv[5] = 1.0f;
    pk.uv[6] = 1.0f; pk.uv[7] = 1.0f;
    func_L00_00250800_4B80(m, 2, &pos);
    func_001F9BF0_4B80(&delta, *(char **)(data + 0x218) + 0x10, &pos);
    func_L00_002607A8_4B80(&delta, 3.0f);
    delta.v[3] = 1.0f;
    delta.v[2] = (*(float *)(*(char **)(data + 0x218) + 0x18) + 0.25f) - pos.v[2];
    length = func_001F9CB8_4B80(&delta);
    func_001F9BC0_4B80(&angles);
    angles.v[1] = -func_L00_001FF860_4B80(func_001F9CE8_4B80(&delta), delta.v[2]);
    angles.v[2] = func_L00_001FF860_4B80(delta.v[0], delta.v[1]);
    func_001FA218_4B80(matrix, &angles);
    qcopy(&direction, &pos);
    direction.v[3] = 1.0f;

    func_001F9BC0_4B80(&pk.vertices[0]);
    pk.vertices[0].v[2] = D_L08_00161924_4B80;
    pk.vertices[0].v[3] = 1.0f;
    qcopy(&pk.vertices[1], data + 0x1D0);
    pk.vertices[1].v[0] = length * *(float *)(data + 0x1DC);
    pk.vertices[1].v[2] += -0.25f;
    pk.vertices[1].v[3] = 1.0f;
    func_001F9BC0_4B80(&pk.vertices[2]);
    pk.vertices[2].v[2] = 0.0f;
    pk.vertices[2].v[3] = 1.0f;
    qcopy(&pk.vertices[3], data + 0x1D0);
    pk.vertices[3].v[0] = length * *(float *)(data + 0x1DC);
    pk.vertices[3].v[2] += 0.25f;
    pk.vertices[3].v[3] = 1.0f;
    func_L00_001FD1D8_4B80(&pk, matrix, 1);
    v = pk.vertices[1].v;
    for (i = 1; i < 4; i += 2, v += 8) {
        if (i < 2) { v[1] += -0.25f; v[2] += 0.25f; }
        else { v[1] += 0.25f; v[2] += -0.25f; }
    }
    func_L00_001FD1D8_4B80(&pk, matrix, 1);

    qcopy(&pk.vertices[0], data + 0x1D0);
    pk.vertices[0].v[0] = length * *(float *)(data + 0x1DC);
    pk.vertices[0].v[2] += -0.25f;
    pk.vertices[0].v[3] = 1.0f;
    qcopy(&pk.vertices[1], data + 0x1E0);
    pk.vertices[1].v[0] = length * *(float *)(data + 0x1EC);
    pk.vertices[1].v[2] += -0.25f;
    pk.vertices[1].v[3] = 1.0f;
    qcopy(&pk.vertices[2], data + 0x1D0);
    pk.vertices[2].v[0] = length * *(float *)(data + 0x1DC);
    pk.vertices[2].v[2] += 0.25f;
    pk.vertices[2].v[3] = 1.0f;
    qcopy(&pk.vertices[3], data + 0x1E0);
    pk.vertices[3].v[0] = length * *(float *)(data + 0x1EC);
    pk.vertices[3].v[2] += 0.25f;
    pk.vertices[3].v[3] = 1.0f;
    func_L00_001FD1D8_4B80(&pk, matrix, 1);
    v = pk.vertices[0].v;
    for (i = 0; i < 4; i++, v += 4) {
        if (i < 2) { v[1] += -0.25f; v[2] += 0.25f; }
        else { v[1] += 0.25f; v[2] += -0.25f; }
    }
    func_L00_001FD1D8_4B80(&pk, matrix, 1);

    qcopy(&pk.vertices[0], data + 0x1E0);
    pk.vertices[0].v[0] = length * *(float *)(data + 0x1EC);
    pk.vertices[0].v[2] += -0.25f;
    pk.vertices[0].v[3] = 1.0f;
    func_001F9BC0_4B80(&pk.vertices[1]);
    pk.vertices[1].v[0] = length * *(float *)(data + 0x1DC) * 3.0f;
    pk.vertices[1].v[2] += D_L08_00161924_4B80;
    pk.vertices[1].v[3] = 1.0f;
    qcopy(&pk.vertices[2], data + 0x1E0);
    pk.vertices[2].v[0] = length * *(float *)(data + 0x1EC);
    pk.vertices[2].v[2] += 0.25f;
    pk.vertices[2].v[3] = 1.0f;
    func_001F9BC0_4B80(&pk.vertices[3]);
    pk.vertices[3].v[0] = length * *(float *)(data + 0x1DC) * 3.0f;
    pk.vertices[3].v[2] = 0.0f;
    pk.vertices[3].v[3] = 1.0f;
    func_L00_001FD1D8_4B80(&pk, matrix, 1);
    v = pk.vertices[0].v;
    for (i = 0; i < 4; i += 2, v += 8) {
        if (i < 2) { v[1] += -0.25f; v[2] += 0.25f; }
        else { v[1] += 0.25f; v[2] += -0.25f; }
    }
    func_L00_001FD1D8_4B80(&pk, matrix, 1);
}
