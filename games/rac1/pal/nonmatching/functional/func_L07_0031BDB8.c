/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L07_0031a9e8), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: exact alone, but its file's other functions stop matching with it in place. */
extern s32 D_0015EE84 MACRO_ADDR;
extern f32 D_0015EE6C MACRO_ADDR;
typedef u32 u128 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f;
typedef union {
    u128 q;
    f32 f[4];
    s32 i[4];
} Vec4;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass {
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
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby.seq */
};
struct Moby {
    Vec4f bsphere;
    Vec4f pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass *pclass;
    struct Moby *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby *moby);
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
    Vec4f rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f rot_row1;
    Vec4f rot_row2;
    u8 padF0[0x10];
};
struct L07MovingVars {
    Vec4f home;
    int target;
    int openState;
    int shutState;
    Vec4f destination;
    float speed;
    int startSound;
    int endSound;
    int indicator;
    int forceReturn;
};
extern u8 D_0014C150[];
extern u8 D_L07_0015FD08[];
extern s32 D_L07_001845A8[];
extern short D_L07_00160058;
extern float func_001F9CB8(void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9EC0(void *, void *, void *);
extern int func_0022ED80_1BDB8(int, int, struct Moby *) __asm__("func_0022ED80");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l07/gameplay/entities/0030cd20.c, FUN_L07_0031a9e8. */
void func_L07_0031BDB8(struct Moby *moby) {
    struct L07MovingVars *vars = (struct L07MovingVars *)moby->pvars;
    struct Moby *target;
    Vec4 next;
    Vec4 start;
    float distance;
    float time;
    int force_end;
    int force_start;
    u8 spawn_id;
    int no_spawn;

    if (moby->mission != 0xff && D_0014C150[moby->mission + D_0015EE84 * 16] == 0xff)
        vars->target = -1;
    spawn_id = moby->mission;
    force_end = 0;
    /* Fakematch: the 0xFF compare needs its own variable for retail register order. */
    no_spawn = 0xff;
    force_start = 0;
    if (D_L07_0015FD08[spawn_id] == no_spawn) {
        if (vars->forceReturn != 0)
            force_end = 1;
        else
            force_start = 1;
    }

    switch (moby->state) {
    case 0:
        qcopy(&vars->home, &moby->pos);
        if (vars->indicator != -1)
            D_L07_001845A8[vars->indicator] = 0;
        moby->state = 1;
        vars->home.w = 0;
        return;
    case 1:
        if (force_start)
            return;
        if (!force_end && vars->target != -1 &&
            (target = &(*(struct Moby * *)&D_L07_00160058)[vars->target]) != 0 &&
            target->state != 0xfe && target->state != 0xfd &&
            target->state != vars->openState)
            return;
        start.q = 0;
        start.f[0] = vars->destination.x;
        start.f[1] = vars->destination.y;
        start.f[2] = vars->destination.z;
        next.q = start.q;
        vars->home.w = func_001F9CB8(&next);
        if (vars->startSound != -1)
            func_0022ED80_1BDB8(vars->startSound, 0, moby);
        moby->state = 2;
        break;
    case 2:
        if (force_start || (!force_end && vars->target != -1 &&
            (target = &(*(struct Moby * *)&D_L07_00160058)[vars->target]) != 0 &&
            target->state != 0xfe && target->state != 0xfd &&
            target->state == vars->shutState)) {
            if (vars->startSound != -1)
                func_0022ED80_1BDB8(vars->startSound, 0, moby);
            moby->state = 4;
            break;
        }
        start.q = 0;
        start.f[0] = vars->destination.x;
        start.f[1] = vars->destination.y;
        start.f[2] = vars->destination.z;
        next.q = start.q;
        func_001F9EC0(&next, &next, &moby->rot_row0);
        func_L00_001FF4B0(&start, &next, vars->destination.w * D_0015EE6C);
        func_001F9BD8(&moby->pos, &moby->pos, &start);
        time = vars->home.w - vars->destination.w * D_0015EE6C;
        vars->home.w = time;
        if (!(time < 0.0f))
            return;
        func_001F9BD8(&moby->pos, &vars->home, &next);
        vars->home.w = 0;
        if (vars->indicator != -1)
            D_L07_001845A8[vars->indicator] = 1;
        moby->state = 3;
        if (vars->endSound != -1)
            func_0022ED80_1BDB8(vars->startSound, 0, moby);
        return;
    case 3:
        if (!force_start && (force_end || vars->target == -1 ||
            (target = &(*(struct Moby * *)&D_L07_00160058)[vars->target]) == 0 ||
            target->state == 0xfe || target->state == 0xfd ||
            target->state != vars->shutState))
            return;
        vars->home.w = 0;
        if (vars->startSound != -1)
            func_0022ED80_1BDB8(vars->startSound, 0, moby);
        moby->state = 4;
        break;
    case 4:
        if (!force_start && (force_end || vars->target == -1 ||
            (target = &(*(struct Moby * *)&D_L07_00160058)[vars->target]) == 0 ||
            target->state == 0xfe || target->state == 0xfd ||
            target->state == vars->openState)) {
            if (vars->startSound != -1)
                func_0022ED80_1BDB8(vars->startSound, 0, moby);
            moby->state = 2;
            break;
        }
        start.q = 0;
        start.f[0] = vars->destination.x;
        start.f[1] = vars->destination.y;
        start.f[2] = vars->destination.z;
        next.q = start.q;
        distance = func_001F9CB8(&next);
        func_L00_001FF4B0(&next, &next, -(vars->speed * D_0015EE6C));
        func_001F9EC0(&start, &next, &moby->rot_row0);
        func_001F9BD8(&moby->pos, &moby->pos, &start);
        time = vars->home.w + vars->speed * D_0015EE6C;
        vars->home.w = time;
        if (!(distance < time))
            return;
        qcopy(&moby->pos, &vars->home);
        if (vars->startSound != -1)
            func_0022ED80_1BDB8(vars->endSound, 0, moby);
        if (vars->indicator != -1)
            D_L07_001845A8[vars->indicator] = 0;
        moby->state = 1;
        break;
    case 5:
        break;
    }
}
