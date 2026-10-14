/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern f32 D_0015EE6C MACRO_ADDR;
typedef u32 u128_F5980 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_F5980;
typedef union {
    u128_F5980 q;
    f32 f[4];
    s32 i[4];
} Vec4_F5980;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_F5980 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby_F5980.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby_F5980.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby_F5980.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby_F5980.seq */
};
struct Moby_F5980 {
    Vec4f_F5980 bsphere;
    Vec4f_F5980 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_F5980 *pclass;
    struct Moby_F5980 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_F5980 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_F5980 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166F40 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass_F5980.collision) */
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
    Vec4f_F5980 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_F5980 rot_row1;
    Vec4f_F5980 rot_row2;
    u8 padF0[0x10];
};
struct Moby_F5980;
typedef struct {
    s32 count;
    u8 pad4[0xC];
    Vec4_F5980 points[1];
} SwarmPath_F5980;
typedef struct {
    s32 point;
    s8 direction;
    u8 pad5[0xB];
    SwarmPath_F5980 *path;
    s32 path_id; /* index into D_L08_001B0CB0 */
    u8 pad18[0x18];
} SwarmPathFollower_F5980;
extern char D_0013F4D0[];
extern char D_L08_0015F660[];
extern short D_L08_00162000;
extern s32 func_L00_0025B478(struct Moby_F5980 *, s32, s32);
extern f32 func_001F9D48(void *, void *);
extern f32 func_001FA748(f32, f32);
extern f32 func_L00_001FF860(f32, f32);
extern f32 func_001F9D10(void *, void *);
extern void func_001F9BC0(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, f32);
extern void func_001F9BD8(void *, void *, void *);
extern s32 func_001F9850(s32);
extern f32 func_00214D28(void *, f32, f32);
extern void func_0020D678(struct Moby_F5980 *);
extern void func_0022ED80(s32, s32, struct Moby_F5980 *);
extern void func_L00_002592B0(struct Moby_F5980 *, f32, f32 *, f32, f32, f32);
extern void func_L00_0025F4A8(void *, void *, void *, f32, f32, s32, s32, s32, f32, f32, f32, f32, s32, f32, s32, s32, s32, s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l08/gameplay/entities/002e9b70.c, FUN_L08_002f44c0. */
void func_L08_002F5980(struct Moby_F5980 *moby)
{
    SwarmPathFollower_F5980 *follower = (SwarmPathFollower_F5980 *)moby->pvars;
    Vec4_F5980 point;
    Vec4_F5980 step;
    Vec4_F5980 velocity; /* Effect scratch is separate from the waypoint movement step. */
    f32 fade;
    s32 waypoint;
    f32 distance;

    switch (moby->state) {
    case 0: goto state_zero;
    case 1: goto state_one;
    case 2: goto state_two;
    case 3: goto state_three;
    default: return;
    }
state_zero: {
        fade = moby->pos.z;
        moby->state = 1;
        moby->pos.z = fade + 0.4f;
        return;
    }
state_one: {
        s32 active = func_L00_0025B478(moby, 0x330000, 0);
        distance = func_001F9D48(&moby->pos, D_0013F4D0);
        if ((distance < 2.0f && *(s32 *)(D_0013F4D0 + 0x2004) == 0x42) || *(struct Moby_F5980 **)(D_0013F4D0 + 0x1bc) == moby) {
            func_0022ED80(0, 0, moby);
            func_L00_0025F4A8(moby, D_L08_0015F660, &moby->pos, 3.0f, 1.0f, 10, 20, 60,
                          3.0f, 5.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
        } else {
            if (active == 0) return;
            func_0022ED80(0, 0, moby);
            func_L00_0025F4A8(moby, D_L08_0015F660, &moby->pos, 0.0f, 0.0f, 10, 20, 60,
                          1.0f, 3.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
        }
        func_0020D678(moby);
        return;
    }
state_two: {
        moby->pos.z -= 0.4f;
        if (follower->path_id == -1 || follower->path->count == 0) {
            func_0020D678(moby);
            return;
        }
        moby->rot.y = func_001FA748(moby->rot.y, (*(f32 *)&D_L08_00162000) * 0.017453292f * D_0015EE6C);
        distance = func_001F9D48(&moby->pos, D_0013F4D0);
        if (distance < 2.0f && *(s32 *)(D_0013F4D0 + 0x2004) == 0x42) {
            func_001F9BC0(&velocity);
            func_L00_0025F4A8(moby, &velocity, &moby->pos, 0.0f, 0.0f, 10, 20, 60,
                          3.0f, 5.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
            func_0020D678(moby);
            return;
        }
        waypoint = (follower->point + follower->path->count + follower->direction) % follower->path->count;
        func_L00_002592B0(moby, func_L00_001FF860(follower->path->points[waypoint].f[0] - moby->pos.x,
                                   follower->path->points[waypoint].f[1] - moby->pos.y),
                     (f32 *)((char *)follower + 0x34), 0.01f, 0.3f, 0.1f);
        qcopy(&point, &follower->path->points[waypoint]);
        if (waypoint != 0) {
            func_001F9BF0(&step, &point, &moby->pos);
            func_L00_001FF4B0(&step, &step, D_0015EE6C * 3.0f);
            func_001F9BD8(&moby->pos, &moby->pos, &step);
        } else {
            distance = func_001F9D48(&moby->pos, D_0013F4D0);
            if (distance < 24.0f) {
                func_001F9BC0(&velocity);
                func_L00_0025F4A8(moby, &velocity, &moby->pos, 0.0f, 0.0f, 10, 20, 60,
                              3.0f, 5.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
                func_0022ED80(0, 0, moby);
                func_0020D678(moby);
                return;
            }
            moby->state = 3;
        }
        if (func_001F9D10(&moby->pos, &point) < 0.1f) {
            follower->point = waypoint;
        }
        moby->pos.z += 0.4f;
        return;
    }
state_three: {
        Vec4_F5980 timer;
        timer.f[0] = (f32)moby->alpha;
        func_00214D28(&timer, 0.0f, 128.0f / (f32)func_001F9850(30));
        moby->alpha = (u8)(s32)timer.f[0];
        if (moby->alpha == 0) func_0020D678(moby);
    }
}
