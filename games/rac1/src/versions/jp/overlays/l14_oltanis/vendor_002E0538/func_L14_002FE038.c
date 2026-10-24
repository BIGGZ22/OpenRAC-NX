void update_enemy_fighter_921(Moby *m) {
    Vec_2FE038 vt;
    Vec_2FE038 vb;
    Jet_2FE038 *d = (struct Jet_2FE038 *)m->pvars;
    unsigned short id;

    if (m->state == 0
        && (D_L14_001BBCC0_2FE038.collected[(short)(id = m->save_id)] != 0
            || (*(int *)(D_0014C290_2FE038 + (((short)id >> 5) * 4 + (D_0015EE84_m << 8))) >> (id & 0x1F)) & 1)) {
        func_L14_002FF6C0_2FE038((Moby *)(D_L14_00160098_2FCEA8 + (d->f8C << 8)));
        func_0020D678_2FE038(m);
        return;
    }
    {
        void *r = func_L00_0025B478_2FE038(m, 0x800000, 0);
        m->hit_slot = 0xFF;
        if (r != 0 && d->f8C == -1 && m->state != 3) {
            if (D_0015EE84_m == 0xE) {
                D_0015EF08_2FE038 = D_0015EF08_2FE038 + 1;
                if (D_0015EF08_2FE038 >= 3 && ((Skill_2FE038 *)D_0013D510_2FE038)->f18 == 0) {
                    ((Skill_2FE038 *)D_0013D510_2FE038)->f18 = 1;
                    func_0022EE28_2FE038(1, 0, 0);
                    func_L00_00264DB8_2FE038(0x542D, -1);
                }
            }
            m->state = 3;
            func_L00_00260108_2FE038(m, (*(Vec_2FE038 *)&m->pos), -1, 5.0f, 13.0f);
            m->flags |= 0x41;
            m->collision = 0;
            if (d->f6C != -1 && ((Voice_2FE038 *)(D_0013E650_2EFD38 + d->f6C * 0x70))->owner == m
                && ((Voice_2FE038 *)(D_0013E650_2EFD38 + d->f6C * 0x70))->on != 0) {
                func_L00_0028EBF0_2FE038(d->f6C);
            }
            d->f6C = -1;
        }
    }
    switch (m->state) {
    case 0:
        if (d->f68 == -1) {
            func_001E9730_2FE038(D_L14_001FD208_2FE038, (short)m->save_id);
            func_0020D678_2FE038(m);
            return;
        }
        if (*(int *)D_L14_001B0F30_2FE038[d->f68] == 0) {
            func_001E9730_2FE038(D_L14_001FD240_2FE038, (short)m->save_id);
            func_0020D678_2FE038(m);
            return;
        }
        if (d->f98 < 0 || d->f9C < 0 || d->fA0 < 0) {
            func_0020D678_2FE038(m);
            return;
        }
        func_L14_002FDD18_2FE038(m);
        if (d->f8C != 0) {
            m->state = 1;
            m->update_dist = 0xFF;
            m->flags |= 0x41;
            m->collision = 0;
            ((Moby *)(D_L14_00160098_2FCEA8 + (d->f8C << 8)))->flags |= 0x41;
            d->f94 = 0;
        } else {
            m->state = 2;
            m->update_dist = 0x80;
        }
        m->draw_dist = 0x80;
        d->f6C = -1;
        d->f7C = func_001F9850_2FE038(D_L14_0016205C_2FE038);
        break;
    case 1: {
        unsigned char a, b, c;
        a = D_0014C150_2EFD38[d->f98 + (D_0015EE84_m << 4)];
        if (a == 0xFF) {
            b = D_0014C150_2EFD38[d->f9C + (D_0015EE84_m << 4)];
            if (b == a) {
                c = D_0014C150_2EFD38[d->fA0 + (D_0015EE84_m << 4)];
                if (c == b && ((Hero *)(D_0013E633_2EC3E8 + 0xE1D))->frames_off_ground.s == 0
                    && ((int)((Hero *)(D_0013E633_2EC3E8 + 0xE1D))->ground_moby) == 0
                    && func_00215570_2FE038((*(Vec_2FE038 *)&((Hero *)(D_0013E633_2EC3E8 + 0xE1D))->motion.pos), d->fA4) != 0) {
                    Moby *jet = (Moby *)(D_L14_00160098_2FCEA8 + (d->f8C << 8));
                    Moby *seat = (Moby *)(D_L14_00160098_2FCEA8 + (((struct Jet_2FE038 *)jet->pvars)->f4 << 8));
                    float pitch, yaw;
                    qcopy(vt, (*(Vec_2FE038 *)&seat->pos));
                    vt[2] = vt[2] + 4.0f;
                    pitch = func_001FA850_2FE038(((Cam_2FE038 *)D_L14_00167480_2FE038)->f154,
                                                 -func_L00_001FF860_2FE038(func_001F9D48_2FE038(((Cam_2FE038 *)D_L14_00167480_2FE038)->pos, vt),
                                                                           vt[2] - ((Cam_2FE038 *)D_L14_00167480_2FE038)->pos[2]));
                    yaw = func_001FA850_2FE038(((Cam_2FE038 *)D_L14_00167480_2FE038)->f158,
                                               func_L00_001FF860_2FE038((*(Vec_2FE038 *)&seat->pos)[0] - ((Cam_2FE038 *)D_L14_00167480_2FE038)->pos[0],
                                                                        (*(Vec_2FE038 *)&seat->pos)[1] - ((Cam_2FE038 *)D_L14_00167480_2FE038)->pos[1]));
                    if (yaw < 0.48869219f && pitch < 0.34906584f
                        && func_L00_001EFFF0_2FE038(((Cam_2FE038 *)D_L14_00167480_2FE038)->pos, vt, 0x12, seat, 0) == 0) {
                        *(int *)(D_0014C290_2FE038 + (((short)m->save_id >> 5) * 4 + (D_0015EE84_m << 8))) |= 1 << (m->save_id & 0x1F);
                        D_L14_001BAF60_2FE038[(short)m->save_id >> 5] |= 1 << (m->save_id & 0x1F);
                        m->flags &= 0xFFBE;
                        m->collision = *(int *)((char *)((struct Moby *)m->pclass) + 0x10);
                        jet->flags &= 0xFFBE;
                        m->state = 2;
                        func_L10_002F6E10_2FE038(d->f90);
                        seat->draw_dist = 0xFF;
                        seat->update_dist = 0xFF;
                    }
                }
            }
        }
        break;
    }
    case 2:
        if (func_L14_002FEDA0_2FE038(m) != 0) {
            return;
        }
        if (d->f8C != 0 && d->f94 < 2) {
            if (d->f94 == 0 && d->f60 >= 0x38) {
                d->f94 = 1;
            }
            func_001F9C30_2FE038(vb, (*(Vec_2FE038 *)&m->rot_row0), 0.21f);
            func_001F9C30_2FE038(vt, (*(Vec_2FE038 *)&m->rot_row2), -1.24f);
            func_001F9BD8_2FE038(vb, vb, vt);
            func_001F9BD8_2FE038(vb, (*(Vec_2FE038 *)&m->pos), vb);
            {
            Moby *jet = (Moby *)(D_L14_00160098_2FCEA8 + (d->f8C << 8));
            qcopy((*(Vec_2FE038 *)&jet->pos), vb);
            qcopy((*(Vec_2FE038 *)&jet->rot), (*(Vec_2FE038 *)&m->rot));
            if (d->f94 == 1) {
                func_L06_00304590_2FE038(jet);
                d->f94 = 2;
            }
            }
        }
        break;
    case 3:
        if (func_001F9908_2FE038(&d->f7C) != 0) {
            Moby *p = D_L14_001B0F30_2FE038[d->f68];
            m->flags &= 0xFFBE;
            m->collision = *(int *)((char *)((struct Moby *)m->pclass) + 0x10);
            d->f7C = func_001F9850_2FE038(D_L14_0016205C_2FE038);
            m->state = 2;
            d->f60 = 0;
            d->f64 = 0;
            qcopy((*(Vec_2FE038 *)&m->pos), (*(Vec_2FE038 *)&p->pos));
            (*(Vec_2FE038 *)&m->rot)[2] = func_L00_001FF860_2FE038(*(float *)((char *)p + 0x20) - (*(Vec_2FE038 *)&p->pos)[0], *(float *)((char *)p + 0x24) - (*(Vec_2FE038 *)&p->pos)[1]);
            d->f80 = d->f84 = d->f88 = d->f70 = d->f74 = d->f78 = 0.0f;
        }
        break;
    }
    func_L14_002FE798_2FE038(m);
    if (m->state != 0 && m->state != 3 && m->state != 1 && func_L00_0028EB98_2FE038(m, d->f6C) == 0) {
        d->f6C = func_0022ED80_2FE038(0, 4, m);
    }
}
