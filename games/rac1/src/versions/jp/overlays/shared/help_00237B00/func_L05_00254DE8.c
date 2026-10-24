void update_hero_hoverboard_race_progress(void) {
    float v0[4] __attribute__((aligned(16)));
    float v1[4] __attribute__((aligned(16)));
    Data_254DE8 *d;
    Path_254DE8 *path;
    int near;
    int j;
    int k;
    int m;
    float ang;
    float dir;

    ((Hero *)(D_0013E633 + 0xE1D))->unk894++;
    if (((Moby *)((Hero *)(D_0013E633 + 0xE1D))->unk86C) == 0) {
        return;
    }
    d = (Data_254DE8 *)((Moby *)((Hero *)(D_0013E633 + 0xE1D))->unk86C)->pvars;
    if (d->f20 == -1) {
        return;
    }
    path = D_L05_001B0CB0_254DE8[d->f20];
    ((Hero *)(D_0013E633 + 0xE1D))->unk8B4 = (void *)path;
    if (D_0015EE84_2523C8 == 0x10) {
        *(u128_254DE8 *)v0 = 0;
        v0[0] = 163.0f;
        v0[1] = 119.0f;
        v0[2] = 80.0f;
        if (func_001F9D10_254DE8(((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f, v0) < 11.0f) {
            if (((Hero *)(D_0013E633 + 0xE1D))->unk898 < 3 || ((Hero *)(D_0013E633 + 0xE1D))->unk898 > 0x46) {
                if (((Hero *)(D_0013E633 + 0xE1D))->unk898 >= 3) {
                    ((Hero *)(D_0013E633 + 0xE1D))->unk89A++;
                }
                ((Hero *)(D_0013E633 + 0xE1D))->unk898 = 0xD;
            }
        }
        *(u128_254DE8 *)v1 = 0;
        v1[0] = 223.0f;
        v1[1] = 281.0f;
        v1[2] = 93.0f;
        if (func_001F9D10_254DE8(((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f, v1) < 7.0f) {
            if (((Hero *)(D_0013E633 + 0xE1D))->unk898 < 0x3B) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk898 = 0x3B;
            }
        }
    }
    {
        int i;
        int n1;
        int next;

        i = ((Hero *)(D_0013E633 + 0xE1D))->unk898;
        n1 = path->n + 1;
        next = (i + n1) % path->n;
        ang = func_L00_001FF860_254DE8(path->p[next].x - path->p[i].x, path->p[next].y - path->p[i].y);
        if (func_001FA850_254DE8(ang, func_L00_001FF860_254DE8(((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f[0] - path->p[i].x, ((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f[1] - path->p[i].y)) < 1.5707964f) {
            ((Hero *)(D_0013E633 + 0xE1D))->unk898 = next;
            if (next == 0) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk89A++;
            }
        }
    }
    near = 0;
    if (D_0015EE84_2523C8 == 0x10) {
        if (((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f[2] < path->p[((Hero *)(D_0013E633 + 0xE1D))->unk898].z - 2.0f) {
            near = 1;
            if (((Hero *)(D_0013E633 + 0xE1D))->unk8AE < scale_ticks(0x46)) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk8AE = scale_ticks(0x46);
            }
        }
    }
    {
        int i;
        int n1;
        int next;

        i = ((Hero *)(D_0013E633 + 0xE1D))->unk8C8;
        n1 = path->n + 1;
        next = (i + n1) % path->n;
        dir = func_L00_001FF860_254DE8(path->p[next].x - path->p[i].x, path->p[next].y - path->p[i].y);
    }
    if (near != 0 || (func_001FA850_254DE8(((Hero *)(D_0013E633 + 0xE1D))->motion.rot.f[2], dir) > 1.5707964f && ((Hero *)(D_0013E633 + 0xE1D))->unk88D == 0)) {
        ((Hero *)(D_0013E633 + 0xE1D))->unk8AE++;
        if (((Hero *)(D_0013E633 + 0xE1D))->unk8AE > scale_ticks(0xB4)) {
            if (D_L05_0015F760 == 0) {
                D_L05_0015F760 = 1;
                D_L05_0015F764 = 0;
                D_L05_0015F75C = D_0015EE60_254DE8 * 0.08f;
            }
            ((Hero *)(D_0013E633 + 0xE1D))->unk8AF = 1;
            if (D_L05_0015F764 == 1) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk8AF = 0;
                func_L05_00254B38_254DE8(v0, v1);
                ((Hero *)(D_0013E633 + 0xE1D))->unk8AE = 0;
                ((Hero *)(D_0013E633 + 0xE1D))->unk874 = v1[2];
                func_L00_00217718_254DE8(v0, v1, 0x6B, 1);
                if (((Path_254DE8 *)((Hero *)(D_0013E633 + 0xE1D))->unk8B4) != 0) {
                    int r = func_L01_0028C2D8_254DE8(((Hero *)(D_0013E633 + 0xE1D))->motion.pos.f, (Path_254DE8 *)((Hero *)(D_0013E633 + 0xE1D))->unk8B4, 0.0f);

                    if (r < ((Hero *)(D_0013E633 + 0xE1D))->unk898) {
                        ((Hero *)(D_0013E633 + 0xE1D))->unk898 = r;
                    }
                }
            }
        }
    } else if (((Hero *)(D_0013E633 + 0xE1D))->unk8AF == 0) {
        ((Hero *)(D_0013E633 + 0xE1D))->unk8AE = 0;
    }
    for (j = 0; j < d->f40; j++) {
        int i;
        int n1;
        int next;

        i = ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[j];
        n1 = path->n + 1;
        next = (i + n1) % path->n;
        ang = func_L00_001FF860_254DE8(path->p[next].x - path->p[i].x, path->p[next].y - path->p[i].y);
        qcopy(v0, D_L05_00160098_254DE8[d->f2C[j]].pos);
        if (func_001FA850_254DE8(ang, func_L00_001FF860_254DE8(v0[0] - path->p[i].x, v0[1] - path->p[i].y)) < 1.5707964f) {
            ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[j] = next;
            if (next == 0) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk7A0[j]++;
            }
        }
    }
    ((Hero *)(D_0013E633 + 0xE1D))->unk8C0 = 1;
    for (j = 0; j < d->f40; j++) {
        if (((Hero *)(D_0013E633 + 0xE1D))->unk89A < ((Hero *)(D_0013E633 + 0xE1D))->unk7A0[j] || (((Hero *)(D_0013E633 + 0xE1D))->unk7A0[j] == ((Hero *)(D_0013E633 + 0xE1D))->unk89A && ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[j] >= ((Hero *)(D_0013E633 + 0xE1D))->unk898)) {
            ((Hero *)(D_0013E633 + 0xE1D))->unk8C0++;
        }
    }
    for (k = 0; k < d->f40; k++) {
        ((Hero *)(D_0013E633 + 0xE1D))->unk7C0[k] = 1;
        if (((Hero *)(D_0013E633 + 0xE1D))->unk89A > ((Hero *)(D_0013E633 + 0xE1D))->unk7A0[k] || (((Hero *)(D_0013E633 + 0xE1D))->unk7A0[k] == ((Hero *)(D_0013E633 + 0xE1D))->unk89A && ((Hero *)(D_0013E633 + 0xE1D))->unk898 > ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[k])) {
            ((Hero *)(D_0013E633 + 0xE1D))->unk7C0[k] = 2;
        }
        for (m = 0; m < d->f40; m++) {
            if (((Hero *)(D_0013E633 + 0xE1D))->unk7A0[k] < ((Hero *)(D_0013E633 + 0xE1D))->unk7A0[m] || (((Hero *)(D_0013E633 + 0xE1D))->unk7A0[k] == ((Hero *)(D_0013E633 + 0xE1D))->unk7A0[m] && ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[k] < ((Hero *)(D_0013E633 + 0xE1D))->unk7B0[m])) {
                ((Hero *)(D_0013E633 + 0xE1D))->unk7C0[k]++;
            }
        }
    }
    if (((Hero *)(D_0013E633 + 0xE1D))->unk89A < 3) {
        return;
    }
    if (*(D_0013D355_254DE8 + 0x13B) != 0) {
        D_L05_0015F788 = 1;
    }
    if (D_0015EE84_2523C8 == 5) {
        D_0015EF38_254DE8++;
    } else if (D_0015EE84_2523C8 == 0x10) {
        D_0015EF3C_254DE8++;
    }
    if (D_0015EE84_2523C8 == 5) {
        int t;

        if (D_0015EE80_254DE8 != 0) {
            t = scale_ticks(0x1680);
        } else {
            t = scale_ticks(0x1644);
        }
        if (t >= ((Hero *)(D_0013E633 + 0xE1D))->unk894) {
            if (((Flags_254DE8 *)(D_0013D50F_254DE8 + 1))->f9 == 0) {
                ((Flags_254DE8 *)(D_0013D50F_254DE8 + 1))->f9 = 1;
                func_0022EE28_254DE8(1, 0, 0);
                func_L00_00264DB8_254DE8(0x542D, -1);
            }
        }
        if (d->f28 == -1) {
            return;
        }
        if (*(D_0013D355_254DE8 + 0x13B) == 0 && ((Hero *)(D_0013E633 + 0xE1D))->unk8C0 == 1) {
            D_L05_00160098_254DE8[d->f28].fBC = 1;
        } else {
            ((Hero *)(D_0013E633 + 0xE1D))->unk890 = (u8 *)(&D_L05_00160098_254DE8[d->f28]);
            func_001FBC80_254DE8(0, 0, 0);
        }
        if (D_0015EF58_254DE8[0] == 0) {
            D_0015EF58_254DE8[0] = ((Hero *)(D_0013E633 + 0xE1D))->unk894;
            D_0015EE98_254DE8 += 0x3E8;
        } else if (((Hero *)(D_0013E633 + 0xE1D))->unk894 < D_0015EF58_254DE8[0]) {
            D_0015EF58_254DE8[0] = ((Hero *)(D_0013E633 + 0xE1D))->unk894;
            D_0015EE98_254DE8 += 0x3E8;
        }
        if (D_0015EF68_254DE8[0] < ((Hero *)(D_0013E633 + 0xE1D))->unk8A8) {
            D_0015EF68_254DE8[0] = ((Hero *)(D_0013E633 + 0xE1D))->unk8A8;
            D_0015EE98_254DE8 += 0x3E8;
        }
        D_L05_00160098_254DE8[d->f28].f30 = 0xFF;
        if (((Hero *)(D_0013E633 + 0xE1D))->unk880 != -1) {
            func_001FFDA0_254DE8(((Hero *)(D_0013E633 + 0xE1D))->unk880, 0);
            ((Hero *)(D_0013E633 + 0xE1D))->unk880 = -1;
        }
        func_001FFFA0_254DE8();
    } else if (D_0015EE84_2523C8 == 0x10) {
        if (d->f28 == -1) {
            return;
        }
        if (D_0013D5E7_254DE8 == 0 && ((Hero *)(D_0013E633 + 0xE1D))->unk8C0 == 1) {
            D_L05_00160098_254DE8[d->f28].fBC = 1;
        } else {
            ((Hero *)(D_0013E633 + 0xE1D))->unk890 = (u8 *)(&D_L05_00160098_254DE8[d->f28]);
            func_001FBC80_254DE8(0, 0, 0);
        }
        if (D_0015EF58_254DE8[1] == 0) {
            D_0015EF58_254DE8[1] = ((Hero *)(D_0013E633 + 0xE1D))->unk894;
            D_0015EE98_254DE8 += 0x3E8;
        } else if (((Hero *)(D_0013E633 + 0xE1D))->unk894 < D_0015EF58_254DE8[1]) {
            D_0015EF58_254DE8[1] = ((Hero *)(D_0013E633 + 0xE1D))->unk894;
            D_0015EE98_254DE8 += 0x3E8;
        }
        if (D_0015EF68_254DE8[1] < ((Hero *)(D_0013E633 + 0xE1D))->unk8A8) {
            D_0015EF68_254DE8[1] = ((Hero *)(D_0013E633 + 0xE1D))->unk8A8;
            D_0015EE98_254DE8 += 0x3E8;
        }
        D_L05_00160098_254DE8[d->f28].f30 = 0xFF;
        if (((Hero *)(D_0013E633 + 0xE1D))->unk880 != -1) {
            func_001FFDA0_254DE8(((Hero *)(D_0013E633 + 0xE1D))->unk880, 0);
            ((Hero *)(D_0013E633 + 0xE1D))->unk880 = -1;
        }
        func_001FFFA0_254DE8();
    }
}
