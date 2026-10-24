extern short D_E1_L18_00161FAC;
void ArcEmitterUpdate(void *moby) {
    V_ec290 s[10];
    V_ec290 d[5];
    unsigned char flag = *(unsigned char *)D_0013E633;
    char *data = *(char **)((char *)moby + 0x78);
    float scale;
    int i;
    int k;
    float f20;
    int r;

    *(float *)&D_L18_00162098 = FastAddRots(*(float *)&D_L18_00162098, *(float *)&D_L18_0016209C);
    *(float *)&D_L18_001620A0 = FastSubRots(*(float *)&D_L18_001620A0, *(float *)&D_L18_001620A4);
    *(float *)&D_L18_001620B4 = FastAddRots(*(float *)&D_L18_001620B4, *(float *)&D_L18_001620B8);
    scale = FastVecDist((char *)moby + 0x10, data + 0x10) * 0.05f;
    if (flag) {
        *(int *)&D_L18_001620E4 = 0x7F2040;
    } else {
        *(int *)&D_L18_001620E4 = 0x7F2020;
    }
    FastVecSub(&s[0], data + 0x10, (char *)moby + 0x10);
    func_L00_001FF4B0(&s[0], &s[0], 1.0f);
    FastVecCross(&s[1], &s[0], D_0013F6E0);
    build_look_at_matrix(&s[0], &s[0], &s[1], 0.17453292f);
    FastVecScale(&s[3], &s[0], scale);
    qcopy(D_L18_001D9FD0[0], (char *)moby + 0x10);
    qcopy(D_L18_001D9FD0[1], (char *)moby + 0x10);
    for (i = 1; i < 20; i++) {
        float f21;
        float len;
        float len0;
        float a;
        float b;
        f21 = *(float *)&D_L18_00162090
              + (*(float *)&D_L18_00162094 - *(float *)&D_L18_00162090) * (IntToFloat(i) * 0.05f);
        f21 = f21 * FastSin(normalize_angle(*(float *)&D_L18_00162098 + (float)i * *(float *)&D_L18_0016208C));
        f21 = f21 + randf_sym(*(float *)&D_L18_001620D0, *(float *)&D_L18_001620D4);
        D_L18_001D9FD0[0][i].f[2] = D_L18_001D9FD0[0][i].f[2] - D_L18_001DA250[i];
        D_L18_001DA250[i] = f21;
        a = random_angle_radians();
        b = random_angle_radians();
        BuildSphericalOffset(&s[1], scale, a, b);
        s[1].f[2] = s[1].f[2] * 0.5f;
        func_001F9C08(&s[2], &s[3], &s[1], *(float *)&D_L18_0016211C);
        len0 = FastVecLength(&s[2]);
        if (len0 == 0.0f) {
            qcopy(&D_L18_001D9FD0[0][i], &D_L18_001D9FD0[0][i - 1]);
        } else {
            FastVecScale(&s[2], &s[2], scale / len0);
            FastVecAdd(&D_L18_001D9FD0[0][i], &D_L18_001D9FC0[i], &s[2]);
            qcopy(&s[3], &s[2]);
        }
        D_L18_001D9FD0[0][i].f[2] = D_L18_001D9FD0[0][i].f[2] + f21;
        FastVecSub(&s[1], &D_L18_001D9FD0[0][i], &D_L18_001D9FC0[i]);
        FastVecSub(&s[5], data + 0x10, &D_L18_001D9FD0[0][i]);
        len = FastVecLength(&s[5]);
        if (len != 0.0f) {
            float w;
            if (i == 19) {
                w = 0.5f;
                scale = len;
            } else if (i >= 16) {
                w = 0.5f;
                scale = len / IntToFloat(19 - i);
            } else {
                w = 0.1f;
            }
            FastVecScale(&s[5], &s[5], scale / len);
            func_001F9C08(&s[3], &s[3], &s[5], w);
        }
        if (i < 10) {
            float g;
            float h;
            float ang;
            float m;
            g = normalize_angle(*(float *)&D_L18_001620A0 + (float)i * *(float *)&D_L18_001620A8);
            h = *(float *)&D_L18_001620AC
                * FastSin(normalize_angle(*(float *)&D_L18_001620B4 + (float)i * *(float *)&D_E1_L18_00161FAC));
            qcopy(&D_L18_001D9FD0[1][i], &D_L18_001D9FD0[0][i]);
            h = h * FastSin(g) + randf_sym(0.0f, 0.1f);
            D_L18_001D9FD0[1][i].f[2] = D_L18_001D9FD0[1][i].f[2] + h;
            D_L18_001DA3E0[i] = h;
            ang = FastAddRots(func_L00_001FF860(s[1].f[0], s[1].f[1]), 1.5707964f);
            m = *(float *)&D_L18_001620B0 * FastCos(g);
            s[4].f[0] = FastCos(ang) * m;
            s[4].f[1] = FastSin(ang) * m;
            s[4].f[2] = 0.0f;
            qcopy(&D_L18_001DA2A0[i], &s[4]);
            FastVecAdd(&D_L18_001D9FD0[1][i], &D_L18_001D9FD0[1][i], &s[4]);
        } else {
            int r = 21 - i;
            qcopy(&D_L18_001D9FD0[1][i], &D_L18_001D9FD0[0][i]);
            D_L18_001D9FD0[1][i].f[2] = D_L18_001D9FD0[1][i].f[2] + D_L18_001DA3E0[r];
            FastVecAdd(&D_L18_001D9FD0[1][i], &D_L18_001D9FD0[1][i], &D_L18_001DA2A0[r]);
            qcopy(&s[4], &D_L18_001DA2A0[i]);
        }
    }
    FastVecCross(&s[6], &s[0], D_0013F6E0);
    func_L00_001FF4B0(&s[6], &s[6], 1.0f);
    func_L00_001FF4B0(&s[7], D_0013F6E0, 1.0f);
    for (k = 0; k < 4; k++) {
        float t;
        if (k < 2) {
            qcopy(&s[8], (char *)moby + 0x10);
            t = 0.4f;
        } else {
            qcopy(&s[8], data + 0x10);
            t = 0.5f;
        }
        if (FastDecTimer(D_L18_00162148 + k)) {
            int j;
            D_L18_00162148[k] = scale_ticks(D_L18_00162158[k]);
            D_L18_00162150[k] = 0;
            scale = randf_sym(0.52359879f, 1.04719758f);
            f20 = random_float_between(-1.04719758f, 0.17453292f);
            FastVecScale(&s[5], &s[0], t);
            build_look_at_matrix(&s[5], &s[5], &s[6], f20);
            build_look_at_matrix(&s[5], &s[5], &s[7], scale);
            qcopy(&D_L18_001DA430[k].p[0], &s[8]);
            FastVecAdd(&D_L18_001DA430[k].p[1], &s[8], &s[5]);
            t = 1.0f;
            for (j = 2; j < 5; j++) {
                FastVecSub(&s[9], &D_L18_001DA420[k].p[j - 1], D_L18_00167840);
                f20 = random_float_between(0.17453292f, 0.959931076f) * t;
                t = -t;
                build_look_at_matrix(&s[5], &s[5], &s[9], f20);
                FastVecAdd((D_L18_001DA430[k].p + j), &D_L18_001DA420[k].p[j - 1], &s[5]);
            }
        } else {
            int j;
            for (j = 1; j < 5; j++) {
                FastVecSub(&d[j], (D_L18_001DA430[k].p + j), &D_L18_001DA420[k].p[j - 1]);
                d[j].f[0] += randf_sym(0.0f, 0.1f);
                d[j].f[1] += randf_sym(0.0f, 0.1f);
                d[j].f[2] += randf_sym(0.0f, 0.1f);
            }
            qcopy(&D_L18_001DA430[k].p[0], &s[8]);
            for (j = 1; j < 5; j++) {
                FastVecAdd((D_L18_001DA430[k].p + j), &D_L18_001DA420[k].p[j - 1], &d[j]);
            }
            {
                int x = D_L18_00162160[k];
                int y = D_L18_00162148[k];
                float fr;
                if (x < y) {
                    x = y - x;
                } else {
                    x = x - y;
                }
                fr = IntToFloat(x);
                fr = fr / IntToFloat(scale_ticks(15));
                D_L18_00162150[k] = func_001FA898_r((1.0f - fr) * 32.0f);
            }
        }
    }
    scale = randf_sym(0.52359879f, 1.04719758f);
    f20 = random_float_between(-0.52359879f, 1.04719758f);
    FastVecScale(&s[5], &s[0], *(float *)&D_L18_00162114 * D_0015EE60);
    build_look_at_matrix(&s[5], &s[5], &s[6], f20);
    build_look_at_matrix(&s[5], &s[5], &s[7], scale);
    r = rand_range(1, 8);
    if (random_integer_below(2)) r = -r;
    f20 = random_float_between(0.2f, 0.6f);
    spawn_ec290((char *)moby + 0x10, scale_ticks(15), f20 * 0.3f, f20, *(int *)&D_L18_001620E4 | 0x7F000000, 0, r, &s[5], 0.0f);
    r = -r;
    spawn_ec290((char *)moby + 0x10, scale_ticks(15), f20 * 0.15f, f20 * 0.5f, 0x307F7F7F, 0, r, &s[5], 0.0f);
    r = rand_range(1, 8);
    if (random_integer_below(2)) r = -r;
    f20 = random_float_between(0.15f, 3.0f);
    spawn_ec290(data + 0x10, scale_ticks(12), f20 * 0.1f, f20, *(int *)&D_L18_001620E4 | 0x7F000000, 0, r, D_L18_0015F660_2D7310, 0.0f);
}
