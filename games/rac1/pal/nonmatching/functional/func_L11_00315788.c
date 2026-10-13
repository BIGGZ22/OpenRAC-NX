/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L11_00314318), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: PAL's compiler gives our 5720 bytes for retail's 5712 bytes. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_15788;
struct Moby_15788;
extern f32 D_0015EE6C MACRO_ADDR;
extern f32 D_0015EE70 MACRO_ADDR;
struct Manip;
struct GifEntry;
struct AnimSeq_15788;
struct MobyClass_15788 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq_15788 *seqs[1]; /* animation sequences, indexed by Moby.seq */
};
struct Moby_15788 {
    Vec4f_15788 bsphere;
    Vec4f_15788 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_15788 *pclass;
    struct Moby_15788 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_15788 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
    u8 frame; /* animation frame */
    u8 prev_frame; /* frame index in prev_seq */
    u8 seq; /* animation sequence id */
    u8 prev_seq;
    f32 anim_time;
    f32 anim_speed;
    u8 pad5C[8];
    struct Manip *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8 anim_flags;
    u8 unk71; /* set to 0xFF when a moby changes class */
    u8 unk72;
    u8 unk73;
    void (*update)(struct Moby_15788 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166F40 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass.collision) */
    s32 collision_disable; /* per-primitive collision disable bits; 1 while a carrier holds the moby (FUN_L00_002c7a58) */
    u8 pad9C[8];
    u8 hit_slot; /* hit-message slot (0xFF: none) */
    u8 padA5;
    s16 oclass;
    u8 padA8[8];
    u8 mission; /* 0xB0: index into the level's D_0014C050 row (0xFF: not spawned) */
    u8 padB1;
    u16 save_id; /* index into the level collected[]/killed[] tables and save bits D_0014C190[level][id >> 5] */
    s16 unkB4;
    u8 padB6[2];
    void *parent; /* 0xB8: spawner (bolt source record); its byte 0xB1 is a per-level id (FUN_L00_002a6b70) */
    u8 command; /* external command byte */
    u8 padBD[3];
    Vec4f_15788 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_15788 rot_row1;
    Vec4f_15788 rot_row2;
    u8 padF0[0x10];
};
typedef struct {
    char _pad00[0x10];
    unsigned char nframes; /* 0x10 */
} AnimSeq_15788;
typedef struct {
    u8 pad00[0x20];
    f32 unk20; /* 0x20 */
    s16 unk24; /* 0x24 */
    u8 pad26[2];
    u8 unk28; /* 0x28 */
    u8 unk29; /* 0x29 */
    u8 pad2A[6];
    f32 unk30; /* 0x30 */
    u8 pad34[0xC];
    f32 vel[4]; /* 0x40: jump velocity; [2] falls by D_L11_001622F0 * frame_time_sq */
    u8 pad50[8];
    u8 unk58; /* 0x58 */
    u8 pad59;
    u8 unk5A; /* 0x5A */
    u8 pad5B[0x6D];
    s16 unkC8; /* 0xC8 */
    u8 padCA[6];
    void *unkD0; /* 0xD0 */
    u8 padD4[0x4C];
    u8 unk120[0x5C]; /* 0x120: FUN_L00_0025c698 / FUN_L00_002dc9f0 state */
    f32 smoke; /* 0x17C: nonzero: dying states give off smoke */
    s32 unk180; /* 0x180: block set up by FUN_L01_0026d930 */
    f32 unk184; /* 0x184 */
    f32 unk188; /* 0x188 */
    f32 unk18C; /* 0x18C */
    u8 pad190[0x28];
    s32 unk1B8; /* 0x1B8 */
    u8 pad1BC[0x14];
    f32 unk1D0[4]; /* 0x1D0 */
    u8 pad1E0[0x30];
    struct Moby_15788 *target; /* 0x210 */
    s32 unk214; /* 0x214: 2 = target in range */
    u8 pad218[8];
    f32 home[4]; /* 0x220 */
    s32 unk230; /* 0x230: path index for FUN_L00_00261968 / the landing check */
    s32 path; /* 0x234: index into D_L11_001B0EB0 */
    s32 mode; /* 0x238: start mode */
    u8 pad23C[8];
    s32 unk244; /* 0x244 */
    u8 pad248[4];
    f32 unk24C; /* 0x24C: heading offset for FUN_L11_00315f58 */
    f32 unk250; /* 0x250: turn speed */
    s32 carrier; /* 0x254: moby index of the carrier, or < 0 */
    f32 unk258; /* 0x258 */
    s32 point; /* 0x25C: current path point */
    f32 unk260[4]; /* 0x260 */
    u8 pad270[0x10];
    u8 unk280[0x10]; /* 0x280 */
} CreatureVars_4318;
extern char * D_L11_001B11B0_15788[] __asm__("D_L11_001B11B0");
extern char * D_L11_00160058 MACRO_ADDR;
extern char D_L11_00167840_15788[] __asm__("D_L11_00167840");
extern char D_0013F4D0[];
extern short D_L11_00162350;
extern short D_L11_0016235C;
extern short D_L11_00162374;
extern short D_L11_00162378;
extern short D_L11_0016237C;
extern short D_L11_00162380;
extern void func_L11_00316DD8_15788(struct Moby_15788 *) __asm__("func_L11_00316DD8");
extern void func_L11_003173C8(char *a0, float *a1);
extern short func_L11_00317500(struct Moby_15788 *self);
extern void func_L00_00264B40(struct Moby_15788 *, int, void *, float);
extern float func_001F9D10(void *, void *);
extern float func_001F9D48_15788(void *, void *) __asm__("func_001F9D48");
extern float func_001F9CB8(void *);
extern void func_L00_0025B178(struct Moby_15788 *);
extern int func_001FA898(float);
extern void func_L01_0026E8E0(void *);
extern int func_001160D8(void);
extern void func_00213DE0_15788(struct Moby_15788 *, int, int, int) __asm__("func_00213DE0");
extern int func_001F9850(int);
extern float func_002140F8(float, float);
extern int func_002140B0(int);
extern float func_00214158(void);
extern float func_00214358(void *, int, float);
extern float func_L00_0025A748(void *);
extern int func_L00_0025A778(void *, void *, int);
extern void func_L00_0025A8E8(struct Moby_15788 *, void *, float, int, float, float, int, int, int);
extern float func_L00_0025BC48(void *, void *, int, float, float);
extern float func_L00_0025CE58(float *, float *, float, float, float, float);
extern int func_L00_0025D6F0(struct Moby_15788 *, void *);
extern void func_L00_00260460(struct Moby_15788 *, void *, int, float, float);
extern void func_L00_00261478(struct Moby_15788 *, void *, void *, void *, void *, void *);
extern int func_L00_002629E0(int, void *, void *);
extern void func_L00_00259868(struct Moby_15788 *, void *, float, float, float, int);
extern float func_L00_00258C80(float, float);
extern void func_L00_0026FF20(void *, void *, float, float);
extern int func_L00_002DDEA0(struct Moby_15788 *, void *);
extern void func_0020D678_15788(struct Moby_15788 *) __asm__("func_0020D678");
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BC0(void *);
extern void func_001FA4A0(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);

