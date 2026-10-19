/* NON_MATCHING func_L00_002CF808 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: BYTES 965/3776 (74.4% of the bytes match), checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-19 (lead, by hand): p0 SIZE 3752 -> p3 BYTES 1427 (roster read as fixed offsets from D_0013F450 via a
 *   padded view struct) -> p10 (roster loop as slots[k]/exclusions[count]: strength reduction reproduces retail's
 *   a0/a1/a2 loop) -> p13 BYTES 984 (random % 3 folded into one local) -> p22 BYTES 965 (block-local temporaries f
 *   every per-state moby/float, so they stay in caller-saved regs like retail). Left: FE/FD constants in fp/s7 in
 *   state 3 (ours rematerialises 0xFE in the hit check), effect pointer copied to s7, delay placement of category 
 *   in the state 2 hit check (retail threads the first null test past the class read), mov.s f13 order in the two
 *   func_L00_0025F4A8 calls, second rotation call's argument order.
 */
typedef struct {
    Vec4 position;            /* 0x00: movement this frame */
    Vec4 offset;              /* 0x10: last target position */
    f32 strength;             /* 0x20 */
    f32 elevation;            /* 0x24 */
    f32 yaw;                  /* 0x28 */
    f32 unused2c;
    struct Moby *target;      /* 0x30 */
    f32 pitch;                /* 0x34 */
    f32 unused38;
    f32 height;               /* 0x3C */
    f32 desired_yaw;          /* 0x40 */
    f32 unused44;
    s32 target_oclass;        /* 0x48 */
    f32 unused4c;
    f32 unused50;
    s16 slot;                 /* 0x54 */
    s16 health;               /* 0x56 */
    f32 yaw_velocity;         /* 0x58 */
    f32 pitch_velocity;       /* 0x5C */
} DroneVars_2CF808;
/* The hero's drone bookkeeping at D_0013F450 + 0x1FF4. */
typedef struct {
    u8 pad_0[0x1FF4];
    u8 counter;               /* 0x1FF4 */
    u8 unk1FF5;
    u8 ammo_used;             /* 0x1FF6: drones alive */
    u8 ammo_capacity;         /* 0x1FF7: drones still orbiting */
    f32 phase;                /* 0x1FF8 */
    struct Moby *exclusions[9]; /* 0x1FFC */
    struct Moby *slots[6];    /* 0x2020 */
} DroneSlots_2CF808;
extern f32 D_0015EE60_2CF808 __asm__("D_0015EE60") MACRO_ADDR;
extern f32 D_0015EE6C_2CF808 __asm__("D_0015EE6C") MACRO_ADDR;
extern f32 D_0015EE70_2CF808 __asm__("D_0015EE70") MACRO_ADDR;
extern Vec4 D_L00_00166EC0_2CF808 __asm__("D_L00_00166EC0");
extern Vec4 D_L00_00173F70_2CF808 __asm__("D_L00_00173F70");
extern f32 func_001FA748_2CF808(f32, f32) __asm__("func_001FA748");
extern f32 func_001FA790_2CF808(f32, f32) __asm__("func_001FA790");
extern f32 func_001FA888_2CF808(s32) __asm__("func_001FA888");
extern s32 func_001FA898_2CF808(f32) __asm__("func_001FA898");
extern f32 func_001F9D48_2CF808(void *, void *) __asm__("func_001F9D48");
extern f32 func_L00_001FF860_2CF808(f32, f32) __asm__("func_L00_001FF860");
extern f32 func_001F9F90_2CF808(f32) __asm__("func_001F9F90");
extern f32 func_001F9FA8_2CF808(f32) __asm__("func_001F9FA8");
extern void func_001FA218_2CF808(void *, void *) __asm__("func_001FA218");
extern void func_001F9EE8_2CF808(void *, void *, void *) __asm__("func_001F9EE8");
extern void func_L00_001FF500_2CF808(void *, void *, f32) __asm__("func_L00_001FF500");
extern void func_L00_001FF548_2CF808(void *, void *, f32) __asm__("func_L00_001FF548");
extern s32 func_001F9938_2CF808(void *) __asm__("func_001F9938");
extern s32 func_L00_001F10E0_2CF808(f32, void *, s32, void *) __asm__("func_L00_001F10E0");
extern s32 func_L00_00259868_2CF808(f32, f32, f32, void *, void *, s32) __asm__("func_L00_00259868");
extern void func_L00_0025A890_2CF808(f32, void *, void *, s32) __asm__("func_L00_0025A890");
extern void func_L00_0025AAC0_2CF808(void *, void *) __asm__("func_L00_0025AAC0");
extern void func_L00_0025B040_2CF808(f32, void *) __asm__("func_L00_0025B040");
extern void *func_L00_0025B478_2CF808(void *, s32, s32) __asm__("func_L00_0025B478");
extern f32 func_L00_00259148_2CF808(f32 *, f32, f32, f32, f32, f32) __asm__("func_L00_00259148");
extern void func_L00_0025F4A8_2CF808(f32, f32, f32, f32, f32, f32, f32, void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32) __asm__("func_L00_0025F4A8");
extern void func_0022ED80_2CF808(s32, s32, void *) __asm__("func_0022ED80");
extern s32 func_001160D8_2CF808(void) __asm__("func_001160D8");
extern void func_0020D678_2CF808(void *) __asm__("func_0020D678");
extern struct Moby *func_L00_002CF3D8_2CF808(void *, void *) __asm__("func_L00_002CF3D8");
extern char *func_L00_0025D390_2CF808(void *) __asm__("func_L00_0025D390");
extern void func_L00_002CF328_2CF808(void *) __asm__("func_L00_002CF328");
extern void func_001F9BD8_2CF808(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0_2CF808(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_001F9C30_2CF808(void *, void *, f32) __asm__("func_001F9C30");

/* Update of a drone moby (class 479, the Glove of Doom): hides while the level is paused or in a cutscene, keeps the
   hero's drone roster (slots, exclusions, counts) and hands the nearest free drone a target, then by state: 2 orbit
   the hero at the assigned yaw and pitch, 3 home on the target and hit it, 4 fade out onto the target, 5 free the slot. */
void func_L00_002CF808(struct Moby *m) {
    DroneVars_2CF808 *v = (DroneVars_2CF808 *)m->pvars;
    Vec4 collision_data[1], old_pos;
    union {
        struct { Vec4 direction, rotated; } vectors;
        u8 effect[0x20];
    } scratch;
    Vec4 target_pos, matrix[4];
    struct Moby *target;
    struct Moby *closest;
    struct Moby *other;
    void *hit;
    f32 f, distance;
    s32 i, count, category, random, health;
    s32 state;
    struct Moby **slots, **exclusions;
    struct Moby *drone;
    struct Moby **slot;
    s32 k, n;

    if (D_L00_0015F6A8_2C9820 == 2 || D_L00_0015F6A8_2C9820 == 5 ||
        D_L00_0015F6A8_2C9820 == 6 || D_0013F450_C9820.state.current == 0x32) {
        m->flags |= 0x41;
        return;
    }
    m->flags &= 0xFFBE;
    if (m->state < 5) {
        ((DroneSlots_2CF808 *)&D_0013F450_C9820)->counter++;
        if (((DroneSlots_2CF808 *)&D_0013F450_C9820)->ammo_used < ((DroneSlots_2CF808 *)&D_0013F450_C9820)->counter) {
            closest = 0;
            count = 0;
            distance = 1000000.0f;
            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->counter = 1;
            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->ammo_capacity = 0;
            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->phase = func_001FA748_2CF808(((DroneSlots_2CF808 *)&D_0013F450_C9820)->phase, D_0015EE60_2CF808 * 0.05f);
            for (k = 0; k < 6; k++) {
                drone = ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots[k];
                if (drone != 0) {
                    if (drone->oclass == 0x1DF) {
                        if (drone->state == 0xFE) {
                            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots[k] = 0;
                        } else if (drone->state == 0xFD) {
                            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots[k] = 0;
                        } else {
                            ((DroneSlots_2CF808 *)&D_0013F450_C9820)->exclusions[count] = *(struct Moby **)(drone->pvars + 0x30);
                            count++;
                            if (drone->state < 4)
                                ((DroneSlots_2CF808 *)&D_0013F450_C9820)->ammo_capacity++;
                            if (count > 8)
                                break;
                        }
                    } else {
                        ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots[k] = 0;
                    }
                }
            }
            for (n = count; n < 9; n++)
                ((DroneSlots_2CF808 *)&D_0013F450_C9820)->exclusions[n] = 0;
            other = func_L00_002CF3D8_2CF808(&D_0013F450_C9820.motion, ((DroneSlots_2CF808 *)&D_0013F450_C9820)->exclusions);
            if (other != 0) {
                char *h;
                h = func_L00_0025D390_2CF808(other);
                qcopy(&old_pos, &other->pos);
                if (h != 0)
                    old_pos.f[2] += *(f32 *)(h + 0x10);
                slots = ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots;
                for (i = 5; i >= 0; i--, slots++) {
                    drone = *slots;
                    if (drone != 0) {
                        struct Moby *t = *(struct Moby **)(drone->pvars + 0x30);
                        if (t == 0 || t->state == 0xFE || t->state == 0xFD) {
                            f32 d = func_001F9D48_2CF808(&drone->pos, &old_pos);
                            if (d < distance) {
                                closest = drone;
                                distance = d;
                            }
                        }
                    }
                }
                if (closest != 0) {
                    DroneVars_2CF808 *assigned = (DroneVars_2CF808 *)closest->pvars;
                    assigned->target = other;
                    assigned->target_oclass = other->oclass;
                    h = func_L00_0025D390_2CF808(other);
                    qcopy(&old_pos, &other->pos);
                    if (h != 0)
                        old_pos.f[2] += *(f32 *)(h + 0x10);
                    qcopy(&assigned->offset, &old_pos);
                    closest->state = 3;
                    random = func_001160D8_2CF808() % 3;
                    func_0022ED80_2CF808(random + (random > 0), 0, m);
                    ((DroneSlots_2CF808 *)&D_0013F450_C9820)->exclusions[count] = assigned->target;
                }
            }
        }
        {
            s32 capacity = ((DroneSlots_2CF808 *)&D_0013F450_C9820)->ammo_used;
            s32 divisor = capacity ? capacity : 1;
            s32 next = capacity + 1;
            s32 counter = ((DroneSlots_2CF808 *)&D_0013F450_C9820)->counter;
            if (next == 0)
                next = 1;
            v->desired_yaw = func_001FA888_2CF808(counter) / (f32)divisor * 6.2831855f - 3.1415927f;
            v->pitch = (func_001FA888_2CF808((counter * (next / 5)) % next) / (f32)divisor * 6.2831855f - 3.1415927f) * 0.25f;
        }
        {
            void *hit = func_L00_0025B478_2CF808(m, 1, 0);
            if (hit != 0) {
                char *h = *(char **)((char *)hit + 0x20);
                s32 cat = h && *(char **)(h + 0x24) ? *(s16 *)(*(char **)(h + 0x24) + 0x46) : 0;
                if (cat > 8) goto dispatch_state;
                if (cat >= 5) {
                    if (hit == 0 || h == 0 || *(s16 *)(h + 0xA6) != 0x4D6 || func_001F9938_2CF808(&v->health))
                        m->state = 4;
                }
            }
        }
    }
dispatch_state:
    state = m->state;
    if (state == 3) goto state3;
    if (state < 4) {
        if (state == 2) goto state2;
    } else {
        if (state == 4) goto state4;
        if (state == 5) goto state5;
    }
    goto update_effects;
state2:
    {
        m->collision = 0;
        qcopy(&old_pos, &m->pos);
        func_L00_002CF328_2CF808(m);
        {
            f32 step = D_0015EE60_2CF808 * 0.05f;
            m->scale += (m->pclass->scale - m->scale) * step;
            v->strength = v->strength + (1.0f - v->strength) * step;
        }
        f = func_001FA748_2CF808(v->desired_yaw, ((DroneSlots_2CF808 *)&D_0013F450_C9820)->phase);
        distance = func_001FA790_2CF808(f, v->yaw);
        v->yaw = func_001FA748_2CF808(v->yaw, distance * (D_0015EE60_2CF808 * 0.05f));
        distance = func_001FA790_2CF808(v->pitch, v->elevation);
        v->elevation = func_001FA748_2CF808(v->elevation, distance * (D_0015EE60_2CF808 * 0.01f));
        scratch.vectors.direction.f[0] = func_001F9F90_2CF808(v->yaw) * v->strength;
        scratch.vectors.direction.f[1] = func_001F9FA8_2CF808(v->yaw) * v->strength;
        scratch.vectors.direction.f[2] = 0.0f;
        scratch.vectors.rotated.f[0] = 0.0f;
        scratch.vectors.rotated.f[1] = v->elevation;
        scratch.vectors.rotated.f[2] = 0.0f;
        scratch.vectors.rotated.f[3] = 0.0f;
        func_001FA218_2CF808(matrix, &scratch.vectors.rotated);
        func_001F9EE8_2CF808(&scratch.vectors.direction, &scratch.vectors.direction, matrix);
        func_001F9BD8_2CF808(&target_pos, &D_0013F450_C9820.motion, (u8 *)D_0013F450_C9820.moby + 0xE0);
        target_pos.f[0] += scratch.vectors.direction.f[0];
        target_pos.f[1] += scratch.vectors.direction.f[1];
        v->height = v->height + (target_pos.f[2] - v->height) * (D_0015EE60_2CF808 * 0.05f);
        target_pos.f[2] = v->height + scratch.vectors.direction.f[2];
        func_001F9BF0_2CF808(&target_pos, &target_pos, &m->pos);
        func_L00_001FF548_2CF808(&target_pos, &target_pos, D_0015EE6C_2CF808 * 10.0f);
        if (func_L00_00259868_2CF808(0.0f, 0.18f, 0.0f, m, &target_pos, 0) & 1) {
            if (!m->visible) {
                Vec4 *hitv = &D_L00_00166EC0_2CF808;
                func_001F9BD8_2CF808(&m->pos, hitv, hitv + 0x23);
                func_001F9BF0_2CF808(&m->pos, &m->pos, hitv + 0x21);
            }
        }
        qcopy(&collision_data[0], &m->pos);
        if (func_L00_001F10E0_2CF808(0.2f, &collision_data[0], 0, m)) {
            struct Moby *hm, *hm2, *hm3;
            hm = (struct Moby *)D_L00_00173F40_C9820.moby;
            if (hm && hm->oclass == 0 && hm->state != 0xFE && hm->state != 0xFD)
                goto update_rotations;
            category = 0;
            hm2 = (struct Moby *)D_L00_00173F40_C9820.moby;
            if (hm2 && hm2->pclass)
                category = *(u8 *)((u8 *)hm2->pclass + 0x46);
            hm3 = (struct Moby *)D_L00_00173F40_C9820.moby;
            if (hm3 && hm3->state != 0xFE && hm3->state != 0xFD &&
                (category == 5 || category == 7 || category == 8)) {
                func_L00_0025A890_2CF808(1.0f, scratch.effect, m, 0x10000);
                func_L00_001FF500_2CF808(scratch.effect, &v->position, 1.0f);
                *(f32 *)(scratch.effect + 8) = 1.0f;
                *(f32 *)(scratch.effect + 12) = 5627.925f;
                scratch.effect[0x18] = 1;
                scratch.effect[0x19] = 2;
                *(u16 *)(scratch.effect + 0x1A) = m->oclass;
                func_L00_0025AAC0_2CF808((struct Moby *)D_L00_00173F40_C9820.moby, scratch.effect);
                if (category == 5) {
                    if (D_L00_00173F40_C9820.moby && ((struct Moby *)D_L00_00173F40_C9820.moby)->oclass == 0x4D6 && (health = v->health) > 0) {
                        random = func_001FA898_2CF808(*(f32 *)(scratch.effect + 0x1C));
                        if (random > 0)
                            random = health - func_001FA898_2CF808(*(f32 *)(scratch.effect + 0x1C));
                        else
                            random = health - 1;
                        v->health = random;
                    } else {
                        func_L00_0025F4A8_2CF808(0, 0, 0.4f, 0.2f, 4.0f, 1.0f, 8.0f, m, v, 0, 0, 1, 1, 1, 1, 1, -1, 0);
                        m->state = 4;
                    }
                }
            }
        }
update_rotations:
        func_001F9BF0_2CF808(&v->position, &m->pos, &old_pos);
        {
            f32 yaw = func_L00_001FF860_2CF808(m->pos.x - old_pos.f[0], m->pos.y - old_pos.f[1]);
            f32 pitch = -func_L00_001FF860_2CF808(func_001F9D48_2CF808(&old_pos, &m->pos), m->pos.z - old_pos.f[2]);
            m->rot.z = func_L00_00259148_2CF808(&v->yaw_velocity, m->rot.z, yaw, D_0015EE70_2CF808 * 6.2831855f, D_0015EE70_2CF808 * 3.1415927f, D_0015EE6C_2CF808 * 6.2831855f);
            m->rot.y = func_L00_00259148_2CF808(&v->pitch_velocity, m->rot.y, pitch, D_0015EE70_2CF808 * 6.2831855f, D_0015EE70_2CF808 * 3.1415927f, D_0015EE6C_2CF808 * 6.2831855f);
        }
        goto update_effects;
    }
state3:
    {
        struct MobyClass *pclass = m->pclass;
        m->collision = pclass->collision;
        qcopy(&old_pos, &m->pos);
        m->scale += (pclass->scale - m->scale) * (D_0015EE60_2CF808 * 0.05f);
        m->collision = pclass->collision;
        func_L00_002CF328_2CF808(m);
        target = v->target;
        if (target != 0 && target->oclass == v->target_oclass && target->state != 0xFE && target->state != 0xFD) {
            char *h = func_L00_0025D390_2CF808(target);
            qcopy(&target_pos, &v->target->pos);
            if (h != 0)
                target_pos.f[2] += *(f32 *)(h + 0x10);
            func_001F9BF0_2CF808(&scratch.vectors.direction, &target_pos, &v->offset);
            func_001F9C30_2CF808(&scratch.vectors.direction, &scratch.vectors.direction, 3.1f);
            func_001F9BD8_2CF808(&scratch.vectors.rotated, &target_pos, &scratch.vectors.direction);
            func_001F9BF0_2CF808(&v->position, &scratch.vectors.rotated, &m->pos);
            func_001F9C30_2CF808(&v->position, &v->position, D_0015EE60_2CF808 * -0.8f + 1.0f);
            func_001F9BD8_2CF808(&m->pos, &m->pos, &v->position);
            qcopy(&v->offset, &target_pos);
            qcopy(&collision_data[0], &m->pos);
            if (func_L00_001F10E0_2CF808(0.25f, &collision_data[0], 0, m)) {
                struct Moby *hm = (struct Moby *)D_L00_00173F40_C9820.moby;
                if (!hm || hm->state == 0xFE || hm->state == 0xFD || hm != v->target || hm->oclass == 0) {
                    qcopy(&m->pos, &D_L00_00173F70_2CF808);
                } else {
                    s32 cat2 = hm->pclass ? *(u8 *)((u8 *)hm->pclass + 0x46) : 0;
                    func_L00_0025A890_2CF808(1.0f, scratch.effect, m, 0x10000);
                    func_L00_001FF500_2CF808(scratch.effect, &v->position, 1.0f);
                    *(f32 *)(scratch.effect + 8) = 1.0f;
                    *(f32 *)(scratch.effect + 12) = 5627.925f;
                    scratch.effect[0x18] = 1;
                    scratch.effect[0x19] = 2;
                    *(u16 *)(scratch.effect + 0x1A) = m->oclass;
                    func_L00_0025AAC0_2CF808((struct Moby *)D_L00_00173F40_C9820.moby, scratch.effect);
                    if (cat2 == 5) {
                        if (D_L00_00173F40_C9820.moby && ((struct Moby *)D_L00_00173F40_C9820.moby)->oclass == 0x4D6 && (health = v->health) > 0) {
                            random = func_001FA898_2CF808(*(f32 *)(scratch.effect + 0x1C));
                            if (random > 0)
                                random = health - func_001FA898_2CF808(*(f32 *)(scratch.effect + 0x1C));
                            else
                                random = health - 1;
                            v->health = random;
                        } else {
                            func_L00_0025F4A8_2CF808(0, 0, 0.4f, 0.2f, 4.0f, 1.0f, 8.0f, m, v, 0, 0, 1, 1, 1, 1, 1, -1, 0);
                            m->state = 4;
                        }
                    }
                }
            }
        }
        else {
            m->state = 2;
            v->target_oclass = -1;
            v->target = 0;
        }
        m->rot.z = func_L00_001FF860_2CF808(m->pos.x - old_pos.f[0], m->pos.y - old_pos.f[1]);
        m->rot.y = -func_L00_001FF860_2CF808(func_001F9D48_2CF808(&old_pos, &m->pos), m->pos.z - old_pos.f[2]);
        goto update_effects;
    }
state4:
    {
        target = v->target;
        if (target == 0) goto fade_clear;
        if (target->oclass != v->target_oclass) goto fade_clear;
        if (target->state == 0xFE) goto fade_clear;
        if (target->state == 0xFD) goto fade_clear;
        {
            char *h = func_L00_0025D390_2CF808(target);
            qcopy(&old_pos, &v->target->pos);
            if (h != 0)
                old_pos.f[2] += *(f32 *)(h + 0x10);
            func_001F9BF0_2CF808(&v->position, &old_pos, &m->pos);
            func_001F9C30_2CF808(&v->position, &v->position, D_0015EE60_2CF808 * -0.8f + 1.0f);
            func_L00_001FF548_2CF808(&v->position, &v->position, D_0015EE60_2CF808 * 0.9f);
            func_001F9BD8_2CF808(&m->pos, &m->pos, &v->position);
        }
        goto fade_alpha;
fade_clear:
        v->target = 0;
fade_alpha:
        m->alpha -= 4;
        if (m->alpha < 6)
            m->state = 5;
        goto update_effects;
    }
state5:
    {
        ((DroneSlots_2CF808 *)&D_0013F450_C9820)->ammo_used--;
        ((DroneSlots_2CF808 *)&D_0013F450_C9820)->slots[v->slot] = 0;
        func_0020D678_2CF808(m);
        return;
    }
update_effects:
    func_L00_0025B040_2CF808(0.17f, m);
}
