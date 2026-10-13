/* sound_update: once per frame. Flushes the voice commands and reverb, measures the
   listener's velocity from its position history and sets the group volumes. Then for each
   voice it works out the radial velocity, distance volume and occlusion, and starts, updates
   or stops its sound. Ends with the music update and the command flush. */
typedef int u128_DD68 __attribute__((mode(TI)));
typedef union {
    u128_DD68 q;
    float f[4];
} VoiceVector_DD68;
typedef struct {
    unsigned char pad00[0x18];
    unsigned char source_state;
    unsigned char attenuation_flags;
    short sound_index;
    int bank_handle;
} VoiceDefinition_DD68;
typedef struct {
    unsigned char pad00[0x10];
    VoiceVector_DD68 position;
    unsigned char state;
    unsigned char pad21[0x9F];
    VoiceVector_DD68 rotation;
} VoiceMoby_DD68;
typedef struct {
    unsigned int handle;
    unsigned char state;
    unsigned char flags;
    unsigned char pad6[2];
    VoiceDefinition_DD68 *definition;
    int pad0C;
    int volume;
    int pitch_bend;
    VoiceMoby_DD68 *moby;
    int owner_context;
    VoiceVector_DD68 position;
    VoiceVector_DD68 position_offset;
    unsigned int occlusion_history_position;
    unsigned char occlusion_history[36];
    unsigned char pad68[8];
} VoiceSlot_DD68;
typedef struct {
    VoiceVector_DD68 listener_history[4];
    int listener_history_position;
    int pad44;
    int group_volume[6];
    int pad60;
    int reverb_depth;
    unsigned char reverb_type;
    unsigned char reverb_delay;
    unsigned char reverb_feedback;
    unsigned char reverb_commands;
    int pending_commands;
    VoiceSlot_DD68 voices[30];
    int padD90[2];
    int occlusion_frame;
    int padD9C;
    VoiceVector_DD68 occlusion_samples[6];
} VoiceRuntime_DD68;

extern VoiceRuntime_DD68 D_0013E650_DD68 __asm__("D_0013E650") NOT_SDA;
extern float D_0013F740_DD68 __asm__("D_0013F740") NOT_SDA;
extern int D_001873D4_DD68 __asm__("D_001873D4") NOT_SDA;
extern int D_0015F6C8_DD68 __asm__("D_0015F6C8") NOT_SDA;
extern int D_0015F6E8_DD68 __asm__("D_0015F6E8") NOT_SDA;
extern int D_0015F6F0_DD68 __asm__("D_0015F6F0") NOT_SDA;
extern float D_0015EE6C_DD68 __asm__("D_0015EE6C") NOT_SDA;
extern VoiceVector_DD68 D_00187180_DD68 __asm__("D_00187180") NOT_SDA;
extern unsigned char D_00187390_DD68[] __asm__("D_00187390") NOT_SDA;
extern VoiceVector_DD68 D_0013E6E0_DD68[] __asm__("D_0013E6E0") NOT_SDA;

extern void func_0012DDC0(void);
extern void func_0012E348(int, int);
extern void func_0012F0A8(int, int, int, int, int);
extern void func_0012F120(int, int, int, int);
extern void func_001F99B0(void *, int, int);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9C78(void *, void *);
extern float func_001F9CB8(void *);
extern void func_001F9DC0(void *, void *, float);
extern void func_001F9EC0(void *, void *, void *);
extern int func_001F9F60(void *, int);
extern float func_001FA888(int);
extern int func_001FA898_DD68(float) __asm__("func_001FA898");
extern void func_001FA4A0(void *, void *);
extern void func_00217130(void);
extern void func_0012EC30(void);
extern void func_0012EC40(void);
extern int func_0012F2E0(int);
extern void func_0012E448(int, int, int, int, int, int, void *, void *);
extern void func_0012E4A8(int);
extern void func_0012E588(int, void *, void *);
extern void func_0012E600(int, int, int, int, int, int, void *, void *);
extern void func_0022D8C0(void *);
extern int func_0022D970(void *, void *);
extern int func_0022DB00_DD68(void *, void *) __asm__("func_0022DB00");
extern int func_0022DB48_DD68(void *, void *, void *) __asm__("func_0022DB48");
extern void func_0022F0A8(int, long);
extern void func_0022F0F0(int, long);

