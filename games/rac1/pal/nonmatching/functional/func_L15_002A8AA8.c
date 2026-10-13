/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef u32 u128_A8AA8 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_A8AA8;
struct Moby_A8AA8;
struct Manip;
struct GifEntry;
struct AnimSeq;
struct MobyClass_A8AA8 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby_A8AA8.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby_A8AA8.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby_A8AA8.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby_A8AA8.seq */
};
struct Moby_A8AA8 {
    Vec4f_A8AA8 bsphere;
    Vec4f_A8AA8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_A8AA8 *pclass;
    struct Moby_A8AA8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_A8AA8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_A8AA8 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166F40 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass_A8AA8.collision) */
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
    Vec4f_A8AA8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_A8AA8 rot_row1;
    Vec4f_A8AA8 rot_row2;
    u8 padF0[0x10];
};
extern s32 D_0015EE84 MACRO_ADDR;
extern f32 D_0015EE6C MACRO_ADDR;
extern char D_0014C290[];
extern unsigned char D_001414F4[];
extern unsigned char D_0013DE58 NOT_SDA;
typedef union { u128_A8AA8 q; float f[4]; } Vec_2a7880_A8AA8;
extern char D_L15_00178780[];
extern char D_L15_001615E8[];
typedef struct { u32 words[3]; } Colors_2a7880_A8AA8;
extern Colors_2a7880_A8AA8 D_L15_001615F8;
extern void func_L00_001FF4B0(float, void *, void *);
extern void func_001F9C30(float, void *, void *);
extern float func_001F9CB8(void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF240(void *, void *, void *);
extern void func_001F9BC0(void *);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_002140F8(float, float);
extern float func_00214158(void);
extern int func_002140B0(int);
extern int func_L00_00258BC8(int, int);
extern int func_L00_00237B70(float, int, int);
extern int func_001F9850(int);
extern void func_0020D678(struct Moby_A8AA8 *);
extern void func_L00_002584A8(void *, int, long);
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int);
extern void *func_L01_002F9908(void *, void *, int, float, int, float, float, float, int);
extern void *func_L00_0026CD70(void *, void *, int, int, float, int, int);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l15/gameplay/entities/0029aff0.c, FUN_L15_002a7880. */
void func_L15_002A8AA8(struct Moby_A8AA8 *moby)
{
    char *slot = 0;
    Vec_2a7880_A8AA8 velocity, first, second, offset, base;
    u32 palette[4];
    u32 colors[4];
    Vec_2a7880_A8AA8 position;
    int i, index, slot_index;
    u32 *palette_entry;
    float radius, angle, size;
    int color, color2, duration;
    void *effect;

    if (D_001414F4[0] != 2 && moby->state == 1)
        return;

    for (slot_index = 0; slot_index < 64; slot_index++) {
        char *entry = D_L15_00178780 + slot_index * 0x40;
        if (*(struct Moby_A8AA8 **)(entry + 0x34) != moby)
            continue;
        if ((*(u32 *)(entry + 0x24) & 0x80000) == 0)
            continue;
        slot = entry;
        break;
    }
    moby->hit_slot = 0xff;
    if (slot == 0)
        goto no_hit;

    __builtin_memcpy(palette, &D_L15_001615E8, 16);
    func_L00_001FF4B0(D_0015EE6C * 10.0f, &first, slot + 0x10);
    func_001F9C30(2.0f, &second, &first);
    if (func_001F9CB8(slot) != 0.0f) {
        base.q = *(u128_A8AA8 *)slot;
    } else {
        base.q = *(u128_A8AA8 *)&moby->pos;
        base.f[2] += 6.0f;
    }

    {
    float radius, angle;
    int i = 0x18;
    do {
        int duration;
        radius = func_002140F8(0.5f, 1.5f);
        angle = func_00214158();
        if (moby->oclass == 0x94)
            radius *= 3.0f;
        offset.f[0] = func_001F9F90(angle) * radius;
        offset.f[1] = func_001F9FA8(angle) * radius;
        offset.f[2] = 0.0f;
        offset.f[2] = func_002140F8(1.0f, 7.0f);
        func_L00_001FF240(colors, &offset, &moby->pos);
        func_001F9BF0(colors, &base, &offset);
        velocity.q = *(u128_A8AA8 *)colors;
        func_L00_001FF4B0(D_0015EE6C * 5.0f, &velocity, &velocity);
        func_L00_001FF240(colors, &velocity, &second);
        velocity.f[2] += D_0015EE6C * 3.0f;
        size = func_002140F8(0.75f, 1.25f);
        func_001F9C30(size, &velocity, &velocity);
        index = func_002140B0(4);
        palette_entry = &palette[index];
        radius = func_002140F8(0.1f, 0.3f);
        duration = func_L00_00258BC8(60, 100);
        size = func_002140F8(3.0f, 7.0f);
        func_L01_002F9908(&offset, &velocity, *palette_entry, radius, duration,
                                size, 0.15f, 2.0f, 1);
        i--;
    } while (i >= 0);
    }
    if (moby->oclass == 0x94) {
        offset.q = *(u128_A8AA8 *)&moby->pos;
        offset.f[2] += 10.0f;
        radius = 1.0f;
        func_001F9BF0(colors, &base, &offset);
        velocity.q = *(u128_A8AA8 *)colors;
        func_L00_001FF4B0(D_0015EE6C * 5.0f, &velocity, &velocity);
        func_L00_001FF240(colors, &velocity, &second);
        velocity.f[2] += D_0015EE6C * 3.0f;
        size = func_002140F8(0.75f, 1.25f);
        func_001F9C30(size, &velocity, &velocity);
        duration = func_L00_00258BC8(60, 100);
        effect = func_L01_002F9908(&offset, &velocity, 0xc9, radius, duration,
                                        5.0f, radius, 2.0f, 1);
        if (effect != 0)
            func_001F9C30(0.35f, (char *)effect + 0x40, (char *)effect + 0x40);
    }
    func_001F9BC0(&velocity);
    func_L00_0025F4A8(moby, &velocity, &base, 0.0f, 0.0f, 20,
                            3, 4, 10.0f, 5.0f, 100000.0f,
                            0, 3.0f, 15.0f, 1, 1, -1, 0);

    i = 0x13;
    do {
        float radius, angle;
        int color, color2, duration;
        __builtin_memcpy(colors, &D_L15_001615F8, 12);
        radius = func_002140F8(0.0f, 1.5f);
        angle = func_00214158();
        if (moby->oclass == 0x94)
            radius *= 3.0f;
        offset.f[0] = func_001F9F90(angle) * radius;
        offset.f[1] = func_001F9FA8(angle) * radius;
        offset.f[2] = 0.0f;
        func_001F9BD8(&offset, &offset, &moby->pos);
        offset.f[2] += func_002140F8(1.0f, 5.5f);
        func_001F9BF0(&position, &base, &offset);
        velocity.q = position.q;
        func_L00_001FF4B0(D_0015EE6C * 5.0f, &velocity, &velocity);
        func_L00_001FF240(&position, &velocity, &first);
        size = func_002140F8(0.75f, 1.25f);
        func_001F9C30(size, &velocity, &velocity);
        size = func_002140F8(0.25f, 1.0f);
        color = func_L00_00237B70(size, 0x7f000000, 0x7f182030);
        size = func_002140F8(0.5f, 1.0f);
        color2 = func_L00_00237B70(size, 0, 0x5f5f5f);
        size = func_002140F8(1.0f, 3.0f) * 210000.0f;
        duration = func_001F9850(180);
        index = func_002140B0(3);
        effect = func_L00_0026CD70(&offset, &velocity, color, color2,
                                         size, duration, colors[index]);
        if (effect != 0)
            *(float *)((char *)effect + 0x2c) = moby->pos.z - 0.5f;
        i--;
    } while (i >= 0);
    func_L00_002584A8(moby, 0, -1);
    func_0020D678(moby);
    return;

no_hit:
    moby->hit_slot = 0xff;
    if (moby->state != 0)
        return;
    moby->update_dist = 0xff;
    moby->state = 1;
    moby->draw_dist = 0x80;
    moby->scale = moby->scale + moby->scale;
    {
    u16 save_id = moby->save_id;
    u32 saved_bit = *(u32 *)(D_0014C290 + (((s16)save_id >> 5) * 4) +
                    D_0015EE84 * 0x100) &
                    (1U << (save_id & 31));
    if (D_0013DE58 != 0 && saved_bit != 0)
        func_0020D678(moby);
    }
}