/* Update for the class 0x4DE creature: spawns on a path or a carrier moby, wanders, chases and attacks the target, jumps between path points, and smokes while it dies.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l11/gameplay/vendor/0030c788.c, FUN_L11_00314318. */
void func_L11_00315788(struct Moby_15788 *moby) {
    CreatureVars_4318 *d = (CreatureVars_4318 *)moby->pvars;
    struct Moby_15788 *target;
    float pos[4];
    int i;

    if (moby->state != 0x14) {
        d->unkC8 = 0;
        func_L11_00316DD8_15788(moby);
    }
    target = d->target;
    func_L00_00264B40(moby, 0, d->unk280, 2.5f);
    if (moby->visible != 0 && func_001F9D10(&moby->pos, D_L11_00167840_15788) < 28.0f) {
        func_L00_0025B178(moby);
        moby->shadow_range = 0x16;
    }
    switch (moby->state) {
    case 0:
        d->unk20 = 2.0f;
        d->unk24 = 2;
        d->unk28 = 1;
        d->unk30 = 0.25f;
        d->unk58 = func_001FA898(8.0f);
        d->unk5A = func_001FA898(2.0f);
        d->unk29 = 1;
        d->unkD0 = &(*(float *)&D_L11_00162350);
        func_L01_0026E8E0(&d->unk180);
        d->unk18C = 0.5f;
        d->unk188 = 0.5f;
        d->unk180 = func_001FA898(512.0f);
        d->unk184 = 0.5f;
        d->unk1B8 |= 0x10;
        if (func_001160D8() & 1) {
            moby->flags |= 0x8000;
        }
        if (d->mode == 2) {
            moby->state = 0x13;
            if (moby->prev_seq != 4) {
                func_00213DE0_15788(moby, 4, 0, func_001F9850(10));
            }
            qcopy(&moby->pos, D_L11_001B11B0_15788[d->path] + 0x10);
            d->point = 1;
        } else if (d->mode == 1) {
            moby->state = 1;
            qcopy(&moby->pos, D_L11_001B11B0_15788[d->path] + 0x10);
            moby->pos.z -= 1.0f;
            qcopy(d->home, D_L11_001B11B0_15788[d->path] + 0x20);
        } else if (d->carrier >= 0) {
            moby->state = 0xC;
            d->unk258 = func_002140F8(-3.0f, 3.0f);
            moby->visible = 0;
            moby->collision = 0;
            moby->flags |= 1;
        } else {
            moby->pos.z = func_L00_0025A748(&moby->pos);
            moby->state = 2;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, 5);
            }
            qcopy(d->home, &moby->pos);
        }
        break;
    case 1: {
        char *path = D_L11_001B11B0_15788[d->path];
        float ang = func_L00_001FF860(*(float *)(path + 0x20) - moby->pos.x, *(float *)(path + 0x24) - moby->pos.y);
        func_L00_0025CE58(&moby->rot.z, &d->unk250, ang,
                     D_0015EE70 * 8.726646f, D_0015EE70 * 8.726646f, D_0015EE6C * 12.566371f);
        if (d->unk214 != 2 && func_002140B0(9) == 0) {
            moby->state = 7;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, 5);
            }
            func_001F9BF0(d->vel, D_L11_001B11B0_15788[d->path] + 0x20, &moby->pos);
            func_L00_001FF500(d->vel, d->vel, (*(float *)&D_L11_0016237C) * D_0015EE6C);
            d->vel[2] = func_L00_0025BC48(&moby->pos, D_L11_001B11B0_15788[d->path] + 0x20, 0,
                                        (*(float *)&D_L11_0016237C) * D_0015EE6C, -((*(float *)&D_L11_00162380) * D_0015EE70));
        }
        break;
    }
    case 7:
    case 8:
        func_001F9BD8(&moby->pos, &moby->pos, d->vel);
        d->vel[2] -= (*(float *)&D_L11_00162380) * D_0015EE70;
        if (d->vel[2] < 0.0f) {
            float h;
            qcopy(pos, &moby->pos);
            pos[2] += 1.0f;
            h = func_00214358(pos, 0, 0.5f);
            if (moby->pos.z < h) {
                if (moby->state == 7) {
                    moby->pos.z = h;
                    moby->state = 2;
                } else {
                    moby->state = 1;
                    moby->pos.z = h - 1.0f;
                }
            }
        }
        break;
    case 2:
        if (d->unk214 != 2) {
            d->unk24C = func_002140F8(-(*(float *)&D_L11_00162378), (*(float *)&D_L11_00162378)) * 0.017453292f;
            moby->state = 4;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        } else if ((moby->anim_flags & 2) && func_002140B0(3) != 0 && moby->visible != 0 &&
                   func_001F9D10(&moby->pos, D_L11_00167840_15788) < 32.0f) {
            float a = func_00214158();
            float r = func_002140F8(1.0f, 3.0f);
            d->unk260[0] = func_001F9F90(a) * r;
            d->unk260[1] = func_001F9FA8(a) * r;
            d->unk260[2] = 0.0f;
            func_001F9BD8(d->unk260, d->unk260, d->home);
            moby->state = 3;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        }
        break;
    case 3:
        func_L11_003173C8((char *)moby, d->unk260);
        if (d->unk214 != 2) {
            d->unk24C = func_002140F8(-(*(float *)&D_L11_00162378), (*(float *)&D_L11_00162378)) * 0.017453292f;
            moby->state = 4;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        } else if ((moby->anim_flags & 2) && func_002140B0(3) == 0) {
            moby->state = 2;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, func_001F9850(10));
            }
        }
        break;
    case 4:
        if ((moby->anim_flags & 2) && func_001F9D48_15788(&moby->pos, &target->pos) < (*(float *)&D_L11_00162374)) {
            moby->state = 5;
            if (moby->prev_seq != 2) {
                func_00213DE0_15788(moby, 2, 0, func_001F9850(10));
            }
        } else {
            func_L11_003173C8((char *)moby, (float *)&target->pos);
        }
        if (d->unk214 == 2) {
            moby->state = 6;
        }
        break;
    case 10:
        func_001F9BF0(pos, d->unk1D0, &moby->pos);
        pos[2] = 0.0f;
        func_L00_001FF4B0(pos, pos, 2.5f);
        func_001F9BD8(pos, pos, &moby->pos);
        if (func_L00_002629E0(d->unk230, &moby->pos, pos)) {
            float *rz = &moby->rot.z;
            float ang = func_L00_001FF860(d->target->pos.x - moby->pos.x, d->target->pos.y - moby->pos.y);
            func_L00_0025CE58(rz, &d->unk250, ang,
                         D_0015EE70 * 12.566371f, D_0015EE70 * 12.566371f, D_0015EE6C * 12.566371f);
        } else {
            func_L11_003173C8((char *)moby, d->unk1D0);
        }
        if (moby->anim_flags & 2) {
            if (d->unk214 != 2) {
                moby->state = 4;
            } else if (d->unk244 == 0) {
                moby->state = 6;
            }
        }
        break;
    case 5:
        func_L11_003173C8((char *)moby, (float *)&target->pos);
        if ((moby->anim_flags & 2) && (*(float *)&D_L11_00162374) < func_001F9D48_15788(&moby->pos, &target->pos)) {
            moby->state = 4;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        } else if (d->unk214 == 2) {
            moby->state = 6;
        }
        qcopy(pos, &moby->pos);
        pos[2] += 0.333f;
        func_L00_0025A8E8(moby, pos, (*(float *)&D_L11_0016235C) * 0.6f, 1, 1.0f, 1.0f, 0, 1, 0);
        break;
    case 6:
        if (func_001F9D48_15788(&moby->pos, d->home) < 1.0f) {
            moby->state = 2;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, func_001F9850(10));
            }
        } else if (d->unk214 != 2) {
            d->unk24C = func_002140F8(-(*(float *)&D_L11_00162378), (*(float *)&D_L11_00162378)) * 0.017453292f;
            moby->state = 4;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        } else {
            func_L11_003173C8((char *)moby, d->home);
        }
        if ((moby->anim_flags & 2) && moby->prev_seq != 1) {
            func_00213DE0_15788(moby, 1, 0, 5);
        }
        break;
    case 9: {
        int hit = func_L00_0025D6F0(moby, d->unk120);
        if (hit & 0x40) {
            char *path;
            if (moby->pos.z < 222.0f) {
                func_0020D678_15788(moby);
                return;
            }
            path = D_L11_001B11B0_15788[d->unk230];
            if (func_L00_0025A778(&moby->pos, path + 0x10, *(int *)path) == 0) {
                func_0020D678_15788(moby);
                return;
            }
            moby->state = 2;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, func_001F9850(10));
            }
            return;
        }
        if (hit & 0x120) {
            func_0020D678_15788(moby);
            return;
        }
        break;
    }
    case 11:
        if (func_L00_0025D6F0(moby, d->unk120) & 0x160) {
            func_001F9BC0(pos);
            func_L00_00260460(moby, &moby->pos, -1, 0.5f, 10.0f);
            func_0020D678_15788(moby);
            return;
        }
        break;
    case 12: {
        float basis[16];
        float dest[4];
        if (d->unk214 != 2 && func_L11_00317500(moby) < 5) {
            struct Moby_15788 *c = (struct Moby_15788 *)(D_L11_00160058 + (d->carrier << 8));
            func_001FA4A0(basis, &c->rot_row0);
            func_001F9BF0(pos, &moby->pos, &c->pos);
            func_001F9EE8(pos, pos, basis);
            if (pos[0] < d->unk258 && pos[0] > -3.1f) {
                float lo;
                float hi;
                int n;
                dest[0] = pos[0];
                if (pos[1] > 0.0f) {
                    dest[1] = func_002140F8(1.5f, 2.0f);
                } else {
                    dest[1] = func_002140F8(-2.0f, -1.5f);
                }
                dest[2] = 1.0f;
                func_001F9EC0(dest, dest, &c->rot_row0);
                func_001F9BD8(dest, dest, &c->pos);
                moby->state = 0xD;
                if (moby->prev_seq != 1) {
                    func_00213DE0_15788(moby, 1, 0, 5);
                }
                func_001F9BF0(d->vel, dest, &moby->pos);
                n = func_001F9850(0x3C);
                moby->anim_speed = (float)n * ((*(float *)&D_L11_0016237C) * D_0015EE6C) / func_001F9CB8(d->vel);
                func_L00_001FF500(d->vel, d->vel, (*(float *)&D_L11_0016237C) * D_0015EE6C);
                d->vel[2] = func_L00_0025BC48(&moby->pos, dest, 0, (*(float *)&D_L11_0016237C) * D_0015EE6C,
                                            -((*(float *)&D_L11_00162380) * D_0015EE70));
                moby->rot.z = func_L00_001FF860(d->vel[0], d->vel[1]);
            }
        }
        break;
    }
    case 13: {
        char *cv = (char *)((struct Moby_15788 *)(D_L11_00160058 + (d->carrier << 8)))->pvars;
        func_001F9BD8(&moby->pos, &moby->pos, d->vel);
        func_001F9BD8(&moby->pos, &moby->pos, cv + 0x70);
        d->vel[2] -= (*(float *)&D_L11_00162380) * D_0015EE70;
        if (d->vel[2] < 0.0f) {
            qcopy(pos, &moby->pos);
            pos[2] += 1.0f;
            if (moby->pos.z < func_00214358(pos, 0, 0.5f)) {
                moby->anim_speed = 1.0f;
                moby->state = 0xE;
            }
        }
        break;
    }
    case 14:
        func_L00_00261478(moby, D_L11_00160058 + (d->carrier << 8), &moby->pos, &moby->rot,
                            &moby->pos, &moby->rot);
        if (d->unk214 != 2) {
            d->unk24C = func_002140F8(-(*(float *)&D_L11_00162378), (*(float *)&D_L11_00162378)) * 0.017453292f;
            moby->state = 0xF;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        }
        break;
    case 15:
        func_L00_00261478(moby, D_L11_00160058 + (d->carrier << 8), &moby->pos, &moby->rot,
                            &moby->pos, &moby->rot);
        func_L11_003173C8((char *)moby, (float *)&target->pos);
        if ((moby->anim_flags & 2) && func_001F9D48_15788(&moby->pos, &target->pos) < (*(float *)&D_L11_00162374)) {
            moby->state = 0x10;
            if (moby->prev_seq != 2) {
                func_00213DE0_15788(moby, 2, 0, func_001F9850(10));
            }
        }
        if (d->unk214 == 2) {
            moby->state = 0xE;
        }
        break;
    case 16:
        func_L00_00261478(moby, D_L11_00160058 + (d->carrier << 8), &moby->pos, &moby->rot,
                            &moby->pos, &moby->rot);
        func_L11_003173C8((char *)moby, (float *)&target->pos);
        if ((moby->anim_flags & 2) && (*(float *)&D_L11_00162374) < func_001F9D48_15788(&moby->pos, &target->pos)) {
            moby->state = 0xF;
            if (moby->prev_seq != 1) {
                func_00213DE0_15788(moby, 1, 0, func_001F9850(10));
            }
        } else if (d->unk214 == 2) {
            moby->state = 0xE;
        }
        qcopy(pos, &moby->pos);
        pos[2] += 0.333f;
        func_L00_0025A8E8(moby, pos, (*(float *)&D_L11_0016235C) * 0.6f, 1, 1.0f, 0.0f, 0, 1, 0);
        break;
    case 17: {
        int hit;
        func_L00_00261478(moby, D_L11_00160058 + (d->carrier << 8), &moby->pos, &moby->rot,
                            &moby->pos, &moby->rot);
        hit = func_L00_0025D6F0(moby, d->unk120);
        if (hit & 0x40) {
            char *path;
            if (moby->pos.z < 222.0f) {
                func_0020D678_15788(moby);
                return;
            }
            path = D_L11_001B11B0_15788[d->unk230];
            if (func_L00_0025A778(&moby->pos, path + 0x10, *(int *)path) == 0) {
                func_0020D678_15788(moby);
                return;
            }
            moby->state = 0xE;
            if (moby->prev_seq != 0) {
                func_00213DE0_15788(moby, 0, 0, func_001F9850(10));
            }
            return;
        }
        if (hit & 0x120) {
            func_0020D678_15788(moby);
            return;
        }
        break;
    }
    case 18:
        func_L00_00261478(moby, D_L11_00160058 + (d->carrier << 8), &moby->pos, &moby->rot,
                            &moby->pos, &moby->rot);
        if (func_L00_0025D6F0(moby, d->unk120) & 0x160) {
            func_001F9BC0(pos);
            func_L00_00260460(moby, &moby->pos, -1, 0.5f, 10.0f);
            func_0020D678_15788(moby);
            return;
        }
        break;
    case 19: {
        float vel[4];
        float lim;
        float dist;
        qcopy(pos, D_L11_001B11B0_15788[d->path] + d->point * 16 + 0x10);
        if (d->carrier >= 0) {
            float h = ((struct Moby_15788 *)D_L11_00160058)[d->carrier].pos.z - 1.0f;
            if (h < pos[2]) {
                pos[2] = h;
            }
        }
        {
        float *rz = &moby->rot.z;
        float ang = func_L00_001FF860(pos[0] - moby->pos.x, pos[1] - moby->pos.y);
        func_L00_0025CE58(rz, &d->unk250, ang,
                     D_0015EE70 * 12.566371f, D_0015EE70 * 12.566371f, D_0015EE6C * 12.566371f);
        }
        vel[0] = func_001F9F90(moby->rot.z) * (D_0015EE6C + D_0015EE6C);
        vel[1] = func_001F9FA8(moby->rot.z) * (D_0015EE6C + D_0015EE6C);
        vel[2] = pos[2] - moby->pos.z;
        lim = D_0015EE6C * 4.0f;
        if (vel[2] > lim) {
            vel[2] = lim;
        } else if (vel[2] < -lim) {
            vel[2] = -lim;
        }
        func_L00_00259868(moby, vel, 0.5f, 0.5f, 0.0f, 0x10);
        if (func_001F9D10(pos, &moby->pos) < 1.0f) {
            d->point = (d->point + 1) % *(int *)D_L11_001B11B0_15788[d->path];
        }
        if (moby->anim_time == 0.0f) {
            dist = func_001F9D48_15788(&moby->pos, D_0013F4D0);
            switch (moby->prev_seq) {
            case 4:
                if (dist < 6.0f) {
                    func_00213DE0_15788(moby, 5, 0, func_001F9850(10));
                }
                break;
            case 6:
                if (dist > 8.0f) {
                    func_00213DE0_15788(moby, 7, 0, func_001F9850(10));
                } else if (dist < 4.0f) {
                    float p[4];
                    qcopy(p, &moby->pos);
                    p[2] += 0.5f;
                    func_L00_0025A8E8(moby, p, (*(float *)&D_L11_0016235C) * 0.55f, 1, 1.0f, 1.0f, 0, 1, 0);
                }
                break;
            case 5:
                func_00213DE0_15788(moby, 6, 0, func_001F9850(10));
                break;
            case 7:
                func_00213DE0_15788(moby, 4, 0, func_001F9850(10));
                break;
            }
        }
        break;
    }
    case 20:
        if (d->unkC8 != 0 && func_L00_002DDEA0(moby, d->unk120)) {
            moby->state = moby->command;
            d->unkC8 = 0;
        }
        break;
    }
    if (d->smoke != 0.0f) {
        if (moby->state == 9 || moby->state == 0xB || moby->state == 0x11 || moby->state == 0x12) {
            for (i = 0; i < 3; i++) {
                float vel[4];
                float a;
                float s;
                pos[0] = moby->pos.x + func_L00_00258C80(0.0f, 0.2f);
                pos[1] = moby->pos.y + func_L00_00258C80(0.0f, 0.2f);
                pos[2] = moby->pos.z + func_L00_00258C80(0.0f, 0.2f) + 0.8f;
                a = func_00214158();
                s = func_002140F8(0.2f, 1.3f) * D_0015EE6C;
                vel[0] = func_001F9F90(a) * s;
                vel[1] = func_001F9FA8(a) * s;
                vel[2] = 0.0f;
                vel[2] = func_002140F8(-0.2f, 0.4f) * D_0015EE6C;
                func_L00_0026FF20(pos, vel, func_002140F8(2100.0f, 14700.0f), 222.5f);
            }
        }
    }
}
