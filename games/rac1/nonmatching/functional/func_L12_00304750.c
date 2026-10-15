/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef u32 u128_04750 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_04750;
struct Moby_04750;
extern short D_L12_00160058;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_04750 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby_04750.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby_04750.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby_04750.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby_04750.seq */
};
struct Moby_04750 {
    Vec4f_04750 bsphere;
    Vec4f_04750 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_04750 *pclass;
    struct Moby_04750 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_04750 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_04750 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166F40 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass_04750.collision) */
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
    Vec4f_04750 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_04750 rot_row1;
    Vec4f_04750 rot_row2;
    u8 padF0[0x10];
};
typedef struct Moby_04750 Moby_04750;
extern int func_001F9908(int *);
extern float func_001F9D48(void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_001FA850(float, float);
extern int func_001F9850(int);
extern int D_L12_00167180;
typedef struct { char pad[0x140]; float position[4]; float pad150; float pitch; float yaw; } Reference_303370_04750;

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l12/gameplay/vendor/003028c8.c, FUN_L12_00303370. */
void func_L12_00304750(void *moby) {
    char *data = (char *)((struct Moby_04750 *)moby)->pvars;
    int found = 0;
    int i;
    int offset;
    float position[4] __attribute__((aligned(16)));

    i = 0;
    do {
        offset = i * 4;
        offset = *(int *)(data + offset + 0x40);
        if (offset != -1) {
            struct Moby_04750 *other = (struct Moby_04750 *)((*(char * *)&D_L12_00160058) + offset * 0x100);
            float yaw_limit;
            float pitch_limit;
            float yaw;
            float distance;
            float pitch;

            *(volatile u128_04750 *)position = *(u128_04750 *)&other->pos;
            yaw_limit = 0.12217305f;
            pitch_limit = yaw_limit;
            if (other->oclass == 0x146) {
                position[2] += 1.5f;
            }
            if (other->oclass == 0x4fe) {
                yaw_limit = 0.034906585f;
                pitch_limit = 0.05235988f;
                position[2] += 1.5f;
            }
            yaw = func_L00_001FF860(position[0] - ((Reference_303370_04750 *)&D_L12_00167180)->position[0],
                                   position[1] - ((Reference_303370_04750 *)&D_L12_00167180)->position[1]);
            distance = func_001F9D48(((Reference_303370_04750 *)&D_L12_00167180)->position, position);
            pitch = func_L00_001FF860(distance, position[2] - ((Reference_303370_04750 *)&D_L12_00167180)->position[2]);
            if (func_001FA850(yaw, ((Reference_303370_04750 *)&D_L12_00167180)->yaw) < yaw_limit &&
                func_001FA850(pitch, -((Reference_303370_04750 *)&D_L12_00167180)->pitch) < pitch_limit) {
                found = 1;
            }
        }
        i++;
        offset = i * 4;
    } while (i < 10);
    if (!found) {
        func_001F9908((int *)(data + 0x88));
    } else {
        *(int *)(data + 0x88) += 2;
        if (func_001F9850(0x1e) < *(int *)(data + 0x88)) {
            *(int *)(data + 0x88) = func_001F9850(0x1e);
        }
    }
}
