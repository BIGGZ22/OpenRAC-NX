void update_moby_proximity_interaction(unsigned char *m) {
    char *d = *(char **)(m + 0x78);
    float *p;
    float tmp[4];
    int v;
    int one;
    unsigned char *q;

    if (m[0x20] != 0x2A) {
        if (m[0x31] != 0) {
            m[0xBC] += 2;
            v = (int)(FastCos(IntToFloat((unsigned char)m[0xBC] - 0x80) * 0.024543693f) * 50.0f) + 0x96;
            if (*(short *)(m + 0xA6) == 0x215) {
                *(int *)(m + 0x90) = (v >> 1) | (v << 8) | (v << 16);
            } else {
                *(int *)(m + 0x90) = v | (v << 8) | (v << 16);
            }
            AddDrawCallback(render_environment_mapped_object, m);
            if (func_001F9D48(m + 0x10, D_L00_00166EC0) < 32.0f) {
                AddDrawCallback(draw_light_flare, m);
            }
        }
        p = (float *)(m + 0x10);
        if (*(short *)(d + 0xC) != 0) {
            func_001F9EC0(tmp, D_L00_001BDC10 + ship_index[0] * 16, m + 0xC0);
            FastVecAdd(tmp, tmp, p);
            if (FastVecDist(tmp, D_0013E633 + 0xE9D) > 4.1f) {
                *(short *)(d + 0xC) = 0;
            }
        } else {
            q = (unsigned char *)D_0013E633 + 0xE9D;
            if (func_001F9D48(p, q) < 6.0f) {
                func_001F9EC0(tmp, D_L00_001BDC10 + ship_index[0] * 16, m + 0xC0);
                FastVecAdd(tmp, tmp, p);
                if (FastVecDist(tmp, q) < 4.0f) {
                    *(short *)(d + 0xC) = 1;
                }
            }
        }
        if (*(unsigned short *)(m + 0x34) & 1) {
            *(short *)(d + 0xC) = 0;
        }
        if (*(short *)(d + 0xC) != 0) {
            int r = try_set_help_message(2, 0x53E4) != 0;
            if ((*(int *)(D_0013A5E0 + 0x2604) & 0x10) && r) {
                if (D_0015EE84_m == 10 && *(unsigned char *)(D_0013E633 + 0x2EC1) == 1) {
                    m[0x20] = 0x2A;
                    *(unsigned short *)(m + 0x34) |= 0x41;
                    StartCutscene(9);
                } else {
                    Hud_ClosePopupIfActive(2);
                    D_L00_0015F710 = 1;
                    ship_index[1] = 0;
                }
            }
        }
    } else {
        if (D_L00_0015F6A8 != 2) {
            q = D_0013D355 + 0x13B;
            if (q[0x48] == 0 && gHaveMorphORay[7] != 0) {
                StartCutscene(4);
                q[0x48] = 1;
            } else if ((q = D_0013D355 + 0x13B)[0x49] == 0 && D_0013DE4B[8] != 0) {
                StartCutscene(5);
                q[0x49] = 1;
            } else if ((q = D_0013D355 + 0x13B)[0x4A] == 0 && D_0013DE4B[8] != 0) {
                start_indexed_disc_stream(0xB);
                q[0x4A] = 1;
            } else if ((q = D_0013D355 + 0x13B)[0x4B] == 0 && D_0013DE4B[8] != 0) {
                StartCutscene(6);
                q[0x4B] = 1;
            } else {
                m[0x20] = 0;
                *(unsigned short *)(m + 0x34) &= 0xFFBE;
                one = 1;
                ship_index[1] = one;
                D_L00_0015F710 = one;
            }
        }
    }
}
