void update_batalia_item_triggers_and_help(char *m) {
    VData *data = *(VData **)(m + 0x78);
    int elapsed;
    int time;
    float volume[4];

    switch (data->state) {
    case 0:
        if (is_point_inside_clip_volume(&hero_p.motion.pos, data->third) && hero_p.state.control_mode == 15)
            data->state = 1;
        break;
    case 1:
        if (hero_p.state.current == 0x42) {
            data->state = 0;
        } else if (is_point_inside_clip_volume(&hero_p.motion.pos, data->fourth) && unl_p[0xB] == 0) {
            unl_p[0xB] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x53D6, -1);
        }
        break;
    }
    if (ia_p[29] == 0) {
        if (is_point_inside_clip_volume(&hero_p.motion.pos, data->first)) {
            elapsed = scale_ticks(D_0015EFA4) - HR[48].best * 600;
            if ((int)((float)scale_ticks(18) * 60.0f) < elapsed || HR[48].best * 600 == 0) {
                Help_Request(8000, 0x30);
            } else {
                time = scale_ticks(D_0015EFA4) / 600;
                if (HR[48].best < time)
                    HR[48].best = scale_ticks(D_0015EFA4) / 600;
            }
        }
    }
    if (ia_p[29] != 0) {
        if (is_point_inside_clip_volume(&hero_p.motion.pos, data->first) && alt_p[0x2F] == 0) {
            if (HR[49].count != 0) {
                elapsed = scale_ticks(D_0015EFA4) - HR[49].best * 600;
                if ((int)((float)scale_ticks(18) * 60.0f) < elapsed || HR[49].best * 600 == 0) {
                    Help_Request(8001, 0x31);
                } else {
                    time = scale_ticks(D_0015EFA4) / 600;
                    if (HR[49].best < time)
                        HR[49].best = scale_ticks(D_0015EFA4) / 600;
                }
            } else {
                HR[49].count++;
                time = scale_ticks(D_0015EFA4) / 600;
                if (HR[49].best < time)
                    HR[49].best = scale_ticks(D_0015EFA4) / 600;
                HR[49].flags = HR[49].flags | (1 << D_0015EE84) | 0x80000000;
            }
        }
    }
    if (ia_p[28] == 0) {
        if (is_point_inside_clip_volume(&hero_p.motion.pos, data->second) && D_0013DE4B[7] != 0) {
            elapsed = scale_ticks(D_0015EFA4) - HR[50].best * 600;
            if ((int)((float)scale_ticks(18) * 60.0f) < elapsed || HR[50].best * 600 == 0) {
                Help_Request(8002, 0x32);
            } else {
                time = scale_ticks(D_0015EFA4) / 600;
                if (HR[50].best < time)
                    HR[50].best = scale_ticks(D_0015EFA4) / 600;
            }
            goto sample_volume;
        }
    }
    if (ia_p[28] != 0) {
        if (is_point_inside_clip_volume(&hero_p.motion.pos, data->second) && D_0013DE4B[7] != 0) {
            if (HR[112].count != 0) {
                elapsed = scale_ticks(D_0015EFA4) - HR[112].best * 600;
                if ((int)((float)scale_ticks(18) * 60.0f) < elapsed || HR[112].best * 600 == 0) {
                    Help_Request(8005, 0x70);
                } else {
                    time = scale_ticks(D_0015EFA4) / 600;
                    if (HR[112].best < time)
                        HR[112].best = scale_ticks(D_0015EFA4) / 600;
                }
            } else {
                HR[112].count++;
                time = scale_ticks(D_0015EFA4) / 600;
                if (HR[112].best < time)
                    HR[112].best = scale_ticks(D_0015EFA4) / 600;
                HR[112].flags = HR[112].flags | (1 << D_0015EE84) | 0x80000000;
            }
        }
    }
sample_volume:
    *(u128_p *)volume = 0;
    volume[2] = -1.0f;
    if (FastVecDot((char *)&hero_p + 0x290, volume) < 0.1f)
        HR[112].count = -1;
    if (alt_p[0x2F] == 0 && hero_p.state.control_mode == 15)
        alt_p[0x2F] = 1;
}
