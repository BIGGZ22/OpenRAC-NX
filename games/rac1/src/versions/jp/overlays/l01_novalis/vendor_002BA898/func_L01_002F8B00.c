void bomber_jet_update(Moby *m) {
    Vars_F8B00 *d = (void *)m->pvars;
    Sub_F8B00 *sub = &d->sub;
    float s;
    void *hit;
    int i;
    int j;
    Quad_F8B00 a;
    Quad_F8B00 b;
    Vec4 dir;
    Vec4 pos;
    Vec4 col;
    {
        float dist = FastVecDist(&m->pos, camera_pos);
        float x0 = D_L01_00161C68;
        float k = -((1.0f - D_L01_00161C60) / (D_L01_00161C64 - x0));
        s = k * dist + (1.0f - k * x0);
    }
    hit = MobyGetHitMessage(m, 0x800000, 0);
    m->hit_slot = 0xFF;
    if (hit != 0) {
        if ((&D_0013D50F_k)[1] == 0) {
            (&D_0013D50F_k)[1] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x542D, -1);
        }
        FastVecSub(&b, d->vD0, d->vE0);
        *(u128 *)&a = *(u128 *)&b;
        func_L00_0025F4A8(m, &a, &m->pos, 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 3.0f, 1, 15.0f, 1, 1, -1, 0);
        SpawnDebrisMoby((char *)m, &a, s * 2.0f);
        for (i = 0; i < 4; i++) {
            if (d->tethers[i] != -1) {
                DeleteMoby((Moby *)(D_L01_00160058_F8B00 + (d->tethers[i] << 8)));
            }
        }
        DeleteMoby(m);
        return;
    }
    do {
        if (m->state == 0) {
            if (sub->path != -1) {
                Path_F8B00 *path = (Path_F8B00 *)D_L01_001B0C30[sub->path];
                j = 0;
                d->fFC = d->fFC * 2.0f;
                if (FastVecDist((char *)path + 0x10, (char *)path + (path->count << 4)) < 0.5f) {
                    path->count--;
                }
                for (i = 0; i < path->count; i++) {
                    if (((Path_F8B00 *)((char *)path + (i << 4)))->w == 1.0f) {
                        d->slots[j] = i;
                        j++;
                        if (j >= 4) break;
                    }
                }
                d->side = 0;
                d->last = -1;
                d->timer = -1;
                d->scale178 = m->scale;
                d->f17C = d->fFC;
            } else {
                STUB_printf(D_L01_0020B790, m->oclass, func_001FA898_r((float)(((int)m - D_L01_00160058_F8B00) >> 8)));
            }
            m->update_dist = 0xFF;
            m->draw_dist = 0xFF;
        }
        if (1.0f < s) s = 1.0f;
        if (s < D_L01_00161C60) s = D_L01_00161C60;
        m->scale = d->scale178 * s;
        d->fFC = d->f17C * s;
        if (175.0f <= m->pos.z) {
            m->alpha = 0;
        } else if (125.0f < m->pos.z && m->pos.z < 175.0f) {
            m->alpha = func_001FA898_r((175.0f - m->pos.z) * 128.0f / 50.0f);
        } else if (m->pos.z <= 125.0f) {
            m->alpha = 0x80;
        }
        if (d->flag170 != -1) {
            if (m->state == 0) {
                gunship_follow_path((char *)m);
            }
            if (d->timer == -1 && is_point_inside_clip_volume(D_0013E633 + 0xE9D, d->flag170)) {
                if (D_L01_001BBE14[(short)m->save_id] != 0
                    || (level_moby_killed[D_0015EE84][(short)m->save_id >> 5] >> (m->save_id & 0x1F)) & 1) {
                    for (j = 0; j < 4; j++) {
                        if (d->turrets[j] != -1) {
                            Moby *o = (Moby *)(D_L01_00160058_F8B00 + (d->turrets[j] << 8));
                            if (o != 0 && o->oclass == 0x1CB && o->state != 0xFE && o->state != 0xFD) {
                                Turret_F8B00 *t = (void *)o->pvars;
                                t->f1E8 = 1;
                                t->f1EC = scale_ticks(2);
                            }
                        }
                    }
                    DeleteMoby(m);
                    return;
                }
                m->flags &= 0xFFBE;
                if (d->cam174 != -1) {
                    Hero_SetMoveState(0x72, 0);
                    CameraScript(camera_points + (d->cam174 << 7) + 0x30, camera_points + (d->cam174 << 7) + 0x70, 0, 0, 0);
                    D_L01_0015F504 = 1;
                    m->collision = 0;
                    m->flags &= 0xEFFF;
                    d->timer = scale_ticks(0x1FE);
                }
            } else if (d->timer != -1) {
                if (FastDecTimer(&d->timer)) {
                    CameraScript2(0);
                    Hero_SetMoveState(0, 1);
                    D_L01_0015F504 = 0;
                    level_moby_killed[D_0015EE84][(short)m->save_id >> 5] |= 1 << (m->save_id & 0x1F);
                    moby_killed[(short)m->save_id >> 5] |= 1 << (m->save_id & 0x1F);
                    DeleteMoby(m);
                    return;
                }
                if (d->timer == scale_ticks(0x3C)) {
                    for (j = 0; j < 4; j++) {
                        if (d->turrets[j] != -1) {
                            Moby *o = (Moby *)((d->turrets[j] << 8) + D_L01_00160058_F8B00);
                            if (o->oclass == 0x1CB) {
                                Turret_F8B00 *t = (void *)o->pvars;
                                t->f1E8 = 1;
                                t->f1EC = scale_ticks(2);
                            }
                        }
                    }
                }
                gunship_follow_path((char *)m);
            } else {
                m->flags |= 0x41;
                break;
            }
        } else {
            gunship_follow_path((char *)m);
        }
        m->flags |= 0x1000;
        d->b2B = 1;
        d->b2C = 0x2F;
        if (d->last != d->sub.cur) {
            int found = -1;
            for (i = 0; i < 4; i++) {
                unsigned char *t = d->slots;
                if (d->sub.cur == t[i]) found = i;
            }
            if (found != -1) {
                int cnt;
                a = D_L01_00161C70;
                j = 0;
                b = D_L01_00161C70;
                cnt = 0;
                for (i = 0; i < 4; i++) {
                    if (d->targets[found][i] != -1) {
                        a.v[j] = d->targets[found][i];
                        j++;
                        b.v[cnt] = d->targets[found][i];
                        cnt++;
                    }
                }
                if (j == 0) break;
                if (is_point_inside_clip_volume(camera_pos, d->flag188)) break;
                for (i = j - 1; i >= 0; i--) {
                    if (func_L00_001EFFF0(camera_pos, camera_points + (a.v[i] << 7) + 0x30, 0x86, m, 0)) {
                        if (D_L01_00174340.moby == 0 || D_L01_00174340.moby != m) {
                            b.v[i] = -1;
                            cnt--;
                            break;
                        }
                    }
                }
                if (cnt != 0) {
                    int k = 0;
                    for (i = 0; i < j; i++) {
                        if (b.v[i] != -1) {
                            a.v[k] = b.v[i];
                            k++;
                        }
                    }
                    j = cnt;
                }
                if (j != 0) {
                    Moby *bm;
                    i = a.v[random_integer_below(j)];
                    d->side ^= 1;
                    Moby_GetJointWorldPos(m, d->side, &pos);
                    FastVecSub(&dir, camera_points + (i << 7) + 0x30, &pos);
                    func_L00_001FF4B0(&dir, &dir, D_0015EE6C * 160.0f);
                    col.q = *(u128 *)((i << 7) + (int)camera_points + 0x30);
                    bm = func_L01_002F7C78_F8B00(m, &pos, &dir, &col, is_point_inside_clip_volume(D_0013E633 + 0xE9D, d->flag18C), d->flag170 != -1);
                    if (bm != 0) {
                        int k = 0;
                        int *out = (int *)((char *)((void *)bm->pvars) + 0x20);
                        {
                            unsigned char *t = d->slots;
                            d->last = t[found];
                        }
                        for (i = 0; i < 8; i++) {
                            if (d->guards[i] != -1) {
                                Moby *o = (Moby *)((d->guards[i] << 8) + D_L01_00160058_F8B00);
                                if (o->oclass == 0x2BC) {
                                    Guard_F8B00 *g = (void *)o->pvars;
                                    if (g->f4 == 0 && g->f0 == 0 && random_integer_below(0x64) < 0x64
                                        && is_point_inside_clip_volume(D_0013E633 + 0xE9D, g->f8)
                                        && is_point_inside_clip_volume(camera_pos, g->f8)) {
                                        out[k] = d->guards[i];
                                        k++;
                                    }
                                    if (k >= 4) break;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (d->timer > scale_ticks(0x1FE) - scale_ticks(0x78));
}
