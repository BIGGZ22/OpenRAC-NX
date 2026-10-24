void update_gameplay_frame(void) {
    Vec4A first_position;
    Vec4A next_position;
    RenderSequenceActor_EB458 *actor;
    Vec4A *animation_positions;
    s32 actor_index;
    f32 fade;

    func_001E97B0_EB458();
    fade = D_0015F53C_EB458 - 0.0625f;
    D_0018CC20_EB458.frame++;
    D_0018CC20_EB458.time++;
    D_0015F53C_EB458 = fade;
    if (fade < 0.0f) {
        D_0015F53C_EB458 = 0.0f;
    }
    if (D_0018CC20_EB458.time >= D_0018CC20_EB458.end_time) {
        D_0018CC20_EB458.time = 0;
        D_0018CC20_EB458.sequence_frame = 0;
        func_00205220_EB458(0);
    } else if (D_0018CC20_EB458.frame >= 0x60) {
        func_00205220_EB458(++D_0018CC20_EB458.sequence_frame);
    }
    func_001EB338_EB458();
    for (actor_index = 0; actor_index < D_0018CC20_EB458.count; actor_index++) {
        u8 frame_index;
        actor = D_0018CC20_EB458.actors[actor_index];
        frame_index = D_0018CC20_EB458.frame >> 1;
        actor->current_frame = D_0018CC20_EB458.frame >> 1;
        actor->next_frame = frame_index + 1;
        func_0020D6D0_EB458(actor);
        actor->frame_fraction = func_001FA888_EB458(D_0018CC20_EB458.frame & 1) * 0.5f;
        animation_positions = ((RenderSequenceSidecar_EB458 *)actor)->animation_positions;
        func_001F9C30_EB458(&first_position, &animation_positions[actor->current_frame],
                            1.0f - actor->frame_fraction);
        func_001F9C30_EB458(&next_position, &animation_positions[actor->next_frame],
                            actor->frame_fraction);
        func_001F9BD8_EB458(&actor->position, &first_position, &next_position);
        actor->cached_frame = 0xFF;
        func_0020ED48_EB458(actor);
        actor->update_enabled = 0;
        if (actor->class_id == 0) {
            func_001E9790_EB458(actor);
        }
    }
    func_001E97A8_EB458();
    if (D_0015F6E8_EB458 == 0) {
        D_0015F058_EABE8++;
        if (func_001F98C0_EB458(0x3C) < D_0015F058_EABE8) {
            if (++D_0015F050_EB458 > 0x40) {
                D_0015F050_EB458 = 0x40;
            }
        }
        if (func_001F98C0_EB458(0x78) < D_0015F058_EABE8) {
            D_0015F054_EB458 =
                (s32)(func_001F9F90_EB458((D_0015F058_EABE8 - func_001F98C0_EB458(0x78)) % 60 * 0.10471976f +
                                          -3.1415927f) *
                      32.0f) +
                0x60;
        }
        if (D_0013CBE4_EB458[0] & 0x840) {
            func_00219E60_EB458();
        }
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 3) {
        D_0015F058_EABE8 = func_001F98C0_EB458(0x3C);
        if ((D_0015F050_EB458 -= 0x10) < 0) {
            D_0015F050_EB458 = 0;
        }
        if ((D_0015F054_EB458 -= 0x10) < 0) {
            D_0015F054_EB458 = 0;
        }
        func_0021A1A0_EB458();
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 4) {
        func_001FD3E8_EB458();
        func_0022DD68_EB458();
    }
}
