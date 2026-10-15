/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_E35C8;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_E35C8 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby_E35C8.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby_E35C8.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby_E35C8.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby_E35C8.seq */
};
struct Moby_E35C8 {
    Vec4f_E35C8 bsphere;
    Vec4f_E35C8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_E35C8 *pclass;
    struct Moby_E35C8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_E35C8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_E35C8 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166F40 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass_E35C8.collision) */
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
    Vec4f_E35C8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_E35C8 rot_row1;
    Vec4f_E35C8 rot_row2;
    u8 padF0[0x10];
};
extern void func_001F9BF0(void *, void *, void *);
struct Moby_E35C8;
typedef struct Moby_E35C8 Moby_E35C8;
extern float D_0015EE6C MACRO_ADDR;
extern void func_L00_001FF4B0(void *, void *, f32);
typedef struct {
    struct Moby_E35C8 *moby;
    struct Moby_E35C8 *anchor;
    s32 joint; /* joint on this piece */
    s32 pjoint; /* joint on the anchor it hangs from */
} TargetSlot_E35C8;
typedef struct {
    u8 pad0[0x60];
    TargetSlot_E35C8 slots[34];
    u8 pad280[0xC];
    s16 points;
} TargetBoardVars_E35C8;
extern int func_L00_0025B478(struct Moby_E35C8 *, int, int);
extern void func_L00_00260108(void *, void *, s32, f32, f32);
extern f32 func_002140F8(f32, f32);
extern s32 func_001FA898(f32);
extern s32 func_0022ED80(s32, s32, struct Moby_E35C8 *);
struct Moby_E35C8;

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l08/gameplay/entities/002deee0.c, FUN_L08_002e2250. */
void func_L08_002E35C8(struct Moby_E35C8 *moby) {
    u8 mark = 0xff;
    int selected = 0;
    TargetBoardVars_E35C8 *vars = (TargetBoardVars_E35C8 *)moby->pvars;
    TargetSlot_E35C8 *slots = vars->slots;
    TargetSlot_E35C8 *slot = slots;
    int i = 0;
    int hits = 0;
    int sound = -1;
    int total;

    do {
        if (slot->moby != 0) {
            if (func_L00_0025B478(slot->moby, 0x10000, 0) != 0) {
                if (i < 7) {
                    if (hits < 1) {
                        hits = 1;
                        sound = 1;
                        goto mark_hit;
                    }
                } else if (i < 10) {
                    if (hits < 2) {
                        hits = 2;
                        sound = 1;
                        goto mark_hit;
                    }
                } else if (hits < 4) {
                    hits += 4;
                    selected = i;
                    sound = 2;
                    goto mark_hit;
                }
mark_hit:
                slot->moby->hit_slot = mark;
            }
        }
        i++;
        slot++;
    } while (i < 34);
    vars->points = (u16)vars->points + hits;
    total = vars->points;
    if (total >= 8) {
        vars->points = total - (total / 8) * 8;
        sound = 0;
        if (selected != 0) {
            u8 *vector;
            slot = &slots[selected];
            vector = slot->moby->pvars;
            func_L00_00260108(moby, &slot->moby->pos, -1, 2.0f, 13.0f);
            func_001F9BF0(vector, &slot->moby->pos, &(*(struct Moby_E35C8 **)((char *)vars + 0x64 + selected * 16))->pos);
            func_L00_001FF4B0(vector, vector, D_0015EE6C * 10.0f);
            slot->moby->state = 1;
            slot->moby = 0;
            return;
        }
        {
            int start = func_001FA898(func_002140F8(0.0f, 23.0f));
            i = 0;
            do {
                int index = (i + start) % 24 + 10;
                slot = &slots[index];
                i++;
                if (slot->moby != 0) {
                    u8 *vector = slot->moby->pvars;
                    func_L00_00260108(moby, &slot->moby->pos, -1, 2.0f, 13.0f);
                    func_001F9BF0(vector, &slot->moby->pos, &(*(struct Moby_E35C8 **)((char *)vars + 0x64 + index * 16))->pos);
                    func_L00_001FF4B0(vector, vector, D_0015EE6C * 10.0f);
                    slot->moby->state = 1;
                    slot->moby = 0;
                    return;
                }
            } while (i < 24);
            moby->state = 99;
        }
    }
play_sound:
    if (sound >= 0)
        func_0022ED80(sound, 0, moby);
}
