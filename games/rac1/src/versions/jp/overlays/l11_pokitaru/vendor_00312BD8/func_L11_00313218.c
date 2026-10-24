void draw_ship_hud(char *moby) {
    char *d = *(char **)(moby + 0x78);
    char *s;
    short *p;
    unsigned char *base;
    unsigned char *g;
    void *list[12];
    int sp40;
    int sp44;
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
    int per;
    float t;
    float k;
    float m;
    float f;
    float q;

    VU1_addGSregister(0x42, 0x8000000044L);
    DrawTexturedQuad(0x170, (s = (char *)D_0013E15A + 0x4A6, *(int *)(s + 4)) - 0x90, 0x80, 0x80, 0, 0, 0x80, 0x80, 0x70808080, func_001F4868_l(0x3C));
    DrawTexturedQuad(0x170, *(int *)(s + 4) - 0x90, 0x80, 0x80, 0, 0, 0x80, 0x80, 0x70808080, func_001F4868_l(0x3D));
    VU1_addGSregister(0x42, 0x8000000048L);
    GetEffectTex(0x3E);
    p = D_L11_001AC540[*(short *)(d + 0x108)];
    if (p != 0) {
        do {
            char *o = (char *)(((*(unsigned short *)p & 0x7FFF) << 8) + D_L11_00160058_m);
            if (o[0x20] >= 0) {
                int cnt = collect_oil_tanker_chain_mobys(o, list);
                int w;
                for (w = 0; w < cnt; w++) {
                    draw_ship_radar_moby_icon((char *)D_L11_00167850, (char *)(D_L11_00167850 - 4), list[w], 0, 0x75808080);
                }
            }
        } while (*p++ >= 0);
    }
    y = 0x40;
    x = 0x18;
    base = (unsigned char *)D_0013E633 + 0xE1D;
    i = 0;
    if (base[0x15F7] != 0) {
        do {
            g = (unsigned char *)D_0013E633 + 0xE1D;
            if (i < g[0x15F6]) {
                append_scaled_screen_triangle_strip(x, y, 0.5f, D_L11_001F0FB8, 0x19, 0xFFFFF3, 0x50008F00);
            } else {
                append_scaled_screen_triangle_strip(x, y, 0.5f, D_L11_001F0FB8, 0x19, 0xFFFFF3, 0x20004F00);
            }
            n = g[0x15F7];
            if (i == n >> 1) {
                y = 0x2E;
                x += 0x1E;
            }
            y += 0x12;
            i++;
        } while (i < n);
    }
    ex = *(int *)(d + 0xE0);
    ey = *(int *)(d + 0xE4);
    tex = GetEffectTex(0x11);
    append_rotated_sprite_quad(ex, ey, 40.0f, 40.0f, 0.0f, 0x3F, 0x3F, tex, 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    tex = GetEffectTex(0x12);
    append_rotated_sprite_quad(ex, ey, 40.0f, 40.0f, 0.0f, 0x3F, 0x3F, tex, 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    tex = GetEffectTex(8);
    append_rotated_sprite_quad(ex, ey, 10.0f, 10.0f, 0.0f, 0x1F, 0x1F, tex, 0xFFFFF3, 0xFF20FF20, 0, 0, 0.5f, 0.5f);
    update_ship_lock_on_target((int)moby, d);
    if (*(int *)(d + 0x88) != 0) {
        project_world_point_to_screen_pixels(*(char **)(d + 0x88) + 0x10, &sp40, &sp44, 0);
        if (*(int *)(d + 0x8C) > *(int *)&lock_hold_frames) {
            t = IntToFloat(*(int *)(d + 0x8C) - *(int *)&lock_hold_frames);
            t = t / IntToFloat(*(int *)&lock_fade_frames);
            m = 2.0f * (1.0f - t);
            k = t * 5.0f + 1.0f;
            if (m > 1.0f) m = 1.0f;
            c = func_001FA898_r(m * 96.0f);
            func_L11_00312510_f(IntToFloat(sp40), IntToFloat(sp44), k, normalize_angle(IntToFloat(level_frame) / 30.0f), 0, 0xFF, 0, c);
            *(int *)(d + 0xEC) = 0;
        } else {
            a = 0xFF;
            if ((*(int *)(d + 0x8C) / scale_ticks(0x14)) & 1) a = 0;
            func_L11_00312510_f(IntToFloat(sp40), IntToFloat(sp44), 1.0f, 0.0f, 0xFF, a, 0, 0x60);
            *(int *)(d + 0xEC) = *(int *)(d + 0x88);
        }
    }
    {
    unsigned char *b = (unsigned char *)D_0013E633 + 0xE1D;
    if (*(float *)(b + 0x15FC) < *(float *)(b + 0x1600) * 0.25f) {
        int cnt = ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].count;
        if (cnt == 0) {
            Help_Request(0x2AFF, 0x58);
        } else if (cnt < 2) {
            *(int *)(d + 0x11C) += 1;
            if (*(int *)(d + 0x11C) > scale_ticks(0x708)) {
                Help_Request(0x2AFF, 0x58);
            }
        }
    } else if (*(int *)(d + 0x11C) != 0) {
        if (((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].count < 0xFFFF) {
            ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].count++;
        }
        per = 600;
        if (scale_ticks(D_0015EFA4) / per > ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].best) {
            ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].best = scale_ticks(D_0015EFA4) / 600;
        }
        ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].flags = ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x58].flags | (1 << D_0015EE84) | 0x80000000;
        *(int *)(d + 0x11C) = 0;
    }
    }
    func_001F9908_i(d + 0x120);
    {
    unsigned char *g2 = (unsigned char *)D_0013E633 + 0xE1D;
    if ((g2[0x15F6] < 5 && ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x57].count == 0)
        || (g2[0x15F6] == 0 && ((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x57].count < 2 && *(int *)(d + 0x120) == 0)) {
        Help_Request(0x2AFE, 0x57);
        *(int *)(d + 0x120) = scale_ticks(0x4B0);
    }
    }
    if (((HelpTab_313218 *)(D_0014171B + 0x34D))->r[0x56].count == 0) {
        if ((*(int *)(d + 0x128))++ > scale_ticks(0xE10)) {
            Help_Request(0x2AFD, 0x56);
        }
    }
    if (*(unsigned char *)(moby + 0x20) == 8) {
        func_001F6FD8_c(0x100, 0xC8, 0x80005080L, func_001FE540_id(0x5295), 0x11);
    }
    {
    unsigned char *h = (unsigned char *)D_0013E633 + 0xE1D;
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
            (*(int **)&D_L11_001621FC)[idx] = D_L11_001F1348[idx];
        } else {
            (*(int **)&D_L11_001621FC)[idx] = 0x80000000;
        }
    }
    *(int *)(d + 0xF4) = v;
    }
}
