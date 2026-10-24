void func_L00_0029D988_r(void) {
    Vec4 a;
    Vec4 b;
    f32 m[4][4];
    s32 st;
    s32 ok;
    s32 i;
    s32 j;
    s32 k;
    s32 id;
    f32 k1;
    f32 k2;
    f32 k3;
    f32 ang;
    RowD *row;
    CutD *cs;
    CamKeyD *cam;
    f32 *rot;
    VMobD *mb;
    VMobD *m2;
    Vec4 *keys;
    s32 lim;
    u8 flag;

    switch (vendor_menu.state) {
    case 0:
    case 2:
        if (vendor_menu.f40 == 0 || vendor_menu.state == 2) {
            if ((((upd_d.flags & (2)) || ((upd_d.flags & 0x10) && (*(s32 *)(D_0013A5E0 + 0x2604) & 0x200))))) {
                d_00299108();
            }
            if ((((upd_d.flags & (2)) || ((upd_d.flags & 0x10) && (*(s32 *)(D_0013A5E0 + 0x2604) & 0x200))))) {
                d_update_visible();
                d_0022dc50();
            }
            d_timing(t0_d, 3);
            if ((((upd_d.flags & (1)) || ((upd_d.flags & 0x10) && (*(s32 *)(D_0013A5E0 + 0x2604) & 0x200))))) {
                d_002070d0();
            }
            d_timing(t1_d, 7);
            if ((((upd_d.flags & (4)) || ((upd_d.flags & 0x10) && (*(s32 *)(D_0013A5E0 + 0x2604) & 0x200))))) {
                d_00217b88();
            }
            d_timing(t2_d, 5);
        }
        st = vendor_menu.state;
        if (st == 0) {
            fade_level_d -= 0.34f;
            if (fade_level_d < 0.0f) {
                fade_level_d = 0.0f;
            }
            vendor_menu.f4++;
            if (vendor_menu.f4 >= d_scale(40) || vendor_menu.f40 != 0) {
                fade_level_d = 0.0f;
                vendor_menu.fC = 1;
                vendor_menu.state = 1;
                vendor_menu.f4 = 0;
                d_blend(vendor_menu.f1c, 3, 0, 8);
                vendor_menu.f1c->f58 = 1.0f;
                G1F20_d = 1;
                G1F1C_d = 8;
                d_voice(4, 0, vendor_menu.f1c);
            }
        } else if (st == 2) {
            ok = 0;
            if (vendor_menu.f40 == 0 && vendor_menu.f48 >= 0) {
                if (idx_d[vendor_menu.f48] >= 0) { ok = 1; }
            }
            vendor_menu.f4++;
            if (vendor_menu.f4 >= d_scale(40) || vendor_menu.f40 != 0) {
                d_blend(vendor_menu.f1c, 1, 0, 8);
                row = row_d;
                for (k = 0; k < 4; k++) {
                    if (row->b1 == 1) {
                        d_0020cb88(vendor_menu.f1c, row);
                    }
                    row++;
                }
                if (ok) {
                    d_0029ad08();
                } else {
                    d_0029ab70(0);
                    d_voice(6, 0, vendor_menu.f1c);
                }
            }
            d_0023ac78(1);
        }
        if (vendor_menu.f4 <= d_scale(36)) {
            d_001f4600(d_0029f990, vendor_menu.f1c);
        }
        if (vendor_menu.f40 == 0) {
            d_ea9d8(&vendor_menu.f60);
            d_eaa30(&vendor_menu.f70);
        }
        d_00201a28();
        if ((((upd_d.flags & (2)) || ((upd_d.flags & 0x10) && (*(s32 *)(D_0013A5E0 + 0x2604) & 0x200))))) {
            d_00297e70();
        }
        break;
    case 1:
        if (G1248_d == 0 && *(s16 *)(D_0014171B + 0x100BD) == 0 && G124C_d != 0) {
            d_0029a330();
            vendor_menu.f24 = vendor_menu.f14;
            d_0020c5f0(vendor_menu.f24, 0xC);
            vendor_menu.f24->h32 = 0x40;
            vendor_menu.f24->rot.f[2] = vendor_menu.f1c->rot.f[2];
            d_00250df8(vendor_menu.f24);
            d_0020d4f0(vendor_menu.f24, 0x202020, 0xE, 0xE, 0);
            G1248_d = 1;
            G124C_d = 0;
            d_0029b4d8(1);
        }
        d_0020d580(vendor_menu.f1c);
        d_00250df8(vendor_menu.f1c);
        if (G1248_d == 1) {
            d_mtx_apply(&vendor_menu.f24->pos, &off_a_d, vendor_menu.f1c->mtx);
            d_vadd(&vendor_menu.f24->pos, &vendor_menu.f24->pos, &vendor_menu.f1c->pos);
            d_0020d580(vendor_menu.f24);
            d_00250df8(vendor_menu.f24);
            d_00237ed0(vendor_menu.f24);
        }
        k1 = -0.05f;
        k2 = -0.36f;
        d_mtx_apply(&vendor_menu.f20[0].pos, &off_c_d, vendor_menu.f1c->mtx);
        k3 = 0.05f;
        d_vadd(&vendor_menu.f20[0].pos, &vendor_menu.f20[0].pos, &vendor_menu.f1c->pos);
        vendor_menu.f20[0].rot.f[0] = d_addang(vendor_menu.f20[0].rot.f[0], d_scalef(k1));
        vendor_menu.f20[0].rot.f[1] = k2;
        vendor_menu.f20[0].rot.f[2] = d_addang(vendor_menu.f20[0].rot.f[2], d_scalef(k3));
        d_00250df8(&vendor_menu.f20[0]);
        d_mtx_apply(&vendor_menu.f20[5].pos, &off_d_d, vendor_menu.f1c->mtx);
        d_vadd(&vendor_menu.f20[5].pos, &vendor_menu.f20[5].pos, &vendor_menu.f1c->pos);
        vendor_menu.f20[5].rot.f[0] = d_addang(vendor_menu.f20[5].rot.f[0], d_scalef(k1));
        vendor_menu.f20[5].rot.f[1] = k2;
        vendor_menu.f20[5].rot.f[2] = d_addang(vendor_menu.f20[5].rot.f[2], d_scalef(k3));
        d_00250df8(&vendor_menu.f20[5]);
        G1F24_d = d_addang(G1F24_d, d_scalef(k3));
        d_0020d580(&vendor_menu.f20[1]);
        if (!vendor_menu.f20[1].b53 && (vendor_menu.f20[1].b70 & 2)) {
            d_blend(&vendor_menu.f20[1], 1, 0, 8);
        }
        d_00250df8(&vendor_menu.f20[1]);
        b.q = 0;
        b.f[2] = d_cos(G1F24_d) / 20.0f;
        a.q = b.q;
        d_vadd(&a, &off_b_d, &a);
        d_vadd(&a, &a, &slot40_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].v20);
        d_mtx_apply(&vendor_menu.f20[2].pos, &a, vendor_menu.f1c->mtx);
        d_vadd(&vendor_menu.f20[2].pos, &vendor_menu.f20[2].pos, &vendor_menu.f1c->pos);
        vendor_menu.f20[2].rot.f[0] = slot40_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f10;
        vendor_menu.f20[2].rot.f[1] = slot40_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f14;
        vendor_menu.f20[2].rot.f[2] = G1F24_d;
        d_00250df8(&vendor_menu.f20[2]);
        b.q = 0;
        b.f[2] = d_cos(G1F24_d) / 20.0f;
        a.q = b.q;
        d_vadd(&a, &off_b_d, &a);
        d_vadd(&a, &a, &slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].v20);
        d_mtx_apply(&vendor_menu.f20[3].pos, &a, vendor_menu.f1c->mtx);
        d_vadd(&vendor_menu.f20[3].pos, &vendor_menu.f20[3].pos, &vendor_menu.f1c->pos);
        vendor_menu.f20[3].rot.f[0] = slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f10;
        vendor_menu.f20[3].rot.f[1] = slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f14;
        vendor_menu.f20[3].rot.f[2] = G1F24_d;
        d_0020d580(&vendor_menu.f20[3]);
        if (!vendor_menu.f20[3].b53 && (vendor_menu.f20[3].b70 & 2)) {
            d_blend(&vendor_menu.f20[3], 1, 0, 8);
        }
        d_00250df8(&vendor_menu.f20[3]);
        b.f[0] = slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f0;
        b.f[1] = slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f4;
        b.f[2] = slot30_d[(vendor_menu.fD0[vendor_menu.f58]).fD0].f8;
        d_001fa030(vendor_menu.f20[3].mtx, &vendor_menu.f20[3].rot);
        d_mtx_apply(&b, &b, vendor_menu.f20[3].mtx);
        d_vadd(&vendor_menu.f20[3].pos, &vendor_menu.f20[3].pos, &b);
        vendor_menu.f4++;
        if (vendor_menu.f5c == 0 && G1F18_d == 0) {
            if (vendor_menu.f210 < 8) {
                if ((pad_state.pressed & 0x2000)) {
                    if (vendor_menu.f58 < vendor_menu.f210 - 1) {
                        G1F34_d = 1;
                        d_0029b4d8(0);
                        vendor_menu.f58++;
                        d_voice(1, 0, vendor_menu.f1c);
                        d_status(d_helptext(D_L00_001C2488[(vendor_menu.fD0[vendor_menu.f58]).fD0].pad[3]));
                        if (vendor_menu.f50 != -1) {
                            d_meter_free(vendor_menu.f50, 0);
                        }
                        vendor_menu.f50 = -1;
                        if ((vendor_menu.fD0[vendor_menu.f58]).fD4 == 1) {
                            vendor_menu.f50 = d_meter(0x30, (vendor_menu.fD0[vendor_menu.f58]).fD0 + 0xEA60, d_cb1, d_cb2, d_cb3, &((*(s32 *)(menu_item_enabled_115 + 0x21 + ((vendor_menu.fD0[vendor_menu.f58]).fD0) * 4))), D_L00_001C43B0[(vendor_menu.fD0[vendor_menu.f58]).fD0].hE);
                        }
                    }
                }
                if ((pad_state.pressed & 0x8000)) {
                    if (vendor_menu.f58 > 0) {
                        G1F34_d = 1;
                        d_0029b4d8(0);
                        vendor_menu.f58--;
                        d_voice(1, 0, vendor_menu.f1c);
                        d_status(d_helptext(D_L00_001C2488[(vendor_menu.fD0[vendor_menu.f58]).fD0].pad[3]));
                        if (vendor_menu.f50 != -1) {
                            d_meter_free(vendor_menu.f50, 0);
                        }
                        vendor_menu.f50 = -1;
                        if ((vendor_menu.fD0[vendor_menu.f58]).fD4 == 1) {
                            vendor_menu.f50 = d_meter(0x30, (vendor_menu.fD0[vendor_menu.f58]).fD0 + 0xEA60, d_cb1, d_cb2, d_cb3, &((*(s32 *)(menu_item_enabled_115 + 0x21 + ((vendor_menu.fD0[vendor_menu.f58]).fD0) * 4))), D_L00_001C43B0[(vendor_menu.fD0[vendor_menu.f58]).fD0].hE);
                        }
                    }
                }
            } else {
                if ((pad_state.pressed & 0x2000)) {
                    if (vendor_menu.f4c <= 0x38) {
                        G1F34_d = 1;
                        vendor_menu.f4c += 0x38;
                        d_voice(1, 0, vendor_menu.f1c);
                        d_0029b4d8(0);
                        if (vendor_menu.f58 < vendor_menu.f210 - 1) {
                            vendor_menu.f58++;
                        } else {
                            vendor_menu.f58 = 0;
                        }
                        d_status(d_helptext(D_L00_001C2488[(vendor_menu.fD0[vendor_menu.f58]).fD0].pad[3]));
                        if (vendor_menu.f50 != -1) {
                            d_meter_free(vendor_menu.f50, 0);
                        }
                        vendor_menu.f50 = -1;
                        if ((vendor_menu.fD0[vendor_menu.f58]).fD4 == 1) {
                            vendor_menu.f50 = d_meter(0x30, (vendor_menu.fD0[vendor_menu.f58]).fD0 + 0xEA60, d_cb1, d_cb2, d_cb3, &((*(s32 *)(menu_item_enabled_115 + 0x21 + ((vendor_menu.fD0[vendor_menu.f58]).fD0) * 4))), D_L00_001C43B0[(vendor_menu.fD0[vendor_menu.f58]).fD0].hE);
                        }
                    }
                }
                if ((pad_state.pressed & 0x8000)) {
                    if (vendor_menu.f4c >= -0x38) {
                        G1F34_d = 1;
                        vendor_menu.f4c -= 0x38;
                        d_voice(1, 0, vendor_menu.f1c);
                        d_0029b4d8(0);
                        if (vendor_menu.f58 > 0) {
                            vendor_menu.f58--;
                        } else {
                            vendor_menu.f58 = vendor_menu.f210 - 1;
                        }
                        d_status(d_helptext(D_L00_001C2488[(vendor_menu.fD0[vendor_menu.f58]).fD0].pad[3]));
                        if (vendor_menu.f50 != -1) {
                            d_meter_free(vendor_menu.f50, 0);
                        }
                        vendor_menu.f50 = -1;
                        if ((vendor_menu.fD0[vendor_menu.f58]).fD4 == 1) {
                            vendor_menu.f50 = d_meter(0x30, (vendor_menu.fD0[vendor_menu.f58]).fD0 + 0xEA60, d_cb1, d_cb2, d_cb3, &((*(s32 *)(menu_item_enabled_115 + 0x21 + ((vendor_menu.fD0[vendor_menu.f58]).fD0) * 4))), D_L00_001C43B0[(vendor_menu.fD0[vendor_menu.f58]).fD0].hE);
                        }
                    }
                }
            }
            if ((pad_state.pressed & 0x60) && (vendor_menu.fD0[vendor_menu.f58]).fD8 == 0) {
                vendor_menu.f5c = 1;
                d_voice(0, 0, vendor_menu.f1c);
            }
            if ((pad_state.pressed & 0x10) && G124C_d == 0) {
                G1F14_d = 8;
                G1F18_d = 1;
                d_voice(5, 0, vendor_menu.f1c);
            }
        }
        d_0029ba78();
        if (G1F20_d == 1) {
            if (G1F1C_d == 0) {
                G1F20_d = 0;
            } else {
                G1F1C_d--;
            }
        }
        if (G1F18_d == 1) {
            if (--G1F14_d == 0) {
                G15F6BC_d = 1;
                d_002335a0();
                d_blend(vendor_menu.f1c, 4, 9, 8);
                vendor_menu.f1c->f58 = G15EE60_d * -0.5f;
                if (vendor_menu.f40 != 0) {
                    qcopy((char *)&cam_d.pos + 0x10, &vendor_menu.f70);
                }
                ((Vec4 *)cam_d.basis)[0].q = snap_d[0].q;
                ((Vec4 *)cam_d.basis)[1].q = snap_d[1].q;
                ((Vec4 *)cam_d.basis)[2].q = snap_d[2].q;
                vendor_menu.state = 2;
                vendor_menu.f8 = 0;
                vendor_menu.f4 = 0;
                if (((HeroD *)(D_0013E633 + 0xE1D))->moby != 0) {
                    d_00204a40(((HeroD *)(D_0013E633 + 0xE1D))->moby->oclass, -1);
                    ((HeroD *)(D_0013E633 + 0xE1D))->moby->pclass =
                        cls_d[((HeroD *)(D_0013E633 + 0xE1D))->moby->class_id];
                    d_0020c880(((HeroD *)(D_0013E633 + 0xE1D))->moby);
                }
            }
        } else if (G1F34_d != 0) {
            d_0029b680();
        }
        d_0023ac78(1);
        break;
    case 3:
        d_00299108();
        d_update_visible();
        d_0022dc50();
        d_002070d0();
        d_00217b88();
        cut_d.f38++;
        cut_d.f34++;
        if (cut_d.h40 - 12 < cut_d.f34) {
            fade_level_d += 0.1f;
            if (fade_level_d > 1.0f) {
                fade_level_d = 1.0f;
            }
        } else {
            fade_level_d -= 0.125f;
            if (fade_level_d < 0.0f) {
                fade_level_d = 0.0f;
            }
        }
        cs = &cut_d;
        if (cs->f34 >= cs->h40) {
            fade_level_d = 0.0f;
            d_002335a0();
            G16CBF0_d = 0.63f;
            d_001f2d98();
            for (j = 0; j < cut_d.h44; j++) {
                m2 = cut_d.f178[j];
                if (m2 != 0) {
                    m2->pclass->count--;
                    m2->pclass->slots[m2->pclass->count] = 0;
                    d_0020c828(m2);
                }
            }
            if (vendor_menu.f28 != 0) {
                d_0029af80();
            } else {
                d_0029ab70(1);
            }
            break;
        }
        lim = G15EE80_d ? 0x50 : 0x60;
        if (cs->f38 >= lim) {
            cs->f3C++;
            d_002049f0(cs->f3C);
        }
        cam = &cs->f54[cs->f38];
        rot = cam->rot;
        flag = cam->flag;
        G16CBF0_d = rot[3];
        d_001f2d98();
        qcopy(cam_d.pos.f, cam);
        d_mtx_apply(cam_d.pos.f, cam_d.pos.f, vendor_menu.f80);
        d_vadd(cam_d.pos.f, cam_d.pos.f, &vendor_menu.fC0);
        ang = d_addang(rot[2], vendor_menu.f80[3].f[2]);
        d_mat_id(m);
        d_mat_a(m, m, rot[0]);
        d_mat_b(m, m, rot[1]);
        d_mat_c(m, m, ang);
        cam_d.basis[0] = -m[2][0];
        cam_d.basis[4] = -m[0][0];
        cam_d.basis[8] = m[1][0];
        cam_d.basis[1] = -m[2][1];
        cam_d.basis[5] = -m[0][1];
        cam_d.basis[9] = m[1][1];
        cam_d.basis[2] = -m[2][2];
        cam_d.basis[6] = -m[0][2];
        cam_d.basis[10] = m[1][2];
        for (i = 0; i < cut_d.h44; i++) {
            mb = cut_d.f178[i];
            id = cut_d.f38 >> 1;
            mb->next_frame = id + 1;
            mb->frame = id;
            d_0020c880(mb);
            mb->blend = d_001fa6c0(cut_d.f38 & 1) * 0.5f;
            if (flag && (cut_d.f38 & 1)) {
                mb->blend = 1.0f;
            }
            keys = mb->keys;
            d_vscale(&a, &keys[mb->frame], 1.0f - mb->blend);
            d_vscale(&b, &keys[mb->next_frame], mb->blend);
            d_vadd(&mb->pos, &a, &b);
            d_mtx_apply(&mb->pos, &mb->pos, vendor_menu.f80);
            d_vadd(&mb->pos, &mb->pos, &vendor_menu.fC0);
            mb->rot.f[2] = vendor_menu.f80[3].f[2];
            mb->b71 = 0xFF;
            d_00250df8(mb);
            if (mb->b7F) {
                d_00213700(mb);
            }
            if (mb->oclass == 0) {
                d_00234b38(mb);
            }
        }
        d_00201a28();
        d_00297e70();
        break;
    }
    d_0022ca50();
}
