void MorphorayUpdate(Moby *m) {
    Vq_2BDAF0 v0;
    Vq_2BDAF0 v[3];
    Vq_2BDAF0 mt[4];
    Ray_2BDAF0 *p;
    int flag;
    int near;
    Moby *hit;
    int r;

    flag = 0;
    if (((Hero *)D_0013F450_2BDAF0)->items[0].state == 3) {
        flag = 1;
    }
    p = (struct Ray_2BDAF0 *)m->pvars;
    if (m->state == 0) {
        m->state = 1;
        D_L00_00161890_2BDAF0 = -1;
        D_L00_00161898_2BDAF0 = -1;
        D_L00_00161894_2BDAF0 = 0;
    }
    near = 1;
    if (m->prev_seq == 0 && func_0020D830_2BDAF0(m) < 6.0f) {
        near = 0;
    }
    func_001F9938_2BDAF0(&p->timer);
    if ((((unsigned int)((Hero *)D_0013F450_2BDAF0)->state.control_mode < 2 || ((Hero *)D_0013F450_2BDAF0)->state.control_mode == 12
          || ((Hero *)D_0013F450_2BDAF0)->state.control_mode == 4 || ((Hero *)D_0013F450_2BDAF0)->state.control_mode == 5))
        && (*(int *)(D_0013A5E0_2BDAF0 + 0x2600) & ((Hero *)D_0013F450_2BDAF0)->items[0].button_mask) != 0
        && !flag && near) {
        if (((Hero *)D_0013F450_2BDAF0)->unk20A8 == 0) {
            func_L00_0020EB60_2BDAF0();
        }
        if (p->seen == 0) {
            p->seen = 1;
            if (((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->kills <= 0xFFFE) {
                ((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->kills++;
            }
            r = func_001F9850_2BDAF0(D_0015EFA4_2BDAF0) / 600;
            if (((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->best < r) {
                ((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->best = func_001F9850_2BDAF0(D_0015EFA4_2BDAF0) / 600;
            }
            ((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->levels = ((Stats_2BDAF0 *)(D_0014171B_2BDAF0 + 0x65))->levels | (1 << D_0015EE84_2BDAF0) | 0x80000000;
        }
        if (p->timer == 0) {
            p->timer = func_001F9850_2BDAF0(15);
        }
        func_L00_00250800_2BDAF0(m, 0, p);
        if (((Hero *)D_0013F450_2BDAF0)->state.current == 1) {
            func_L00_00222B80_2BDAF0(30, 1);
        }
        *(u128_2BDAF0 *)&v0 = 0;
        if (((Hero *)D_0013F450_2BDAF0)->state.current == 30
            && (*(unsigned short *)((char *)((Hero *)D_0013F450_2BDAF0)->moby + 0x34) & 1)) {
            func_001FA480_2BDAF0(v, D_L00_00166D80_2BDAF0);
            func_001FA4A0_2BDAF0(mt, v);
            v0.z = 1.0f;
            func_001F9EE8_2BDAF0(&v0, &v0, mt);
        } else if (((Hero *)D_0013F450_2BDAF0)->unk22C8 == 1) {
            func_001F9C30_2BDAF0(&v0, &m->rot_row1, -1.0f);
        } else {
            v0.x = 1.0f;
            func_001F9EC0_2BDAF0(&v0, &v0, ((Hero *)D_0013F450_2BDAF0)->basis[0].f);
        }
        p->yaw = func_L00_001FF860_2BDAF0(v0.x, v0.y);
        p->pitch = func_L00_001FF860_2BDAF0(func_001F9CE8_2BDAF0(&v0), v0.z);
        *(u128_2BDAF0 *)&v[0] = *(u128_2BDAF0 *)p->pos;
        hit = func_L00_002BD6B8_2BDAF0(m, &v[0], &p->yaw, &p->pitch);
        func_001F9908_2BDAF0(&p->delay);
        if (p->target != 0) {
            if (p->target->state == 0xFE) {
                p->target = 0;
                p->delay = 0;
            } else if (p->target->state == 0xFD) {
                p->target = 0;
                p->delay = 0;
            }
        }
        if (hit != 0 || p->delay != 0) {
        if (hit != 0) {
            float *s = func_L00_0025D390_2BDAF0(hit);
            if (hit != p->target && p->delay == 0) {
                p->target = hit;
                p->total = func_001FA888_2BDAF0(((short *)s)[2]);
                p->left = s[0];
                p->delay = func_001F9850_2BDAF0(30);
                if (p->total <= 1.0f) {
                    p->left = 0.0f;
                }
                p->snd = func_001FFB38_2BDAF0(4, 0x7533, func_L00_00236BF8_2BDAF0, func_L00_00236DE8_2BDAF0,
                                              func_L00_002377E0_2BDAF0, &D_L00_001617BC_2BDAF0, 10000);
            }
        }
            {
                float a = func_001FA888_2BDAF0(D_0013E633_2BDAF0[2]) * 0.33f + 1.0f;
                float d = p->left - D_L00_001617B0_2BDAF0 * D_0015EE6C_2BDAF0 * a;
                p->left = d;
                if (d < 0.0f) {
                    p->left = 0.0f;
                }
            }
            D_L00_001617BC_2BDAF0 = 10000 - func_001FA898_2BDAF0(p->left * 10000.0f / p->total);
        }
        if (p->target != 0 && p->left <= 0.0f) {
            if (D_0015EE84_2BDAF0 == 5 && p->target->oclass == 0x271 && ((Skill_2BDAF0 *)D_0013D510_2BDAF0)->f8 == 0) {
                ((Skill_2BDAF0 *)D_0013D510_2BDAF0)->f8 = 1;
                func_0022EE28_2BDAF0(1, 0, 0);
                func_L00_00264DB8_2BDAF0(0x53D6, -1);
            }
            func_L00_002CA728_2BDAF0(p->target);
            p->target = 0;
        }
        func_L00_002BE3A8_2BDAF0(m);
        if (D_L00_00161890_2BDAF0 == -1) {
            D_L00_00161890_2BDAF0 = func_L00_0023EF78_2BDAF0((float *)&m->pos, 7.5f, 0.0f, 0x207F7F);
        }
        if (D_L00_00161890_2BDAF0 >= 0) {
            D_L00_00161894_2BDAF0 = func_001F9850_2BDAF0(20);
            func_L00_001FF4B0_2BDAF0(&v[0], &m->rot_row1, -0.5f);
            func_001F9BD8_2BDAF0(&v[1], p, &v[0]);
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[0] = v[1].x;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[1] = v[1].y;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[2] = v[1].z;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].radius = 7.5f;
        }
        if (func_L00_0028EB98_2BDAF0(m, D_L00_00161898_2BDAF0) == 0) {
            D_L00_00161898_2BDAF0 = func_0022ED80_2BDAF0(0, 4, m);
        }
    } else {
        p->target = 0;
        p->seen = 0;
        p->delay = 0;
        if (p->snd != -1) {
            func_001FFCB0_2BDAF0(p->snd);
            p->snd = -1;
        }
        D_L00_0016188C_2BDAF0 = 1;
        if (D_L00_00161890_2BDAF0 >= 0) {
            float f;
            func_L00_001FF4B0_2BDAF0(&v0, &m->rot_row1, -0.5f);
            func_001F9BD8_2BDAF0(&v[0], p, &v0);
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[0] = v[0].x;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[1] = v[0].y;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].pos[2] = v[0].z;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].radius = 7.5f;
            f = func_001FA888_2BDAF0(D_L00_00161894_2BDAF0) * 0.05f;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].col[0] = f;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].col[1] = f;
            ((Light_2BDAF0 *)D_L00_001803C0_2BDAF0)[D_L00_00161890_2BDAF0].col[2] = f * 0.25f;
            if (func_001F9908_2BDAF0(&D_L00_00161894_2BDAF0) != 0) {
                func_L00_0023F1D0_2BDAF0(D_L00_00161890_2BDAF0);
                D_L00_00161890_2BDAF0 = -1;
            }
        }
        if (D_L00_00161890_2BDAF0 >= 0 && D_L00_00161894_2BDAF0 == 0) {
            func_L00_0023F1D0_2BDAF0(D_L00_00161890_2BDAF0);
            D_L00_00161890_2BDAF0 = -1;
        }
        if (D_L00_00161898_2BDAF0 >= 0) {
            if (D_L00_00161898_2BDAF0 != -1 && ((Voice_2BDAF0 *)(D_0013E650_2BDAF0 + D_L00_00161898_2BDAF0 * 0x70))->owner == m
                && ((Voice_2BDAF0 *)(D_0013E650_2BDAF0 + D_L00_00161898_2BDAF0 * 0x70))->on != 0) {
                func_L00_0028EBF0_2BDAF0(D_L00_00161898_2BDAF0);
            }
            D_L00_00161898_2BDAF0 = -1;
        }
        func_L00_0020ED30_2BDAF0();
    }
    if (flag && D_L00_00161890_2BDAF0 >= 0) {
        func_L00_0023F1D0_2BDAF0(D_L00_00161890_2BDAF0);
        D_L00_00161890_2BDAF0 = -1;
    }
}