int func_0022DD68(void) {
    VoiceRuntime_DD68 *rt = &D_0013E650_DD68;
    VoiceVector_DD68 listener_occlusion_position;
    VoiceVector_DD68 scratch;
    VoiceVector_DD68 listener_velocity;
    VoiceVector_DD68 relative_velocity;
    VoiceVector_DD68 direction;
    VoiceVector_DD68 listener_matrix[4];
    int flags[30];
    int volumes[30];
    float radial_velocities[30];
    int underwater;
    float water_height;
    int history_index;
    int previous_history_index;
    int listener_sample_count;
    int group_volume;
    int history_offset;
    int sample_index;
    int repeat_index;
    int owner_removed;
    int distance_volume;
    int occluded_samples;
    int parameter_mask;
    int pan;
    int pitch_modifier;
    int pitch_bend;
    int slot_index;
    int handle;
    int command_flags;
    VoiceSlot_DD68 *voice;
    VoiceMoby_DD68 *moby;
    int *voice_flags;
    int *volume_p;

    underwater = D_001873D4_DD68;
    water_height = 0.0f;
    if (underwater != 0) {
        water_height = D_0013F740_DD68;
    }
    func_0012DDC0();

    if (rt->reverb_commands & 8) {
        func_0012F0A8(2, 0, 0, 0, 0);
    } else if (rt->reverb_commands & 0x13) {
        func_0012F0A8(2, rt->reverb_type, rt->reverb_depth, rt->reverb_delay, rt->reverb_feedback);
    } else if (rt->reverb_commands & 4) {
        func_0012F120(2, rt->reverb_depth, 0xC, 3);
    }
    rt->reverb_commands = 0;

    scratch.q = 0;
    listener_occlusion_position.q = 0;
    if ((D_0015F6E8_DD68 == 0 || D_0015F6E8_DD68 == 2) && D_0015F6C8_DD68 == 0) {
        func_0022D8C0(&scratch);
    }

    listener_velocity.q = 0;
    {
        history_index = rt->listener_history_position + 1;
        history_index = (history_index < 0 ? history_index + 3 : history_index) % 4;
        rt->listener_history_position = history_index;
    }
    listener_sample_count = 0;
    rt->listener_history[history_index] = D_00187180_DD68;
    previous_history_index = (history_index + 3) % 4;
    if (previous_history_index != history_index) {
        do {
            func_001F9BF0(&relative_velocity, &rt->listener_history[history_index],
                          &rt->listener_history[previous_history_index]);
            if (!(func_001F9CB8(&relative_velocity) < D_0015EE6C_DD68 * 60.0f)) {
                break;
            }
            listener_sample_count++;
            func_001F9BD8(&listener_velocity, &listener_velocity, &relative_velocity);
            history_index = previous_history_index;
            previous_history_index = (history_index + 3) % 4;
        } while (previous_history_index != rt->listener_history_position);
    }
    if (listener_sample_count >= 2) {
        func_001F9C30(&listener_velocity, &listener_velocity,
                      1.0f / func_001FA888(listener_sample_count));
    }

    if (D_0015F6E8_DD68 == 2) {
        func_0012E348(1, 0);
        func_0012E348(0, rt->group_volume[0] / 2);
        func_0012E348(3, rt->group_volume[3] / 2);
    } else {
        group_volume = rt->group_volume[1];
        if (underwater) {
            group_volume = group_volume * 3 / 5;
        }
        func_0012E348(1, group_volume);
        func_0012E348(0, rt->group_volume[0]);
        func_0012E348(3, rt->group_volume[3]);
    }
    func_0012E348(2, rt->group_volume[2]);
    func_0012E348(4, rt->group_volume[4]);
    func_0012E348(5, rt->group_volume[5]);

    voice_flags = flags;
    volume_p = volumes;
    func_001F99B0(voice_flags, 0, 0x78);
    func_001F99B0(volume_p, 0, 0x78);
    func_001F99B0(radial_velocities, 0, 0x78);

    for (slot_index = 0; slot_index < 30; slot_index++) {
        voice = &rt->voices[slot_index];
        if (voice->state != 7) {
            if (voice->handle == 0) {
                continue;
            }
            if (voice->handle == (unsigned int)-1) {
                continue;
            }
        }
        moby = voice->moby;
        owner_removed = 0;
        if (moby != (void *)0) {
            if (moby->state == 0xFE || moby->state == 0xFD) {
                owner_removed = 1;
            }
        }
        if (owner_removed) {
            voice->moby = (void *)0;
        }
        if (voice->state == 4 ||
            (voice->state != 6 && owner_removed && voice->definition->source_state != 0)) {
            voice_flags[slot_index] = 0x20;
            continue;
        }
        if (D_0015F6E8_DD68 != 0 && D_0015F6E8_DD68 != 2 && D_0015F6E8_DD68 != 6 &&
            voice->state != 7) {
            voice_flags[slot_index] = 0x10;
            continue;
        }

        if (voice->moby != (void *)0 && !(voice->flags & 8)) {
            if (voice->flags & 0x40) {
                func_001F9EC0(&direction, &voice->position_offset, &moby->rotation);
                func_001F9BD8(&direction, &direction, &moby->position);
                func_001F9BF0(&relative_velocity, &direction, &voice->position);
                voice->position = *(VoiceVector_DD68 *)&direction;
            } else {
                voice->position.f[2] -= 1.0f;
                func_001F9BF0(&relative_velocity, &voice->moby->position, &voice->position);
                voice->position = voice->moby->position;
                voice->position.f[2] += 1.0f;
            }
        } else {
            relative_velocity.q = 0;
        }
        func_001F9BF0(&relative_velocity, &relative_velocity, &listener_velocity);
        func_001F9BF0(&direction, &D_00187180_DD68, &voice->position);
        func_001F9DC0(&direction, &direction, 1.0f);
        radial_velocities[slot_index] = func_001F9C78(&direction, &relative_velocity);

        if (!(voice->flags & 0x10)) {
            distance_volume = func_0022DB00_DD68(voice, &voice->position);
            volumes[slot_index] = distance_volume * voice->volume / 1024;
            if (!(voice->definition->attenuation_flags & 2)) {
                voice_flags[slot_index] |= 8;
            }
        } else {
            distance_volume = 0x400;
            volumes[slot_index] = voice->volume;
        }
        if (underwater && !(voice->definition->attenuation_flags & 4) &&
            water_height < voice->position.f[2]) {
            volumes[slot_index] /= 2;
        }
        voice_flags[slot_index] |= 1;
        if (distance_volume < 0x20 && volumes[slot_index] < 0x20 && (voice->flags & 4)) {
            voice_flags[slot_index] = 0x20;
            continue;
        }
        if ((voice->flags & 1) != 1) {
            voice_flags[slot_index] |= 2;
            if (!(voice->flags & 0x20)) {
                voice_flags[slot_index] |= 4;
            }
        }
    }

    for (slot_index = 0; slot_index < 30; slot_index++) {
        voice = &rt->voices[slot_index];
        if (!(voice_flags[slot_index] & 8)) {
            continue;
        }
        if (voice->state == 7) {
            if ((D_0015F6E8_DD68 != 0 && D_0015F6E8_DD68 != 2) || D_0015F6C8_DD68 != 0) {
                continue;
            }
            if (D_0015F6F0_DD68 != rt->occlusion_frame) {
                for (history_offset = 0; history_offset < 6; history_offset++) {
                    func_0022D8C0(&rt->occlusion_samples[history_offset]);
                }
                rt->occlusion_frame = D_0015F6F0_DD68;
            }
            occluded_samples = 0;
            for (history_offset = 0, sample_index = 0; history_offset < 36;
                 history_offset += 6, sample_index++) {
                voice->occlusion_history[history_offset] =
                    func_0022D970(voice, &rt->occlusion_samples[sample_index]);
                for (repeat_index = 1; repeat_index < 6; repeat_index++) {
                    voice->occlusion_history[history_offset + repeat_index] =
                        voice->occlusion_history[history_offset];
                }
                if (voice->occlusion_history[history_offset]) {
                    occluded_samples += 6;
                }
            }
            if (occluded_samples >= 36) {
                volumes[slot_index] = 0;
            } else if (occluded_samples >= 19) {
                volumes[slot_index] = (36 - occluded_samples) * volumes[slot_index] / 18;
            }
        } else {
            if ((D_0015F6E8_DD68 == 0 || D_0015F6E8_DD68 == 2) && D_0015F6C8_DD68 == 0) {
                voice->occlusion_history_position = (voice->occlusion_history_position + 1) % 36;
                command_flags = (slot_index ^ D_0015F6F0_DD68) & 1;
                if (!(voice->flags & 4)) {
                    command_flags = ((D_0015F6F0_DD68 ^ slot_index) & 3) == 0;
                }
                if (command_flags) {
                    voice->occlusion_history[voice->occlusion_history_position] =
                        func_0022D970(voice, &scratch);
                } else {
                    voice->occlusion_history[voice->occlusion_history_position] =
                        voice->occlusion_history[(voice->occlusion_history_position + 35) % 36];
                }
            }
            occluded_samples = func_001F9F60(voice->occlusion_history, 36);
            if (occluded_samples >= 36) {
                volumes[slot_index] = 0;
            } else if (occluded_samples >= 19) {
                volumes[slot_index] = (36 - occluded_samples) * volumes[slot_index] / 18;
            }
        }
    }

    func_001FA4A0(listener_matrix, D_00187390_DD68);
    for (slot_index = 0; slot_index < 30; slot_index++) {
        command_flags = voice_flags[slot_index];
        if (command_flags == 0) {
            continue;
        }
        voice = &rt->voices[slot_index];
        handle = voice->handle;
        voice->handle = -1;
        if (command_flags & 0x20) {
            if (voice->state == 7) {
                voice->state = 0;
                voice->moby = (void *)0;
                voice->owner_context = 0;
            } else {
                func_0012E4A8(handle);
                voice->state = 6;
                func_0012E588(handle, func_0022F0F0, voice);
            }
        } else if (command_flags & 0x10) {
            func_0012E588(handle, func_0022F0F0, voice);
        } else {
            pitch_bend = voice->pitch_bend;
            parameter_mask = pitch_bend ? 0x11 : 1;
            pan = 0;
            pitch_modifier = 0;
            if (command_flags & 2) {
                pan = func_0022DB48_DD68(voice, &voice->position, listener_matrix);
                parameter_mask |= 6;
            }
            if (voice_flags[slot_index] & 4) {
                parameter_mask |= 8;
                pitch_modifier = func_0012F2E0(func_001FA898_DD68(radial_velocities[slot_index] * 300.0f));
            }
            if (underwater && !(voice->definition->attenuation_flags & 8)) {
                parameter_mask |= 8;
                pitch_modifier -= 0x5F4;
            }
            if (voice->state != 7) {
                func_0012E600(handle, parameter_mask, volumes[slot_index], pan, pitch_modifier,
                              pitch_bend, func_0022F0F0, voice);
            } else {
                voice->state = 1;
                func_0012E448(voice->definition->bank_handle, voice->definition->sound_index,
                              volumes[slot_index], pan, pitch_modifier, pitch_bend,
                              func_0022F0A8, voice);
            }
        }
    }

    func_00217130();
    func_0012EC40();
    func_0012DDC0();
    func_0012EC30();
    rt->pending_commands = 0;
    return 0;
}
