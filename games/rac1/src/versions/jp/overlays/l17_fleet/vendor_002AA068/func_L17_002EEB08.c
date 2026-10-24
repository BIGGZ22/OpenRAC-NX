void FighterJetUpdate(Moby1379 *moby) {
    State1379 *d;
    char *r;
    float v[4];
    int st;
    int s2;

    if (moby != 0) {
        d = (State1379 *)moby->d;
        func_001F9908(&d->tDC);
        st = moby->state;
        if (st != 7 && st != 1 && st != 2) {
            r = MobyGetHitMessage(moby, 0x30001, 0);
            if (r != 0 && moby->state == 4) {
                Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
                if (*(float *)(r + 0x2C) < g->t) {
                    g->t = g->t - *(float *)(r + 0x2C);
                    if (*(int *)(r + 0x30) & 1) {
                        char *p;
                        Cam1379 *q = (Cam1379 *)D_L17_00167600;
                        float f;
                        q->f160 = 1.0f;
                        p = r + 0x10;
                        q->i168 = scale_ticks(4);
                        f = *(float *)(r + 0x2C) * D_L17_00162108;
                        BuildSphericalOffset(v, d->f64, d->v30[2], -d->v30[1]);
                        func_L00_001FF548(p, p, D_0015EE6C * 3.0f);
                        FastVecAdd(moby->pos, moby->pos, p);
                        func_L00_001FF610(v, v, p);
                        d->v30[2] = FastAddRots(d->v30[2], FastSubRots(func_L00_001FF860(v[0], v[1]), d->v30[2]) * *(float *)&D_L17_00162098_s * f);
                        d->v30[1] = FastAddRots(d->v30[1], FastSubRots(func_L00_001FF860(func_001F9CE8(v), v[2]), d->v30[1]) * *(float *)&D_L17_0016209C_s * f);
                    }
                } else {
                    Cam1379 *q;
                    BuildSphericalOffset(v, d->f64, d->v30[2], -d->v30[1]);
                    q = (Cam1379 *)D_L17_00167600;
                    q->f160 = 0.2f;
                    q->i168 = scale_ticks(5);
                    g->t = 0.0f;
                    moby->state = 7;
                    d->t62 = scale_ticks(0x1E);
                    g->f88 = g->f88 + 0.2f;
                    g->p2080->i98 = 0;
                    HeroL17_SetMoveState(0, 1);
                    g->cur = 0;
                    g->id = -1;
                    func_L00_0025F4A8(moby, v, 0, 0.0f, 0.0f, 0xA, 3, 0x10, 4.0f, 2.0f, 9.0f, -1, 1.0f, 15.0f, 1, 1, -1, 0);
                    moby->flags |= 0x41;
                }
            }
        }
        moby->a4 = 0xFF;
        {
            Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
            if (g->cur == (int)moby) {
                g->i1608 = (int)(g->t / D_L17_00162104 * 200.0f);
            }
        }
        switch (moby->state) {
        case 0:
            moby->state = 1;
            d->b60 = 0;
            moby->scale = *(float *)(moby->cls + 0x24);
            d->v20[0] = d->v30[0];
            d->v20[1] = d->v30[1];
            {
                float t = func_L00_001FF860(1.0f, D_L17_0016D470_a[0]);
                t = t + t;
                d->iC0 = 0;
                d->fAC = t;
                d->fB0 = t;
            }
            qcopy(d->v30, moby->rot);
            qcopy(d->v50, moby->rot);
            qcopy(d->v40, moby->pos);
            {
                int i = d->i104 + 0x28;
                *(char **)&D_L17_00162200 = D_L17_0015F520 + (D_L17_0016DB00 + i)->h * 16;
            }
            if (*(int *)&D_L17_00162204 == 0) {
                *(Blk400 *)D_L17_001D9E38 = **(Blk400 **)&D_L17_00162200;
            }
            *(int *)&D_L17_00162204 = 1;
            d->tF8 = 1;
            d->iE8 = 1;
            if (moby->b0 != 0xFF && D_0014C150[D_0015EE84][moby->b0] == 0xFF) {
                moby->state = 9;
            }
            if (d->i11C > 0) {
                jet_group_state(d->i11C, 0, 0, 0);
            }
            update_moby_class_flags_and_link(0x325, 1, 1, -1);
            d->i124 = -1;
            break;
        case 1: {
            Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
            int ok = 0;
            if (jet_distance(moby, g->p2080) < *(float *)&D_L17_00162060) {
                int t = g->i2084;
                if (t != 0x32 && t != 0x1D && g->b20A4 == 0 && try_set_help_message(7, 0x543B) != 0) {
                    int m = *(int *)(D_0013A5E0 + 0x2604) & 0x10;
                    ok = m != 0;
                }
            }
            if (ok) {
                Hud_ClosePopupIfActive(7);
                HeroL17_SetMoveState(0x32, 1);
                moby->state = 2;
                d->i124 = -1;
            } else if (d->f130 != 0.0f) {
                ApproachValue(&d->f130, 0.0f, D_0015EE6C * 4.0f);
                D_L17_0015F4FC = d->f130;
            }
            jet_animate((unsigned char *)moby, *(float *)&D_L17_001620D4 * D_L17_0016211C);
            break;
        }
        case 2: {
            float f;
            ApproachValue(&d->f130, 1.0f, D_0015EE6C * 4.0f);
            f = d->f130;
            D_L17_0015F4FC = f;
            if (f == 1.0f) {
                Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
                g->cur = (int)moby;
                g->id = moby->a6;
                g->n6 = D_L17_001620E4;
                g->b160E = 1;
                g->n7 = D_L17_001620E8;
                g->n8 = D_L17_001620E8;
                g->t = D_L17_001620D8;
                d->iF0 = 0;
                d->iF4 = 0xFF;
                g->f1600 = D_L17_00162104;
                g->f1604 = 100.0f;
                g->n9 = 3;
                g->nA = 3;
                g->nB = 3;
                g->b160F = 0;
                MusicRequestTrack(2, 4);
                if (d->i120 != -1) {
                    char *e = D_L17_0016016C + d->i120 * 128;
                    Hero_ResetToPosition(e + 0x30, e + 0x70, 0x32, 1);
                    {
                        int i = d->i120;
                        char *b2 = D_L17_0016016C;
                        qcopy(moby->pos, b2 + i * 128 + 0x30);
                        qcopy(moby->rot, b2 + i * 128 + 0x70);
                    }
                }
                force_help_message(6, 0);
                CameraScript(moby->pos, moby->rot, 1, 0, 0);
                d->f64 = D_L17_00162064;
                d->h68 = 0;
                d->i6C = 0;
                d->i70 = 0;
                d->i84 = 0;
                d->i80 = 0;
                moby->h32 = 0xFF;
                moby->b30 = 0xFF;
                d->v30[2] = moby->rot[2];
                d->v20[2] = d->v30[2];
                d->fB4 = D_L17_001620B4 * D_L17_0016211C;
                d->fBC = D_L17_001620B8 * D_L17_0016211C;
                d->iB8 = 0;
                g->p2080->i98 = -1;
                BuildSphericalOffset(d, D_L17_00162064, d->v30[2], -d->v30[1]);
                if (d->i11C > 0) {
                    jet_group_state(d->i11C, 1, 1, 1);
                }
                update_moby_class_flags_and_link(0x325, 0, 0, -1);
                moby->pos[2] = moby->pos[2] + 3.0f;
                moby->scale = *(float *)(moby->cls + 0x24) * D_L17_0016211C;
                g->b20A5 = 1;
                if (d->i108 == -1) {
                    d->i108 = Moby_PlaySound(0, 4, (int)moby);
                }
                jet_camera_mode();
                D_L17_0015F6E8 = 2;
                update_fighter_jet_flight(moby, d);
                moby->state = 4;
            }
            break;
        }
        case 4: {
            char *e;
            force_help_message(6, 0);
            D_L17_0015F6E8 = 2;
            e = D_0013E650 + d->i108 * 0x70;
            if (*(void **)(e + 0x88) != moby || *(unsigned char *)(e + 0x74) == 0) {
                d->i108 = -1;
                d->i108 = Moby_PlaySound(0, 4, (int)moby);
            }
            update_fighter_jet_flight(moby, d);
            if (d->iE8 <= 0) {
                d->tF8 = *(int *)&D_L17_001621F8;
                moby->state = 8;
            }
            if (d->f130 != 0.0f) {
                ApproachValue(&d->f130, 0.0f, D_0015EE6C * 4.0f);
                D_L17_0015F4FC = d->f130;
            }
            {
                Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
                if (g->b160F & 1) {
                    moby->state = 5;
                }
            }
            break;
        }
        case 5: {
            Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
            char *e;
            g->b20A5 = 0;
            g->p2080->i98 = 0;
            HeroL17_SetMoveState(0, 1);
            CameraScript2(0);
            Hud_SetBarValueById(g->h160C, 0);
            qcopy(moby->pos, d->v40);
            qcopy(moby->rot, d->v50);
            moby->scale = *(float *)(moby->cls + 0x24);
            func_0020EEE8(moby);
            e = D_L17_0016016C + d->iFC * 128;
            Hero_ResetToPosition(e + 0x30, e + 0x70, 0, 1);
            MusicRequestTrack(0, 5);
            if (d->i108 != -1) {
                char *e = D_0013E650 + d->i108 * 0x70;
                if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                    Sound_StopSlot(d->i108);
                }
            }
            d->i108 = -1;
            if (d->i10C != -1) {
                char *e = D_0013E650 + d->i10C * 0x70;
                if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                    Sound_StopSlot(d->i10C);
                }
            }
            d->i10C = -1;
            moby->state = 1;
            d->f130 = 0.99f;
            D_L17_0015F4FC = 0.99f;
            break;
        }
        case 7:
            force_help_message(6, 0);
            D_L17_0015F6E8 = 2;
            if (FastDecTimer(&d->t62) != 0) {
                clear_jet_camera_mode_latch();
                {
                    Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
                    g->p2080->i98 = 0;
                }
                Hero_OnKilled();
                CameraScript2(0);
                MusicRequestTrack(0, 5);
                qcopy(moby->pos, d->v40);
                qcopy(moby->rot, d->v50);
                func_0020EEE8(moby);
                if (d->i108 != -1) {
                    char *e = D_0013E650 + d->i108 * 0x70;
                    if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                        Sound_StopSlot(d->i108);
                    }
                }
                d->i108 = -1;
                if (d->i10C != -1) {
                    char *e = D_0013E650 + d->i10C * 0x70;
                    if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                        Sound_StopSlot(d->i10C);
                    }
                }
                d->i10C = -1;
                moby->state = 0;
            }
            break;
        case 8:
            force_help_message(6, 0);
            D_L17_0015F6E8 = 2;
            update_fighter_jet_flight(moby, d);
            if (func_001F9908(&d->tF8) != 0) {
                ApproachValue(&d->f130, 1.0f, D_0015EE6C * 4.0f);
                D_L17_0015F4FC = d->f130;
            }
            if (d->f130 >= 1.0f) {
                Glob1379 *g = (Glob1379 *)(D_0013E633 + 0xE1D);
                int i;
                g->b20A5 = 0;
                g->p2080->i98 = 0;
                HeroL17_SetMoveState(0, 1);
                CameraScript2(0);
                MusicRequestTrack(0, 5);
                i = d->i100;
                if (i == -1) {
                    STUB_printf(D_L17_001E71A0);
                    qcopy(moby->pos, d->v40);
                    qcopy(moby->rot, d->v50);
                } else {
                    char *b2 = D_L17_0016016C;
                    qcopy(moby->pos, b2 + i * 128 + 0x30);
                    qcopy(moby->rot, b2 + i * 128 + 0x70);
                    moby->pos[2] = moby->pos[2] + 0.57318f;
                }
                moby->scale = *(float *)(moby->cls + 0x24);
                func_0020EEE8(moby);
                if (d->iFC == -1) {
                    STUB_printf(D_L17_001E71D8);
                } else {
                    char *e;
                    if (d->i11C > 0) {
                        jet_group_state(d->i11C, 0, 0, 0);
                    }
                    update_moby_class_flags_and_link(0x325, 1, 1, -1);
                    e = D_L17_0016016C + d->iFC * 128;
                    Hero_ResetToPosition(e + 0x30, e + 0x70, 0, 1);
                }
                if (moby->b0 != 0xFF) {
                    jet_clear_persistent(moby->b0);
                }
                if (d->i108 != -1) {
                    char *e = D_0013E650 + d->i108 * 0x70;
                    if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                        Sound_StopSlot(d->i108);
                    }
                }
                d->i108 = -1;
                if (d->i10C != -1) {
                    char *e = D_0013E650 + d->i10C * 0x70;
                    if (*(void **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                        Sound_StopSlot(d->i10C);
                    }
                }
                d->i10C = -1;
                jet_reset_camera(gHeroPos, gHeroPos + 0x10);
                moby->state = 9;
            }
            break;
        case 9:
            if (d->f130 == 0.0f) {
                DeleteMoby(moby);
                return;
            }
            ApproachValue(&d->f130, 0.0f, D_0015EE6C * 4.0f);
            D_L17_0015F4FC = d->f130;
            break;
        case 3:
        case 6:
        case 10:
            break;
        }
        if (moby->pos[0] < 15.0f) moby->pos[0] = 15.0f;
        if (moby->pos[0] > 1008.0f) moby->pos[0] = 1008.0f;
        if (moby->pos[1] < 15.0f) moby->pos[1] = 15.0f;
        if (moby->pos[1] > 1008.0f) moby->pos[1] = 1008.0f;
        if (moby->pos[2] < 15.0f) moby->pos[2] = 15.0f;
        if (moby->pos[2] > 1008.0f) moby->pos[2] = 1008.0f;
        s2 = moby->state;
        if (s2 == 4) {
            qcopy(gHeroPos, moby->pos);
            qcopy(gHeroPos + 0x10, d->v30);
        }
        if (s2 >= 3 && s2 <= 4) {
            emit_type_74_particles_from_moby_joints(moby, d);
        }
    }
}
