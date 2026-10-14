/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L12_002ece00), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: 32 of 1152 bytes differ under PAL's compiler. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_EE180;
struct Moby_EE180;
extern s32 D_0015EE84_EE180 __asm__("D_0015EE84") MACRO_ADDR;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_EE180 {
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
struct Moby_EE180 {
    Vec4f_EE180 bsphere;
    Vec4f_EE180 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_EE180 *pclass;
    struct Moby_EE180 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_EE180 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_EE180 *moby);
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
    Vec4f_EE180 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_EE180 rot_row1;
    Vec4f_EE180 rot_row2;
    u8 padF0[0x10];
};
extern int func_001F9908(int *);
extern float func_001FA748(float, float);
extern int func_001F9850(int);
typedef struct {
    float vel[4];
    struct Moby_EE180 *owner;
    int timer;
    float unk18;
    float yaw;
} ShotVars_ece00;
typedef struct {
    float dir[4];
    float unk10[2];
    char unk18;
    char unk19;
    short oclass;
    int pad1C;
    int pad20[4];
} ShotHit_ece00;
typedef struct { char pad0[0x18]; int moby; } ShotColl_ece00;
extern ShotColl_ece00 D_L12_00174340;
extern int D_001414D4;
extern short D_L12_00161FA0_EE180 __asm__("D_L12_00161FA0");
extern char D_L12_001672C0[];
extern int D_L12_0015F660_EE180 __asm__("D_L12_0015F660");
extern float D_0015EE6C MACRO_ADDR;
extern char *func_L00_0025B478_EE180(struct Moby_EE180 *, int, int) __asm__("func_L00_0025B478");
extern void func_0020D678_EE180(struct Moby_EE180 *) __asm__("func_0020D678");
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_L00_001FF240(void *, void *, void *);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026A7F8(void *, void *, int, int, int, int, int, int);
extern int func_L00_001EFFF0(void *, void *, int, struct Moby_EE180 *, int);
extern int func_L00_001F10E0(void *, float, int, struct Moby_EE180 *);
extern void func_L00_0025A8C0(void *, struct Moby_EE180 *, int, float, void *);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_L00_0025AAC0(int, void *);
extern void func_L00_0025F4A8_EE180(struct Moby_EE180 *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L00_00260108(void *, void *, int, float, float);

/* Level 12 shot: flies with a smoke trail, damages the first moby it hits other than its owner, then pops.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l12/gameplay/entities/002ec720.c, FUN_L12_002ece00. */
void func_L12_002EE180(struct Moby_EE180 *moby) {
    ShotVars_ece00 *vars = (ShotVars_ece00 *)moby->pvars;
    float old_pos[4];
    float side[4];
    float back[4];
    float trail[4];
    ShotHit_ece00 hit;
    switch (moby->state) {
    case 1: {
        char *touch = func_L00_0025B478_EE180(moby, 0x230000, 0);
        int damage;
        if (touch != 0 && *(struct Moby_EE180 **)(touch + 0x20) != vars->owner) {
            moby->state = 2;
            break;
        }
        if (D_0015EE84_EE180 == 12 && D_001414D4 == 50 && vars->owner != 0) {
            if ((signed char)vars->owner->state < 0) {
                func_0020D678_EE180(moby);
                return;
            }
            if (vars->owner->state < 2) {
                moby->state = 2;
                break;
            }
        }
        qcopy(old_pos, &moby->pos);
        func_001F9BD8(&moby->pos, &moby->pos, vars);
        moby->rot.z = vars->yaw;
        moby->rot.x = func_001FA748(moby->rot.x, D_0015EE6C * 6.2831855f);
        func_L00_001FF4B0(side, vars, 0.2f);
        func_L00_001FF4B0(back, vars, -0.2f);
        func_L00_001FF240(trail, back, &moby->pos);
        func_L00_0026A7F8(back, side, 0x6F00AFFF, 0xFF, func_001F9850(func_L00_00258BC8(15, 22)), 40,
                         func_L00_00258BC8(20, 35), 1);
        func_L00_0026A7F8(back, side, 0x1FFFFFFF, 0x4F4F4F, func_001F9850(func_L00_00258BC8(30, 60)), 40,
                         func_L00_00258BC8(50, 75), 0);
        if (func_L00_001EFFF0(&moby->pos, old_pos, 0, moby, 0) || func_L00_001F10E0(&moby->pos, 0.2f, 0, moby)) {
            if (D_L12_00174340.moby != 0 && D_L12_00174340.moby != (int)vars->owner) {
                damage = 1;
                if (D_0015EE84_EE180 == 12 && D_001414D4 == 50) {
                    damage = (*(int *)&D_L12_00161FA0_EE180) >= 16 ? 2 : 3;
                }
                func_L00_0025A8C0(&hit, moby, 0x10003, damage, vars);
                hit.dir[0] = func_001F9F90(vars->yaw);
                hit.dir[1] = func_001F9FA8(vars->yaw);
                hit.dir[2] = 1.0f;
                hit.dir[3] = 5627.9248f;
                hit.unk18 = 1;
                hit.unk19 = 1;
                hit.oclass = moby->oclass;
                func_L00_0025AAC0(D_L12_00174340.moby, &hit);
            }
            moby->state = 2;
        }
        if (func_001F9908(&vars->timer)) {
            moby->state = 2;
        }
        break;
    }
    case 2:
        func_001F9BF0(old_pos, &moby->pos, D_L12_001672C0);
        func_L00_001FF4B0(old_pos, old_pos, 0.4f);
        func_001F9BD8(&moby->pos, &moby->pos, old_pos);
        if (D_0015EE84_EE180 == 12 && D_001414D4 == 50) {
            char *camera = D_L12_001672C0 - 0x140;
            func_L00_0025F4A8_EE180(moby, &D_L12_0015F660_EE180, 0, 0.0f, 0.0f, 3, 3, 5, 0.3f, 0.2f, 0.0f, 0.15f, -1, 0.0f, 0, 0,
                             -1, 0);
            *(float *)(camera + 0x160) = 0.025f;
            *(int *)(camera + 0x168) = func_001F9850(20);
        } else {
            func_L00_00260108(moby, &moby->pos, 0, 0.25f, 13.0f);
        }
        func_0020D678_EE180(moby);
        break;
    }
}
