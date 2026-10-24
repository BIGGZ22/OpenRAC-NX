void update_resident_gameplay_state(void) {
    Vector4_230A90 scratch_vectors[4];
    s32 keyframe_flags;
    s32 ticks_per_bank;
    s32 skip;
    s32 object_index;
    s32 point_index;
    ResidentRenderObject_230A90 *object;
    ResidentRenderObject_230A90 *object_cursor;
    ResidentRenderObject_230A90 *expired_object;
    s32 release_index;
    s32 bank_tick_limit;
    ResidentRenderObject_230A90 *player;
    s32 path_point_index;
    f32 camera_blend_step;
    CameraKeyframe_230A90 *keyframe;
    f32 *keyframe_angles;
    u8 *animation_positions;
    s32 current_frame;
    ScriptedPath_230A90 *path;
    f32 segment_angle;
    f32 maximum_turn;
    f32 turn_scale;
    f32 blend;

    resident_data_0013E130.timer++;
    switch (resident_data_0013E130.state) {
    case 0:
    case 8:
        resident_call_001E97B0();
        resident_call_00213C78();
        resident_call_0022EF68();
        resident_call_001E97A0();
        resident_call_00218A80();
        resident_data_0015F53C -= 0.125f;
        resident_data_0018CC20.frame++;
        resident_data_0018CC20.time++;
        if (resident_data_0015F53C < 0.0f) {
            resident_data_0015F53C = 0.0f;
        }
        ticks_per_bank = resident_data_0015EE80 ? 0x50 : 0x60;
        skip = resident_data_0013E130.skip;
        if ((resident_data_0013CBE4[0] & 0x50) && resident_call_001F98C0(30) < resident_data_0018CC20.time &&
            resident_data_0015F53C == 0.0f) {
            if (resident_data_0013E130.state == 0 &&
                resident_data_0018CC20.time <
                    resident_call_001F98C0(((s32 *)&resident_duration_base)[resident_data_0013E130.content_variant] - 30)) {
                skip = 1;
            }
            if (resident_data_0013E130.state == 8 &&
                resident_data_0018CC20.time < resident_data_0018CC20.end - resident_call_001F98C0(30)) {
                skip = 1;
            }
        }
        if (skip) {
            if (resident_data_001517D0.unk5A != 6 && resident_data_001517D0.unk5A != 7) {
                resident_data_001517D0.unk5A = 5;
            }
            if (resident_data_0013E130.state == 0) {
                resident_data_0018CC20.time = resident_call_001F98C0(((s32 *)&resident_duration_base)[resident_data_0013E130.content_variant]);
                resident_data_0018CC20.sequence_frame = resident_data_0018CC20.time / ticks_per_bank;
                resident_call_00205220(resident_data_0018CC20.sequence_frame);
                resident_data_0018CC20.frame = resident_data_0018CC20.time % ticks_per_bank;
                if (resident_data_0013E130.skip) {
                    resident_data_0013E130.skip = 0;
                } else {
                    resident_call_001F4E08(4);
                }
                resident_data_0015F53C = 1.0f;
            } else {
                resident_data_0018CC20.time = resident_data_0018CC20.end;
            }
        }
        if (resident_data_0018CC20.time >= resident_data_0018CC20.end) {
            if (resident_data_001517D0.unk5A != 6 && resident_data_001517D0.unk5A != 7) {
                resident_data_001517D0.unk5A = 5;
            }
            resident_call_002348B8();
            resident_data_0018CEB0[0] = 0.63f;
            resident_call_001F3140();
            /* Retail releases the attachment once per sequence object, even when that object is null. */
            for (release_index = 0; release_index < resident_data_0018CC20.count; release_index++) {
                expired_object = resident_data_0018CC20.objects[release_index];
                if (expired_object != 0) {
                    expired_object->model->count--;
                    expired_object->model->frames[expired_object->model->count] = 0;
                    resident_call_0020D678(expired_object);
                }
                if (resident_data_0013E130.attachment != 0) {
                    resident_call_0020D678(resident_data_0013E130.attachment);
                }
            }
            resident_data_0013E130.player->flags &= ~1;
            for (object_cursor = resident_data_0016001C; object_cursor->state != 0xFF; object_cursor++) {
                if (!(object_cursor->state & 0x80) &&
                    (object_cursor->class_id == 0x4A || object_cursor->class_id == 0xCB)) {
                    object_cursor->flags &= ~0x80;
                }
            }
            if (resident_data_0013E130.state == 0) {
                resident_call_00219C70(0);
                resident_data_001D5F70 = 0xE;
                resident_data_0015F6FC[0] = 1;
                return;
            }
            resident_data_0015F6E8 = 0;
            resident_call_00216F28();
            resident_data_0015F6FC[0] = 1;
            resident_data_001414F5[0] = 0;
            resident_call_001E97B8();
            if (resident_data_0013E130.unk2C != 0) {
                resident_call_001E9788();
                return;
            }
            resident_call_001E97C0(&resident_data_0013E130.unk60, &resident_data_0013E130.unk70, 0, 1);
            return;
        }
        bank_tick_limit = resident_data_0015EE80 ? 0x50 : 0x60;
        if (resident_data_0018CC20.frame >= bank_tick_limit) {
            resident_call_00205220(++resident_data_0018CC20.sequence_frame);
        }
        keyframe = &resident_data_0018CC20.frames[resident_data_0018CC20.frame];
        keyframe_angles = keyframe->rotation;
        keyframe_flags = keyframe->position.bytes[12];
        resident_data_0018CEB0[0] = keyframe_angles[3];
        resident_call_001F3140();
        qcopy(&resident_data_00187040.position, &keyframe->position);
        resident_call_001F9EC0(&resident_data_00187040.position, &resident_data_00187040.position,
                      resident_data_0013E130.player->transform);
        resident_call_001F9BD8(&resident_data_00187040.position, &resident_data_00187040.position,
                    &resident_data_0013E130.player->position);
        segment_angle = resident_call_001FA748(keyframe_angles[2],
                                           resident_data_0013E130.player->rotation.components[2]);
        resident_call_00125358(scratch_vectors);
        resident_call_001254A0(scratch_vectors, scratch_vectors, keyframe->rotation[0]);
        resident_call_00125548(scratch_vectors, scratch_vectors, keyframe_angles[1]);
        resident_call_001253F8(scratch_vectors, scratch_vectors, segment_angle);
        resident_data_00187040.forward.components[0] = -scratch_vectors[2].components[0];
        resident_data_00187040.right.components[0] = -scratch_vectors[0].components[0];
        resident_data_00187040.up.components[0] = scratch_vectors[1].components[0];
        resident_data_00187040.forward.components[1] = -scratch_vectors[2].components[1];
        resident_data_00187040.right.components[1] = -scratch_vectors[0].components[1];
        resident_data_00187040.up.components[1] = scratch_vectors[1].components[1];
        resident_data_00187040.forward.components[2] = -scratch_vectors[2].components[2];
        resident_data_00187040.right.components[2] = -scratch_vectors[0].components[2];
        resident_data_00187040.up.components[2] = scratch_vectors[1].components[2];
        if (resident_data_0015EEB4[0] != 0) {
            resident_call_001F9CA0(&resident_data_00187040.right, &resident_data_00187040.up, &resident_data_00187040.forward);
        }
        for (object_index = 0; object_index < resident_data_0018CC20.count; object_index++) {
            object = resident_data_0018CC20.objects[object_index];
            current_frame = resident_data_0018CC20.frame >> 1;
            object->current_frame = current_frame;
            object->next_frame = current_frame + 1;
            resident_call_0020D6D0(object);
            object->blend = resident_call_001FA888(resident_data_0018CC20.frame & 1) * 0.5f;
            if (keyframe_flags != 0 && (resident_data_0018CC20.frame & 1)) {
                object->blend = 1.0f;
            }
            animation_positions = object->animation_positions;
            resident_call_001F9C30(&scratch_vectors[0], animation_positions + object->current_frame * 16,
                         1.0f - object->blend);
            resident_call_001F9C30(&scratch_vectors[1], animation_positions + object->next_frame * 16,
                         object->blend);
            resident_call_001F9BD8(&object->position, &scratch_vectors[0], &scratch_vectors[1]);
            resident_call_001F9EC0(&object->position, &object->position,
                          resident_data_0013E130.player->transform);
            resident_call_001F9BD8(&object->position, &object->position, &resident_data_0013E130.player->position);
            object->rotation.components[2] = resident_data_0013E130.player->rotation.components[2];
            object->cached_selector = 0xFF;
            resident_call_0020ED48(object);
            if (object->class_id == 0) {
                if (resident_data_0013E130.state == 0) {
                    if (D_0015EE84_305A0 == 10 && resident_data_0013D5C8[6] == 0) {
                        object->unk7F = 0;
                    } else if (resident_data_0018CC20.time <= resident_call_001F98C0(0x3E)) {
                        object->unk7F = 0x18;
                    } else {
                        object->unk7F = 0;
                    }
                } else if (resident_call_001F98C0(360) < resident_data_0018CC20.time) {
                    if (D_0015EE84_305A0 == 10 && resident_data_0013D5C8[6] == 0) {
                        object->unk7F = 0;
                    } else {
                        object->unk7F = 0x18;
                    }
                } else {
                    object->unk7F = 0;
                }
            } else if (object->class_id == 10) {
                if (resident_data_0013E130.state == 0) {
                    if (resident_data_0018CC20.time <= resident_call_001F98C0(350)) {
                        object->unk7F = 0x18;
                    } else {
                        object->unk7F = 0;
                    }
                } else if (resident_data_0018CC20.time <= resident_call_001F98C0(524)) {
                    object->unk7F = 0;
                } else {
                    object->unk7F = 0x18;
                }
            }
            if (object->unk7F != 0) {
                resident_call_00214550(object);
            }
            if (object->class_id == 10) {
                resident_call_001E9798(object);
            }
            if (object->class_id == 0) {
                resident_call_001E9790(object);
                if ((D_0015EE84_305A0 == 10 && resident_data_0013D5C8[6] != 0) ||
                    D_0015EE84_305A0 == 13) {
                    if (resident_data_0013E130.attachment == 0) {
                        resident_data_0013E130.attachment = resident_call_0020D348(0x509);
                        resident_data_0013E130.attachment->selected_index = 0x40;
                        resident_data_0013E130.attachment->flags |= 0x806;
                        resident_data_0013E130.attachment->lifetime_stamp = object->lifetime_stamp;
                        if (resident_data_0013E130.attachment->model->unk6 != 0) {
                            resident_data_0013E130.attachment->unk73 = 0x18;
                        }
                    }
                    resident_call_001E97D0(object, resident_data_0013E130.attachment);
                }
            }
            if ((u16)object->class_id - 0x213U < 3) {
                resident_call_001F4B68(resident_call_00233AB8, object);
                resident_call_001F49B0(resident_call_0022F258, object);
                if (resident_data_0013E130.state == 0) {
                    if (resident_data_0018CC20.time < resident_call_001F98C0(360)) {
                        s32 color_intensity =
                            (s32)(resident_call_001F9F90(
                                      resident_call_001FA888((resident_data_0018CC20.time & 0x3F) - 32) *
                                      0.09817477f) *
                                  80.0f) +
                            120;
                        if (object->class_id == 0x215) {
                            object->color = (color_intensity >> 1) | (color_intensity << 8) |
                                            (color_intensity << 16);
                        } else {
                            object->color =
                                color_intensity | (color_intensity << 8) | (color_intensity << 16);
                        }
                    } else if (resident_data_0018CC20.time < resident_call_001F98C0(0x1F8)) {
                        s32 color_intensity =
                            (s32)(((f32)resident_data_0018CC20.time - resident_call_001F98B0(360.0f)) *
                                  (resident_data_0015EE60 * 1.25f));
                        if (color_intensity >= 256) {
                            color_intensity = 255;
                        }
                        if (object->class_id == 0x215) {
                            object->color = (color_intensity << 8) | (color_intensity << 16);
                        } else {
                            object->color =
                                color_intensity | (color_intensity << 8) | (color_intensity << 16);
                        }
                        if (resident_call_001F98C0(0x1A4) < resident_data_0018CC20.time) {
                            resident_data_0013E130.player->unkBC =
                                (s32)(((f32)resident_data_0018CC20.time - resident_call_001F98B0(420.0f)) *
                                      (resident_data_0015EE60 * 1.2f));
                            resident_data_0013E130.player->unkB2 = 0;
                            resident_call_001F49B0(resident_effect_entry + 8, resident_data_0013E130.player);
                        }
                        if (resident_call_001F98C0(0x1D0) < resident_data_0018CC20.time) {
                            resident_data_0015F540 += 0.025f;
                            if (resident_data_0015F540 > 1.0f) {
                                resident_data_0015F540 = 1.0f;
                            }
                        }
                    } else {
                        object->color = 0xA0A0A0;
                        resident_data_0015F540 -= 0.025f;
                        if (resident_data_0015F540 < 0.0f) {
                            resident_data_0015F540 = 0.0f;
                        }
                    }
                } else if (resident_data_0018CC20.time < resident_call_001F98C0(0xF0)) {
                    s32 color_intensity;
                    if (resident_data_0018CC20.time < resident_call_001F98C0(0xC8)) {
                        resident_call_002308C8(object, resident_data_0015EE6C * -4.0f);
                        resident_call_002308C8(object, resident_data_0015EE6C * -3.0f);
                    }
                    color_intensity = (s32)(resident_call_001F9F90(resident_call_001FA888(
                                                         (resident_data_0018CC20.time & 0x3F) - 32) *
                                                     0.09817477f) *
                                            80.0f) +
                                      120;
                    if (object->class_id == 0x215) {
                        object->color = (color_intensity >> 1) | (color_intensity << 8) |
                                        (color_intensity << 16);
                    } else {
                        object->color =
                            color_intensity | (color_intensity << 8) | (color_intensity << 16);
                    }
                    object->unkBC = 0x32;
                    object->unkB2 = 10;
                    resident_call_001F49B0(resident_effect_entry + 8, object);
                } else if (resident_data_0018CC20.time <= resident_call_001F98C0(360)) {
                    s32 color_intensity;
                    if (resident_data_0018CC20.time < resident_call_001F98C0(300)) {
                        resident_data_0013E130.player->unkBC =
                            (s32)((resident_call_001F98B0(300.0f) - (f32)resident_data_0018CC20.time) *
                                  (resident_data_0015EE60 * 1.5f));
                        resident_data_0013E130.player->unkB2 = 0;
                        resident_call_001F49B0(resident_effect_entry + 8, resident_data_0013E130.player);
                    }
                    color_intensity = (s32)((resident_call_001F98B0(360.0f) - (f32)resident_data_0018CC20.time) *
                                            (resident_data_0015EE60 * 1.25f));
                    if (object->class_id == 0x215) {
                        object->color = (color_intensity << 8) | (color_intensity << 16);
                    } else {
                        object->color =
                            color_intensity | (color_intensity << 8) | (color_intensity << 16);
                    }
                } else {
                    s32 color_intensity = (s32)(resident_call_001F9F90(resident_call_001FA888(
                                                             (resident_data_0018CC20.time & 0x3F) - 32) *
                                                         0.09817477f) *
                                                80.0f) +
                                          120;
                    if (object->class_id == 0x215) {
                        object->color = (color_intensity >> 1) | (color_intensity << 8) |
                                        (color_intensity << 16);
                    } else {
                        object->color =
                            color_intensity | (color_intensity << 8) | (color_intensity << 16);
                    }
                }
            }
        }
        resident_call_0022DD68();
        resident_call_00202260();
        resident_call_0020DE20();
        resident_call_001E97A8();
        break;

    case 3:
        resident_call_001E97B0();
        resident_call_00213C78();
        resident_call_0022EF68();
        resident_call_001E97A0();
        resident_call_00218A80();
        if (resident_data_0013E130.timer == 0) {
            resident_call_00216EF0(0);
            resident_call_00217748(0);
            resident_data_001517D0.unk1C = resident_data_0013E130.content_variant + 0x9C4F;
            resident_call_001F4E08(resident_call_001F98C0(12));
            while ((s16)resident_data_001517D0.unk5A != 3) {
                resident_call_0022DD68();
                resident_call_00122598(0);
            }
            resident_call_00216960();
            resident_data_0013E130.player->selected_a = 1;
            resident_data_0013E130.player->selected_b = 2;
            resident_data_0013E130.player->blend = 0.0f;
            resident_call_0020D6D0(resident_data_0013E130.player);
            resident_data_0013E130.player->opacity = 0xFF;
            resident_data_0013E130.player->unk94 = 0;
            if (resident_data_0013E130.path >= 0) {
                resident_data_0013E130.companion_a = resident_call_0020D348(0);
                resident_data_0013E130.companion_a->selected_index = 0x1FF;
                resident_data_0013E130.companion_a->opacity = 0xFF;
                resident_data_0013E130.companion_a->unk94 = 0;
                resident_data_0013E130.companion_a->flags |= 6;
                resident_data_0013E130.companion_a->lifetime_stamp =
                    resident_data_0013F450.reference_object->lifetime_stamp;
                resident_data_0013E130.companion_b = resident_call_0020D348(10);
                resident_data_0013E130.companion_b->selected_index = 0x1FF;
                resident_data_0013E130.companion_b->opacity = 0xFF;
                resident_data_0013E130.companion_b->unk94 = 0;
                resident_data_0013E130.companion_b->flags |= 6;
                resident_data_0013E130.companion_b->lifetime_stamp =
                    resident_data_0013F450.reference_object->lifetime_stamp;
            }
            resident_call_001E9770(&resident_data_00187040.position, &resident_data_00187040.unk150, 1, 0, 0);
            resident_data_0013E130.blend = 0.0f;
            resident_data_0013E130.interpolation_velocity = 0.0f;
            if (resident_data_0013E130.path >= 0) {
                path = resident_data_001CC730[resident_data_0013E130.path];
                maximum_turn = resident_data_0013E130.blend;
                path->p[0].components[3] = maximum_turn;
                for (path_point_index = 1; path_point_index < path->point_count - 1;
                     path_point_index++) {
                    segment_angle = resident_call_001FA058(path->p[path_point_index].components[0] -
                                                      path->p[path_point_index - 1].components[0],
                                                  path->p[path_point_index].components[1] -
                                                      path->p[path_point_index - 1].components[1]);
                    path->p[path_point_index].components[3] = resident_call_001FA790(
                        resident_call_001FA058(path->p[path_point_index + 1].components[0] -
                                          path->p[path_point_index].components[0],
                                      path->p[path_point_index + 1].components[1] -
                                          path->p[path_point_index].components[1]),
                        segment_angle);
                    if (maximum_turn < resident_call_001F9B88(path->p[path_point_index].components[3])) {
                        maximum_turn = resident_call_001F9B88(path->p[path_point_index].components[3]);
                    }
                }
                segment_angle = resident_call_001FA058(path->p[path->point_count - 1].components[0] -
                                                  path->p[path->point_count - 2].components[0],
                                              path->p[path->point_count - 1].components[1] -
                                                  path->p[path->point_count - 2].components[1]);
                path->p[path->point_count - 1].components[3] = resident_call_001FA790(
                    resident_call_001FA058(
                        path->p[0].components[0] - path->p[path->point_count - 1].components[0],
                        path->p[0].components[1] - path->p[path->point_count - 1].components[1]),
                    segment_angle);
                turn_scale = 0.34906584f / maximum_turn;
                for (point_index = 0; point_index < path->point_count; point_index++) {
                    path->p[point_index].components[3] *= turn_scale;
                }
                resident_data_0013E130.path_segment_length = resident_call_001F9D10(&path->p[0], &path->p[1]);
                resident_data_0013E130.path_progress = 0.0f;
                resident_data_0013E130.speed = 0.0f;
                resident_data_0013E130.trail = 0;
                resident_data_0013E130.history_count = 0;
                player = resident_data_0013E130.player;
                qcopy(&resident_data_0013E130.startPos, &player->position);
                qcopy(&resident_data_0013E130.pathPos, &path->p[0]);
                qcopy(&resident_data_0013E130.startRot, &player->rotation);
                resident_data_0013E130.rotY = resident_call_001FA058(
                    resident_call_001F9D48(&path->p[1], &path->p[0], &resident_data_0013E130.startPos),
                    path->p[0].components[2] - path->p[1].components[2]);
                resident_data_0013E130.rotZ =
                    resident_call_001FA058(path->p[1].components[0] - path->p[0].components[0],
                                  path->p[1].components[1] - path->p[0].components[1]);
            }
        }
        if (resident_data_0013E130.source_camera_index >= 0 &&
            resident_data_0013E130.destination_camera_index >= 0) {
            camera_blend_step = resident_data_0015EE70 * 0.666f;
            resident_call_00214D88(&resident_data_0013E130.blend,
                                       &resident_data_0013E130.interpolation_velocity, 1.0f,
                                       camera_blend_step, camera_blend_step, resident_data_0015EE6C * 0.5f);
            resident_call_001F9C08(&scratch_vectors[0],
                          &resident_data_00160134[resident_data_0013E130.source_camera_index].position,
                          &resident_data_00160134[resident_data_0013E130.destination_camera_index].position,
                          resident_data_0013E130.blend);
            resident_call_001F9BC0(&scratch_vectors[1]);
            scratch_vectors[1].components[1] =
                resident_call_001FA790(
                    resident_data_00160134[resident_data_0013E130.destination_camera_index].rotY,
                    resident_data_00160134[resident_data_0013E130.source_camera_index].rotY) *
                resident_data_0013E130.blend;
            scratch_vectors[1].components[1] =
                resident_call_001FA748(resident_data_00160134[resident_data_0013E130.source_camera_index].rotY,
                                   scratch_vectors[1].components[1]);
            scratch_vectors[1].components[2] =
                resident_call_001FA790(
                    resident_data_00160134[resident_data_0013E130.destination_camera_index].rotZ,
                    resident_data_00160134[resident_data_0013E130.source_camera_index].rotZ) *
                resident_data_0013E130.blend;
            scratch_vectors[1].components[2] =
                resident_call_001FA748(resident_data_00160134[resident_data_0013E130.source_camera_index].rotZ,
                                   scratch_vectors[1].components[2]);
            resident_call_001E9778(&scratch_vectors[0]);
            resident_call_001E9780(&scratch_vectors[1]);
        }
        if (resident_data_0013E130.path >= 0) {
            if (resident_data_0013E130.timer < resident_call_001F98C0(150)) {
                blend =
                    (1.0f - resident_call_001F9F90(resident_call_001FA888(resident_data_0013E130.timer) *
                                     (3.1415927f / resident_call_001FA888(resident_call_001F98C0(120))))) *
                    0.5f;
                if (resident_data_0013E130.timer >= resident_call_001F98C0(120)) {
                    blend = 1.0f;
                }
                resident_data_0013E130.player->blend = blend;
                resident_call_001F9C08(&resident_data_0013E130.player->position, &resident_data_0013E130.startPos,
                              &resident_data_0013E130.pathPos, blend);
                resident_data_0013E130.player->rotation.components[0] = 0.0f;
                resident_data_0013E130.player->rotation.components[1] = resident_call_001FA748(
                    resident_data_0013E130.startRot.components[1],
                    resident_call_001FA790(resident_data_0013E130.rotY,
                                            resident_data_0013E130.startRot.components[1]) *
                        blend);
                resident_data_0013E130.player->rotation.components[2] = resident_call_001FA748(
                    resident_data_0013E130.startRot.components[2],
                    resident_call_001FA790(resident_data_0013E130.rotZ,
                                            resident_data_0013E130.startRot.components[2]) *
                        blend);
                if (resident_data_0015F53C > 0.0f) {
                    resident_data_0015F53C -= 0.125f;
                    if (resident_data_0015F53C < 0.0f) {
                        resident_data_0015F53C = 0.0f;
                    }
                }
                if (blend < 1.0f) {
                    resident_call_002308C8(resident_data_0013E130.player, resident_data_0015EE6C * -3.75f);
                }
            } else {
                ScriptedPath_230A90 *active_path;
                resident_data_0013E130.speed += 0.8f;
                active_path = resident_data_001CC730[resident_data_0013E130.path];
                if (resident_data_0013E130.speed > 100.0f) {
                    resident_data_0013E130.speed = 100.0f;
                }
                if ((resident_data_0013CBE4[0] & 0x50) && resident_data_0015F53C < 0.0625f) {
                    resident_data_0015F53C = 0.0625f;
                }
                if ((f32)(active_path->point_count - 6) < resident_data_0013E130.path_progress &&
                    resident_data_0015F53C < 0.0625f) {
                    resident_data_0015F53C = 0.0625f;
                }
                if (resident_data_0013E130.content_variant < 2) {
                    resident_data_0013E130.player->unkBC = 0x32;
                    resident_data_0013E130.player->unkB2 = 10;
                    resident_call_001F49B0(resident_effect_entry + 8, resident_data_0013E130.player);
                }
                resident_data_0013E130.path_progress +=
                    resident_data_0013E130.speed * resident_data_0015EE6C / resident_data_0013E130.path_segment_length;
                if ((f32)(active_path->point_count - 1) < resident_data_0013E130.path_progress ||
                    resident_data_0015F53C >= 1.0f) {
                    *(s32 *)&resident_data_0015F690 = 1;
                    resident_data_0015F6FC[0] = 1;
                } else {
                    resident_call_00215CA8(active_path, 1, &resident_data_0013E130.player->position,
                                       &resident_data_0013E130.player->rotation, 0,
                                       resident_data_0013E130.path_progress);
                    resident_data_0013E130.player->rotation.components[0] =
                        resident_data_0013E130.player->rotation.components[3];
                    resident_data_0013E130.trail = (resident_data_0013E130.trail + 1) & 0x1F;
                    if (resident_data_0013E130.history_count < 0x20) {
                        resident_data_0013E130.history_count++;
                    }
                    resident_call_001F9EC0(&resident_data_0013E130.trailA[resident_data_0013E130.trail],
                                  &resident_data_001D9D40[resident_data_0013E130.content_variant].a,
                                  resident_data_0013E130.player->transform);
                    resident_call_001F9BD8(&resident_data_0013E130.trailA[resident_data_0013E130.trail],
                                &resident_data_0013E130.trailA[resident_data_0013E130.trail],
                                &resident_data_0013E130.player->position);
                    resident_data_0013E130.trailA[resident_data_0013E130.trail].components[3] = 1.0f;
                    resident_call_001F9EC0(&resident_data_0013E130.trailB[resident_data_0013E130.trail],
                                  &resident_data_001D9D40[resident_data_0013E130.content_variant].b,
                                  resident_data_0013E130.player->transform);
                    resident_call_001F9BD8(&resident_data_0013E130.trailB[resident_data_0013E130.trail],
                                &resident_data_0013E130.trailB[resident_data_0013E130.trail],
                                &resident_data_0013E130.player->position);
                    resident_data_0013E130.trailB[resident_data_0013E130.trail].components[3] = 1.0f;
                    resident_call_001F49B0(resident_call_0022F738,
                                            resident_data_0013E130.player);
                    if (resident_data_0015F53C > 0.0f) {
                        resident_data_0015F53C += 0.0625f;
                        if (resident_data_0015F53C > 1.0f) {
                            resident_data_0015F53C = 1.0f;
                        }
                    }
                }
            }
            {
                ResidentRenderObject_230A90 *p = resident_data_0013E130.player;
                ResidentRenderObject_230A90 *scratch_vectors = resident_data_0013E130.companion_a;
                qcopy(&scratch_vectors->rotation, &p->rotation);
                resident_call_001F9EC0(&scratch_vectors->position,
                              &resident_data_001DA010[resident_data_0013E130.content_variant], p->transform);
            }
            resident_call_001F9BD8(&resident_data_0013E130.companion_a->position,
                        &resident_data_0013E130.companion_a->position,
                        &resident_data_0013E130.player->position);
            resident_call_0020ED48(resident_data_0013E130.companion_a);
            {
                ResidentRenderObject_230A90 *scratch_vectors = resident_data_0013E130.companion_b;
                ResidentRenderObject_230A90 *p = resident_data_0013E130.player;
                qcopy(&scratch_vectors->rotation, &p->rotation);
                resident_call_001F9EC0(&scratch_vectors->position,
                              &resident_data_001DA040[resident_data_0013E130.content_variant], p->transform);
            }
            resident_call_001F9BD8(&resident_data_0013E130.companion_b->position,
                        &resident_data_0013E130.companion_b->position,
                        &resident_data_0013E130.player->position);
            resident_call_0020ED48(resident_data_0013E130.companion_b);
        }
        resident_call_001EDE50();
        resident_call_0022DD68();
        resident_call_00202260();
        resident_call_0020DE20();
        resident_call_001E97A8();
        break;

    case 4:
        resident_call_0022FDC0();
        break;
    case 1:
    case 2:
    case 5:
    case 6:
    case 7:
        break;
    }
}
