void update_kerwan_tutorial_hints(Moby *moby) {
    Data_2E08E8 *data = *(Data_2E08E8 **)&moby->pvars;

    func_001F9908(&data->f58);
    switch (moby->state) {
    case 0:
        data->f50 = 0;
        data->f54 = 0;
        moby->update_dist = 0xFF;
        moby->state = 1;
        break;
    case 1:
        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f78) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)) {
            unsigned char *g = D_0013D355 + 0x13B;
            g[0x1A] = 1;
        }

        {
            unsigned char *g = D_0013D355 + 0x13B;
            if (g[0x1A] == 0) {
                if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f0) != 0
                    && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
                    && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
                    Help_2E08E8 *b = D_00141A68_2E08E8;
                    if (!(b[0x14].mask & 0x80000000)) {
                        Help_Request(0xBB8, 0x14);
                    }
                }
                if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->fC) != 0
                    && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
                    && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
                    Help_2E08E8 *b = D_00141A68_2E08E8;
                    if (!(b[0x15].mask & 0x80000000)) {
                        Help_Request(0xBB9, 0x15);
                    }
                }
                if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f18) != 0
                    && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
                    && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
                    Help_2E08E8 *b = D_00141A68_2E08E8;
                    if (!(b[0x17].mask & 0x80000000)) {
                        Help_Request(0xBBB, 0x17);
                    }
                }
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f30) != 0) {
            Help_2E08E8 *h = D_00141948_2E08E8;
            if (h[3].count == 0
                && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)) {
                int n;
                h[3].count++;
                n = scale_ticks(D_0015EFA4) / 600;
                if (h[3].time < n) {
                    h[3].time = scale_ticks(D_0015EFA4) / 600;
                }
                h[3].mask = h[3].mask | (1 << D_0015EE84) | 0x80000000;
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f24) != 0 && D_00141948_2E08E8[3].count == 0
            && ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) >= 2 && ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) != 9
            && data->f58 == 0) {
            data->f50++;
            data->f58 = scale_ticks(0x3C);
        } else if (data->f58 != 0) {
            if (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9) {
                data->f58 = 0;
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f24) != 0 && D_00141948_2E08E8[3].count == 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && data->f50 % 6 == 0 && data->f50 != 0
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x16].count == 0) {
                Help_Request(0xBBA, 0x16);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f28) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x18].count == 0) {
                int t = scale_ticks(D_0015EFA4) - D_00141948_2E08E8[9].time * 600;
                if (scale_ticks(0x34BC0) < t) {
                    Help_Request(0xBBC, 0x18);
                }
            }
        }

        {
            Node_2E08E8 *m;
            int count = 0;
            for (m = D_L03_00160064_2E08E8; m != 0; m = m->next) {
                if (m->f24 != 0 && m->f24->f46 == 5
                    && func_001F9D48(m->pos, D_0013E633 + 0xE9D) < 8.0f) {
                    count++;
                }
            }

            if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f3C) != 0 && count == 0
                && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
                && ((Hero *)(D_0013E633 + 0xE1D))->state.current != 0x1E
                && (gHaveHeliPack + 0xD)[0] != 0
                && get_ammo(0xF) >= 0x15
                && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1
                && (D_00141A68_2E08E8)[0x4A].count == 0
                && D_00141948_2E08E8[0x13].count == 0) {
                Help_Request(0x4E20, 0x4A);
            } else if (((Hero *)(D_0013E633 + 0xE1D))->state.current == 0x1E) {
                Help_2E08E8 *h = D_00141948_2E08E8;
                unsigned int c = h[0x13].count;
                if (c <= 0xFFFE) {
                    h[0x13].count = c + 1;
                }
                if (scale_ticks(D_0015EFA4) / 600 > h[0x13].time) {
                    h[0x13].time = scale_ticks(D_0015EFA4) / 600;
                }
                h[0x13].mask = h[0x13].mask | (1 << D_0015EE84) | 0x80000000;
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f38) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && D_L03_0015F76C < 3
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x40].count == 0) {
                Help_Request(0x3EC, 0x40);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f38) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && ((Hero *)(D_0013E633 + 0xE1D))->state.current == 1
            && D_L03_0015F768 == -1
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x40].count != 0 && b[0x41].count == 0) {
                Help_Request(0x3ED, 0x41);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f40) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x54].count == 0) {
                Help_Request(0xBC0, 0x54);
            }
        }

        if (D_L03_0015F6A8 == 0 && (discount_purchase_pricing + 0x11)[0] != 0
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x19].count == 0) {
                Help_Request(0xBBD, 0x19);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f34) != 0) {
            Help_2E08E8 *h = D_00141948_2E08E8;
            if (h[0xF].count == 0
                && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)) {
                int n;
                h[0xF].count++;
                n = scale_ticks(D_0015EFA4) / 600;
                if (h[0xF].time < n) {
                    h[0xF].time = scale_ticks(D_0015EFA4) / 600;
                }
                h[0xF].mask = h[0xF].mask | (1 << D_0015EE84) | 0x80000000;
            }
        }

        if (((Hero *)(D_0013E633 + 0xE1D))->unk988 != 0 && D_00141948_2E08E8[0xF].count == 0) {
            ((Hero *)(D_0013E633 + 0xE1D))->unk988 = 0;
            data->f54++;
        }
        if (D_00141948_2E08E8[0xF].count == 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && data->f54 >= 3
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x1A].count == 0) {
                Help_Request(0xBBE, 0x1A);
            }
        }

        if (((Hero *)(D_0013E633 + 0xE1D))->state.current == 0x74) {
            Help_2E08E8 *h = D_00141948_2E08E8;
            if (h[0x10].count == 0) {
                int n;
                h[0x10].count++;
                n = scale_ticks(D_0015EFA4) / 600;
                if (h[0x10].time < n) {
                    h[0x10].time = scale_ticks(D_0015EFA4) / 600;
                }
                h[0x10].mask = h[0x10].mask | (1 << D_0015EE84) | 0x80000000;
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f2C) != 0 && D_00141948_2E08E8[0x10].count == 0
            && ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 6 && ((Hero *)(D_0013E633 + 0xE1D))->items[0].item_id == 8
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x1B].count == 0) {
                Help_Request(0xBBF, 0x1B);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f44) != 0
            && (((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2 || ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) == 9)
            && D_L03_00179590[0] == 0 && D_L03_00179590[9] == -1) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x79].count == 0) {
                Help_Request(0x4E2F, 0x79);
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f7C) != 0
            && ((unsigned int)((Hero *)(D_0013E633 + 0xE1D))->state.control_mode) < 2) {
            Help_2E08E8 *b = D_00141A68_2E08E8;
            if (b[0x4F].count == 0 && D_00141948_2E08E8[0x14].count == 0) {
                int count = 0;
                int i;
                for (i = 0; i < 37; i++) {
                    if (D_L03_00179C40[i].active == 0 && i != 8 && i != 24) {
                        if ((menu_item_enabled_115 + 0xB9)[i] != 0) {
                            count++;
                        }
                    }
                }
                if (count >= 2) {
                    Help_Request(0x4E25, 0x4F);
                }
            }
        }

        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f70) != 0) {
            unsigned char *g = D_0013D355 + 0x13B;
            g[0x18] = 1;
        }
        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, data->f74) != 0) {
            unsigned char *q = menu_item_enabled_115 + 1;
            if (q[3] == 0) {
                q[3] = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x53D6, -1);
            }
        }
        break;
    }
}
