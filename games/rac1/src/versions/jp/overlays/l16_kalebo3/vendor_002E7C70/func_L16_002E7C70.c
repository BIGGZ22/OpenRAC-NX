void gadgetron_ceo_update(L16ChallengeMoby *m) {
    L16ChallengeScratch scratch;
    L16ChallengeData *d = m->data;
    L16ChallengeActivation *activation = (L16ChallengeActivation *)d->activation;
    float rate, head_rate;
    int tracking;
    if (m->visible && FastVecDist(m->position, D_L16_00167240) < 28.0f) {
        func_L00_0025B178(m);
        m->distance_class = 22;
    }
    AttachManipulatorsIfCheat(m);
    switch (m->state) {
    case 0:
        m->opacity = 255;
        m->state = 1;
        DialogRequest_Init(m, activation);
        *(float *)((char *)d + 0x2C) = 3.7f;
        d->initial_yaw = m->yaw;
        if (*(int *)(D_0013D605 + 0x71F) == 0) m->yaw = FastAddRots(m->yaw, 3.14159265f);
        restore_gadgetron_ceo_non_challenge_environment((char *)m);
        break;
    case 1: {
        if (((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->mode != 15)
            *(float *)D_L16_0015F718_view = 115.0f;
        else
            *(float *)D_L16_0015F718_view = 100.0f;
        if (((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->state != 29 && Npc_TryStartDialog(m, (char *)d + 0x20)) {
            m->yaw = d->initial_yaw;
            PlaceMarkerAheadOfMoby(3.5f, m);
            m->state = 2;
        }
        if (!gHaveMorphORay[9] && func_001F9D48(D_0013E633 + 0xE9D, m->position) < 3.5f) {
            int elapsed, period;
            char *statistics;
            elapsed = scale_ticks(D_0015EFA4);
            period = 600;
            statistics = D_0014171B + 0x34D;
            elapsed -= *(unsigned short *)(statistics + 0x43A) * period;
            if (elapsed > (int)((float)scale_ticks(18) * 60.0f) || *(unsigned short *)(statistics + 0x43A) * period == 0) {
                Help_Request(0x3E83, 0x87);
            } else if (scale_ticks(D_0015EFA4) / 600 > *(unsigned short *)(statistics + 0x43A)) {
                *(unsigned short *)(statistics + 0x43A) = scale_ticks(D_0015EFA4) / 600;
            }
        }
        if (activation->mode == 3) {
            char *camera;
            char *pose;
            int pending;
            pending = D_L16_0015F6B0;
            activation->previous = (unsigned short)activation->mode;
            activation->mode = 2;
            activation->pending = pending;
            m->event = 0;
            pose = D_L16_001D9B10;
            Hero_ResetToPosition(pose, pose + 0x10, 0, 1);
            camera = D_0013E633 + 0x2B1D;
            qcopy(camera, pose + 0x20);
            qcopy(camera + 0x10, pose + 0x30);
            camera[0x3AC] = 1;
            m->state = 3;
            activate_gadgetron_ceo_challenge_environment((char *)m);
        }
        if (((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->mode == 22) {
            m->state = 3;
            activate_gadgetron_ceo_challenge_environment((char *)m);
        }
        break;
    }
    case 2:
        if (D_L16_0015F6A8 != 2) {
            int previous;
            m->state = 1;
            previous = activation->previous;
            if (previous == 4) {
                start_indexed_disc_stream(0);
                m->state = previous;
            }
        }
        break;
    case 4:
        if (D_L16_0015F6A8 == 0) {
            m->state = 1;
            GiveItem(31, 1);
            memcard_Save(0, -1);
        }
        break;
    case 3:
    default: {
        L16ChallengePlayer *player = (L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D);
        unsigned char event;
        if (player->points >= 4500) {
            unsigned char *flags = (unsigned char *)menu_item_enabled_115 + 1;
            if (!flags[0x1A]) {
                flags[0x1A] = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x542D, -1);
            }
            if (D_0015EF68_challenge[1] < 4500) D_0015EF68_challenge[1] = player->points;
        }
        if (((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->mode == 0) {
            m->state = 1;
            restore_gadgetron_ceo_non_challenge_environment((char *)m);
        }
        event = m->event;
        if (event == 1) {
            m->state = event;
            Hero_ResetToPosition(D_L16_001D9B30, D_L16_001D9B30 + 0x10, 0, 1);
            m->event = 0;
            if (!D_0013D605[0xA]) {
                int pending;
                unsigned short previous;
                previous = activation->mode;
                pending = D_L16_0015F6B0;
                activation->previous = previous;
                activation->mode = 4;
                activation->pending = pending;
                activation->flag = event;
            }
            restore_gadgetron_ceo_non_challenge_environment((char *)m);
        }
        if (m->event == 2) {
            m->state = 1;
            Hero_ResetToPosition(D_L16_001D9B30, D_L16_001D9B30 + 0x10, 0, 1);
            m->event = 0;
            restore_gadgetron_ceo_non_challenge_environment((char *)m);
        }
        if (m->event == 3) {
            char *camera;
            char *pose = D_L16_001D9B10;
            m->event = 0;
            Hero_ResetToPosition(pose, pose + 0x10, 0, 1);
            camera = D_0013E633 + 0x2B1D;
            qcopy(camera, pose + 0x20);
            qcopy(camera + 0x10, pose + 0x30);
        }
        break;
    }
    }
    rate = 0.02f;
    head_rate = 0.3f;
    tracking = 0;
    if (m->animation == 0) {
        char *player = (char *)&D_0013E633_challenge + 0xE9D;
        tracking = 1;
        if (func_001F9D48(m->position, player) < 8.0f &&
            FastDiffRots(m->yaw,
                func_L00_001FF860(((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->aim[0] - m->position[0],
                    ((L16ChallengePlayer *)((char *)&D_0013E633_challenge + 0xE1D))->aim[1] - m->position[1])) < 1.5707964f) {
            if (FastVecLength(player + 0x80) > 0.01f) d->moving_timer = scale_ticks(120);
            else func_001F9908(&d->moving_timer);
        } else if (d->moving_timer) {
            d->moving_timer = 0;
            qcopy(d->target, (char *)&D_0013E633_challenge + 0xEED);
        }
        if (func_001F9908(&d->target_timer)) {
            float heading;
            d->target_timer = truncate_float_to_s32(func_001F9878(random_float_between(180.0f, 300.0f)));
            heading = FastAddRots(m->yaw, random_float_between(-90.0f, 90.0f) * 0.017453292f);
            BuildSphericalOffset(d->target, 6.0f, heading, random_float_between(0.0f, 30.0f) * 0.017453292f);
            FastVecAdd(d->target, d->target, m->position);
        }
        if (d->moving_timer) {
            qcopy(scratch.target, (char *)&D_0013E633_challenge + 0xEED);
            rate = 0.04f;
            head_rate = 0.3f;
        } else {
            qcopy(scratch.target, d->target);
        }
    }
    if (tracking) {
        float yaw;
        float pitch;
        qcopy(scratch.eye, m->position);
        scratch.eye[2] += 1.0f;
        FastVecSub(scratch.delta, scratch.target, scratch.eye);
        yaw = FastSubRots(func_L00_001FF860(scratch.delta[0], scratch.delta[1]), m->yaw);
        pitch = -func_L00_001FF860(func_001F9CE8(scratch.delta), scratch.delta[2]);
        if (yaw > 1.5707964f) yaw = 1.5707964f;
        else if (yaw < -1.5707964f) yaw = -1.5707964f;
        if (pitch > 0.5235988f) pitch = 0.5235988f;
        else if (pitch < -0.5235988f) pitch = -0.5235988f;
        d->pitch = pitch;
        d->yaw = yaw * 0.7f;
        d->head_yaw = yaw * 0.3f;
    }
    if (*(unsigned char *)&D_0015EEB0_challenge) *(float *)((char *)d + 0x150) = 2.75f;
    EaseManipulatorOffsets(rate * D_0015EE64, head_rate * D_0015EE64, m, d->body, 0);
    EaseManipulatorOffsets(rate * D_0015EE64, head_rate * D_0015EE64, m, d->head, 1);
}
