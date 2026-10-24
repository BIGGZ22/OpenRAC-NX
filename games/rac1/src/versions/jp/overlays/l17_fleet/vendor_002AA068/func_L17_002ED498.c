void DrawFleetHud(char *moby) {
    char *d = *(char **)(moby + 0x78);
    char *s;
    unsigned char *base;
    unsigned char *g;
    unsigned char *h;
    unsigned char *b;
    int sp10;
    int sp14;
    int tex;
    int i;
    unsigned char n;
    unsigned char c;
    int j;
    int x;
    int y;
    int ex;
    int ey;
    int a;
    int v;
    int lo;
    int hi;
    int idx;
    float t;
    float k;
    float m;
    float px;
    float py;
    float f;
    float q;

    VU1_addGSregister(0x42, 0x8000000044L);
    DrawTexturedQuad(0x170, (s = (char *)D_0013E15A + 0x4A6, *(int *)(s + 4)) - 0x90, 0x80, 0x80, 0, 0, 0x80, 0x80, 0x70808080, func_001F4868_l(*(int *)(d + 0x104) + 0x28));
    DrawTexturedQuad(0x170, *(int *)(s + 4) - 0x90, 0x80, 0x80, 0, 0, 0x80, 0x80, 0x70808080, func_001F4868_l(*(int *)(d + 0x104) + 0x29));
    VU1_addGSregister(0x42, 0x8000000048L);
    update_fighter_jet_target_lock((int)moby, d);
    if (*(int *)(d + 0x88) != 0) {
        project_world_point_to_screen_pixels(*(char **)(d + 0x88) + 0x10, &sp10, &sp14, 0);
        if (*(int *)(d + 0x8C) > *(int *)&D_L17_001621F0) {
            t = IntToFloat(*(int *)(d + 0x8C) - *(int *)&D_L17_001621F0);
            t = t / IntToFloat(*(int *)&D_L17_001621F4);
            m = 2.0f * (1.0f - t);
            k = t * 5.0f + 1.0f;
            if (m > 1.0f) m = 1.0f;
            c = func_001FA898_r(m * 96.0f);
            func_L17_002EBF08_f(IntToFloat(sp10), IntToFloat(sp14), k, normalize_angle(IntToFloat(D_L17_0015F6B0) / 30.0f), 0, 0xFF, 0, c);
            *(int *)(d + 0xEC) = 0;
        } else {
            a = 0xFF;
            if ((*(int *)(d + 0x8C) / scale_ticks(0x14)) & 1) a = 0;
            func_L17_002EBF08_f(IntToFloat(sp10), IntToFloat(sp14), 1.0f, 0.0f, 0xFF, a, 0, 0x60);
            *(int *)(d + 0xEC) = *(int *)(d + 0x88);
        }
    }
    y = 0x40;
    x = 0x18;
    base = D_0013E633 + 0xE1D;
    n = base[0x15F7];
    for (i = 0; i < n; i++) {
        g = D_0013E633 + 0xE1D;
        if (i < g[0x15F6]) {
            append_scaled_screen_triangle_strip(x, y, 0.5f, D_L17_001D9CB0, 0x19, 0xFFFFF3, 0x50008F00);
        } else {
            append_scaled_screen_triangle_strip(x, y, 0.5f, D_L17_001D9CB0, 0x19, 0xFFFFF3, 0x20004F00);
        }
        n = g[0x15F7];
        if (i == n >> 1) {
            y = 0x2E;
            x += 0x1E;
        }
        y += 0x12;
    }
    ex = *(int *)(d + 0xE0);
    ey = *(int *)(d + 0xE4);
    tex = GetEffectTex(0x11);
    append_rotated_sprite_quad(ex, ey, 40.0f, 40.0f, 0.0f, 0x3F, 0x3F, tex, 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    append_rotated_sprite_quad(ex, ey, 40.0f, 40.0f, 0.0f, 0x3F, 0x3F, GetEffectTex(0x12), 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    append_rotated_sprite_quad(ex, ey, 10.0f, 10.0f, 0.0f, 0x1F, 0x1F, GetEffectTex(8), 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    b = D_0013E633 + 0xE1D;
    if (*(float *)(b + 0x15FC) < *(float *)(b + 0x1600) / 10.0f && (D_L17_0015F6B0 / 90) & 1) {
        func_001F6F40_c(0x100, 0x186, 0x80000080L, func_001FE540_id(0x52C0), 0x64);
    }
    if (*(unsigned char *)(moby + 0x20) == 8) {
        func_001F6F40_c(0x100, 0xC8, 0x80005080L, func_001FE540_id(0x5295), 0x64);
    }
    h = D_0013E633 + 0xE1D;
    f = *(float *)(h + 0x15FC) - *(float *)(d + 0xF0);
    f *= 0.2f;
    q = FastAbsF(f);
    if (q > 1.0f) f /= q;
    *(float *)(d + 0xF0) += f;
    v = func_001FA898_r(*(float *)(d + 0xF0) * 251.0f / *(float *)(h + 0x1600)) + 2;
    if (v > 0xFD) v = 0xFD;
    if (v < 2) v = 2;
    lo = *(int *)(d + 0xF4) <= v ? *(int *)(d + 0xF4) : v;
    hi = *(int *)(d + 0xF4) < v ? v : *(int *)(d + 0xF4);
    for (j = lo; j <= hi; j++) {
        idx = (j & 0xE7) | ((j & 0x10) >> 1) | ((j & 8) << 1);
        if (j < v) {
            (*(int **)&D_L17_00162200)[idx] = D_L17_001D9E38[idx];
        } else {
            (*(int **)&D_L17_00162200)[idx] = 0x80000000;
        }
    }
    *(int *)(d + 0xF4) = v;
}
