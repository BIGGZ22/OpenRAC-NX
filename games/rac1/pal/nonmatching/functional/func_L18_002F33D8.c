/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L18_002f1fd8), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: its PAL file declares a symbol it uses with another type, so it was not built there. */
typedef u32 u128 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_F33D8;
typedef union {
    u128 q;
    f32 f[4];
    s32 i[4];
} Vec4_F33D8;
struct Moby_F33D8;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_F33D8 {
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
struct Moby_F33D8 {
    Vec4f_F33D8 bsphere;
    Vec4f_F33D8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_F33D8 *pclass;
    struct Moby_F33D8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_F33D8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_F33D8 *moby);
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
    Vec4f_F33D8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_F33D8 rot_row1;
    Vec4f_F33D8 rot_row2;
    u8 padF0[0x10];
};
extern float D_L18_00167840[];
struct GeometryQuad {
    Vec4_F33D8 positions[4]; /* 0x00 */
    u32 colors[4]; /* 0x40 */
    f32 texture_coordinates[4][2]; /* 0x50 */
    union {
        u64 value;
        volatile u64 ordered;
    } reserved; /* 0x70 */
    u64 texture; /* 0x78 */
    u64 texture_state; /* 0x80 */
    u64 primitive; /* 0x88 */
};
struct GlowVars {
    u8 pad0[8];
    f32 scroll;
    f32 max_alpha;
};
extern void func_001FA460(void *, void *);
extern float func_001F9D10(void *, void *);
extern u64 func_001F4868_F33D8(int) __asm__("func_001F4868");
extern int func_001FA898(float);
extern int func_001FA8A8(int, int, float);
extern void func_L00_001FD1D8(void *, void *, int);
extern float D_0015EE6C MACRO_ADDR;
extern short D_L18_00162370;
extern short D_L18_00162374;
extern short D_L18_00162378;
extern short D_L18_0016237C;
extern short D_L18_00162380;
extern short D_L18_00162358;
extern short D_L18_0016235C;
extern short D_L18_00162360;
extern short D_L18_00162364;
extern f32 D_L18_001DA620[4][2];
extern u128 D_L18_001DA640[4];

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l18/gameplay/vendor/002efb88.c, FUN_L18_002f1fd8. */
void func_L18_002F33D8(struct Moby_F33D8 *moby) {
    struct GeometryQuad quads[4];
    float matrix[4][4] __attribute__((aligned(16)));
    struct GlowVars *vars;
    float alpha;
    float step;
    float phase;
    int color;
    int i;
    int j;
    int pass;

    vars = (struct GlowVars *)moby->pvars;
    func_001FA460(matrix, &moby->rot_row0);
    qcopy(matrix[3], &moby->pos);
    matrix[3][3] = 1.0f;
    vars->scroll += (*(f32 *)&D_L18_00162370) * D_0015EE6C;
    if (1.0f < vars->scroll) {
        vars->scroll -= 1.0f;
    }
    alpha = (48.0f - func_001F9D10(&moby->pos, D_L18_00167840)) * 0.25f;
    if (vars->max_alpha < alpha) {
        alpha = vars->max_alpha;
    } else if (alpha < 0.0f) {
        alpha = 0.0f;
    }
    step = 0.25f;
    for (i = 0; i < 4; i++) {
        quads[i].texture = func_001F4868_F33D8(0xE);
        quads[i].reserved.value = 0;
        quads[i].texture_state = 0xFF9000000260ULL;
        quads[i].primitive = (u64)(*(int *)&D_L18_00162374) | ((u64)(*(int *)&D_L18_00162378) << 2) | ((u64)(*(int *)&D_L18_0016237C) << 4) |
                             ((u64)(*(int *)&D_L18_00162380) << 6) | 0x8000000000ULL;
        phase = vars->scroll + i * step;
        phase = phase - (float)func_001FA898(phase);
        color = func_001FA8A8((*(int *)&D_L18_00162358), (*(int *)&D_L18_0016235C), phase);
        color = func_001FA8A8(color & 0xFFFFFF, color, alpha);
        for (j = 0; j < 4; j++) {
            quads[i].texture_coordinates[j][0] = D_L18_001DA620[j][0] + i * step;
            quads[i].texture_coordinates[j][1] = D_L18_001DA620[j][1];
            quads[i].colors[j] = color;
            qcopy_nc(&quads[i].positions[j], &D_L18_001DA640[j]);
            if (j & 1) {
                quads[i].positions[j].f[2] += phase * (*(f32 *)&D_L18_00162364);
            } else {
                quads[i].positions[j].f[2] -= phase * (*(f32 *)&D_L18_00162364);
            }
        }
    }
    for (pass = 2; pass >= 0; pass--) {
        matrix[3][2] += (*(f32 *)&D_L18_00162360);
        func_L00_001FD1D8(&quads[0], matrix, 0);
        func_L00_001FD1D8(&quads[1], matrix, 0);
        func_L00_001FD1D8(&quads[2], matrix, 0);
        func_L00_001FD1D8(&quads[3], matrix, 0);
    }
}
