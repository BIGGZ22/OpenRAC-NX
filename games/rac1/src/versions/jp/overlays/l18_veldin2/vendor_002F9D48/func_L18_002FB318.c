extern float D_J_L18_001E0290[4][4];
void draw_moby_three_scaled_two_tint_quads(struct Moby *m) {
    Pk_9c20_FB318 pk;
    float mat[4][4];
    float up[4];
    Vec4 vertices[4];
    int i = 0;
    int color;

    pk.b = GetEffectTex(0xB);
    pk.c = 0x0000FF9000000260ULL;
    pk.d = 0x8000000048ULL;
    pk.a = 0;
    VU1_addGSregister(0x4A, 0);
    VU1_addGSregister(0x47, 0x51001);
    func_001F9EC0(mat[3], &D_L18_001625F0, &m->rot_row0);
    FastVecAdd(mat[3], mat[3], &m->pos);
    *(u128_9c20_FB318 *)up = 0;
    up[2] = -1.0f;
    up[3] = 1.0f;
    func_001F9EC0(mat[0], up, &m->rot_row0);
    FastVecSub(mat[1], mat[3], D_L18_00167840);
    FastVecCross(mat[2], mat[1], mat[0]);
    func_L00_001FF4B0(mat[2], mat[2], 1.0f);
    FastVecCross(mat[1], mat[0], mat[2]);

    color = (*(int *)&D_L18_00162594) | 0xFF000000;
    pk.uv[0] = 1.0f;
    pk.uv[1] = 1.0f;
    pk.uv[2] = 0.0f;
    pk.uv[3] = 1.0f;
    pk.uv[4] = 1.0f;
    pk.uv[5] = 0.0f;
    pk.uv[6] = 0.0f;
    pk.uv[7] = 0.0f;

    pk.col[3] = color;
    pk.col[2] = color;
    pk.col[1] = color;
    pk.col[0] = color;
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_J_L18_001E0290[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_001625A4);
            vertices[i].f[0] *= D_L18_001625BC;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9EE8(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);

    pk.col[3] = (*(int *)&D_L18_001625A0) | 0x7F000000;
    pk.col[2] = pk.col[3];
    pk.col[1] = pk.col[3];
    pk.col[0] = pk.col[3];
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_J_L18_001E0290[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_001625A8);
            vertices[i].f[0] *= D_L18_001625BC;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9EE8(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);

    pk.col[3] = (*(int *)&D_L18_001625A0) | 0x7F000000;
    pk.col[2] = pk.col[3];
    pk.col[1] = pk.col[3];
    pk.col[0] = pk.col[3];
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_J_L18_001E0290[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_001625AC);
            vertices[i].f[0] *= D_L18_001625BC;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9EE8(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD1D8(&pk, 0, 0);
}
