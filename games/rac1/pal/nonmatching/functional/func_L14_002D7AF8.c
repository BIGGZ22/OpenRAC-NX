/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L14_002d67e8), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: its PAL file declares a symbol it uses with another type, so it was not built there. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_D7AF8;
struct Moby_D7AF8;
struct Manip;
struct GifEntry;
struct AnimSeq_D7AF8;
struct MobyClass_D7AF8 {
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
    struct AnimSeq_D7AF8 *seqs[1]; /* animation sequences, indexed by Moby.seq */
};
struct Moby_D7AF8 {
    Vec4f_D7AF8 bsphere;
    Vec4f_D7AF8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00197300[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_D7AF8 *pclass;
    struct Moby_D7AF8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_D7AF8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_D7AF8 *moby);
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
    Vec4f_D7AF8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_D7AF8 rot_row1;
    Vec4f_D7AF8 rot_row2;
    u8 padF0[0x10];
};
typedef struct {
    char _pad00[0x10];
    unsigned char nframes; /* 0x10 */
} AnimSeq_D7AF8;
extern int D_L14_0015F7EC MACRO_ADDR;
extern int func_L00_0025EFC0(void *, void *, float *, int *, float *, int, float, float, float);
extern short D_L14_00161B08;
typedef int u128_67e8 __attribute__((mode(TI)));
typedef struct { char pad00[0x64]; int seg; float frac; } RacerSeg_67e8;
typedef struct { float x, y, z, w; } V_67e8;
extern V_67e8 D_0013F4D0;
typedef struct {
    V_67e8 dir; /* 0x00 */
    struct Moby_D7AF8 *self; /* 0x10 */
    int flags; /* 0x14 */
    unsigned char b18;
    unsigned char b19;
    short oclass; /* 0x1A */
    float f1C;
    int i20;
    int pad[3];
} Ray_67e8;
extern short D_L14_00161B0C;
extern short D_L14_00161B10;
extern short D_L14_00161B14;
extern short D_L14_00161B18;
extern short D_L14_00161B1C;
extern short D_L14_00161B20;
extern short D_L14_00161B24;
extern short D_L14_00161B28;
extern char D_L14_00180AC0[];
extern char D_L14_0015F660_D7AF8[] __asm__("D_L14_0015F660") MACRO_ADDR;
extern void func_L00_0025E860_D7AF8(void *, void *, void *, void *, int, float) __asm__("func_L00_0025E860");
extern void func_L00_0025C918_D7AF8(void *, void *, float, float, float, float) __asm__("func_L00_0025C918");
extern float func_L00_001FF860(float, float);
extern float func_001FA790(float, float);
extern int func_001F9908(void *);
extern int func_001F9850(int);
extern float func_001FA888(int);
extern int func_001FA898(float);
extern void func_0020DAF8_D7AF8(struct Moby_D7AF8 *, int, void *) __asm__("func_0020DAF8");
extern void func_L00_00250800_D7AF8(struct Moby_D7AF8 *, int, void *) __asm__("func_L00_00250800");
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001FA4A0(void *, void *);
extern void func_001FA540(void *, void *, void *);
extern void func_002150B0(void *, void *);
extern void func_001FA5C8(void *, void *, void *, float);
extern void func_L00_001FFED8(void *, int, float);
extern void func_L00_002E33D8(struct Moby_D7AF8 *, int, int);
extern int func_L00_002E3640(struct Moby_D7AF8 *, void *, void *);
extern int func_L00_001EFFF0_D7AF8(void *, void *, int, struct Moby_D7AF8 *, void *) __asm__("func_L00_001EFFF0");
extern char *func_L00_002757E8_D7AF8(void *, void *, int, struct Moby_D7AF8 *) __asm__("func_L00_002757E8");
extern int func_L00_0023F0D0_D7AF8(void *, float, float, float, float, float) __asm__("func_L00_0023F0D0");
extern void func_L00_0023F1D0(int);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l14/gameplay/entities/002d6358.c, FUN_L14_002d67e8. */
void func_L14_002D7AF8(struct Moby_D7AF8 *m) {
    char *d = (char *)m->pvars;
    char *path = *(char **)(D_L14_0015F7EC + *(int *)(d + 0x60) * 32 + 0x10);
    V_67e8 near;
    V_67e8 v[4];
    V_67e8 ma[4];
    V_67e8 mb[4];
    V_67e8 mc[4];
    V_67e8 md[4];
    V_67e8 scl;
    V_67e8 rot;
    V_67e8 one;
    Ray_67e8 ray;
    int ti;
    float tf;
    float z;
    float yaw;
    float blend;
    float a;

    if (*(int *)(d + 0x64) >= *(short *)(d + 0x8E)) {
        float sum = 0.0f;
        int k;
        for (k = *(short *)(d + 0x8E); k < *(int *)(d + 0x64); k++) {
            sum += *(float *)(path + k * 16 + 0x1C);
        }
        sum += *(float *)(d + 0x68);
        sum /= *(float *)(d + 0x78);
        if (sum > 1.0f) {
            sum = 1.0f;
        }
        *(float *)(d + 0x74) = (*(float *)&D_L14_00161B08) + (-3.0f - (*(float *)&D_L14_00161B08)) * sum;
    }
    *(int *)(d + 0x68) = 0;
    *(int *)(d + 0x64) = 0;
    z = *(float *)(d + 0x68);
    func_L00_0025EFC0(path, &D_0013F4D0, &near.x, (int *)(d + 0x64), (float *)(d + 0x68), 0, 20.0f, 5.0f, z);
    func_L00_0025E860_D7AF8(path, &near, d + 0x64, d + 0x68, 0, -*(float *)(d + 0x74));
    func_L00_0025C918_D7AF8(&m->pos.x, d + 0x90, near.x, 0.01f, 0.2f, z);
    func_L00_0025C918_D7AF8(&m->pos.y, d + 0x94, near.y, 0.01f, 0.2f, z);
    func_L00_0025C918_D7AF8(&m->pos.z, d + 0x98, near.z, 0.01f, 0.2f, z);
    {
        int i = *(int *)(d + 0x64);
        char *p0 = path + i * 16;
        char *p1 = path + (i + 5) * 16;
        m->rot.z = func_L00_001FF860(*(float *)(p1 + 0x10) - *(float *)(p0 + 0x10), *(float *)(p1 + 0x14) - *(float *)(p0 + 0x14));
    }
    if (m->command == 5 && func_001F9908(d + 0x1C8) != 0) {
        m->command = 1;
    }
    if (m->command == 1) {
        *(int *)(d + 0x1C0) = func_001F9850((*(int *)&D_L14_00161B0C));
        *(int *)(d + 0x1C4) = 0;
        m->command = 2;
        func_0020DAF8_D7AF8(m, 4, v);
        func_0020DAF8_D7AF8(m, 5, ma);
        func_L00_001FF4B0(&ma[0], &ma[0], 1.0f);
        func_L00_001FF4B0(&ma[1], &ma[1], 1.0f);
        func_L00_001FF4B0(&ma[2], &ma[2], 1.0f);
        func_001FA4A0(mb, ma);
        mb[3].w = 1.0f;
        mb[3].x = mb[3].y = mb[3].z = 0.0f;
        func_001FA540(mc, mb, v);
        func_002150B0(d + 0x1D0, mc);
    }
    yaw = m->rot.z;
    if ((unsigned char)(m->command - 2) < 3) {
        float turn;
        float t;
        float k;
        func_0020DAF8_D7AF8(m, 0, d + 0xA0);
        blend = 1.0f;
        t = func_001FA888(*(int *)(d + 0x1C0)) / func_001FA888(func_001F9850((*(int *)&D_L14_00161B0C)));
        k = (*(float *)&D_L14_00161B10);
        ti = ((RacerSeg_67e8 *)d)->seg;
        tf = ((RacerSeg_67e8 *)d)->frac;
        func_L00_0025E860_D7AF8(path, &v[0], &ti, &tf, 0, k + (1.0f - k) * t);
        qcopy(d + 0x1E0, &v[0]);
        func_001F9BF0(&v[1], &v[0], &m->pos);
        yaw = func_L00_001FF860(v[1].x, v[1].y);
        turn = func_001FA790(yaw, m->rot.z);
        if (turn > 1.2217305f) {
            turn = 1.2217305f;
        } else if (turn < -1.2217305f) {
            turn = -1.2217305f;
        }
        (*(int *)(d + 0x1C4))++;
        if (func_001F9850(30) < *(int *)(d + 0x1C4)) {
            if (m->command == 4) {
                m->command = 0;
            } else {
                unsigned char s = 3;
                func_L00_002E33D8(m, s, 1);
                m->command = s;
            }
            *(int *)(d + 0x1C4) = func_001F9850(30);
        }
        if (m->command == 2 || m->command == 4) {
            blend = func_001FA888(*(int *)(d + 0x1C4)) / func_001FA888(func_001F9850(30));
            if (m->command == 4) {
                blend = 1.0f - blend;
            }
            turn = (turn - 0.0f) * blend + 0.0f;
        }
        func_L00_001FFED8(d + 0x150, 2, turn);
        func_L00_00250800_D7AF8(m, 4, &v[2]);
        *(u128_67e8 *)&v[3] = 0;
        v[3].z = 1.0f;
        v[3].w = 1.0f;
        func_L00_001FF4B0(&ma[1], &v[1], 1.0f);
        func_001F9CA0(&ma[2], &ma[1], &v[3]);
        func_L00_001FF4B0(&ma[2], &ma[2], 1.0f);
        func_001F9CA0(&ma[0], &ma[2], &ma[1]);
        ma[3].x = ma[3].y = ma[3].z = 0.0f;
        ma[3].w = 1.0f;
        func_0020DAF8_D7AF8(m, 5, mb);
        func_L00_001FF4B0(&mb[0], &mb[0], 1.0f);
        func_L00_001FF4B0(&mb[1], &mb[1], 1.0f);
        func_L00_001FF4B0(&mb[2], &mb[2], 1.0f);
        func_001FA4A0(mc, mb);
        mc[3].x = mc[3].y = mc[3].z = 0.0f;
        mc[3].w = 1.0f;
        func_001FA540(md, mc, ma);
        *(u128_67e8 *)&scl = 0;
        scl.w = 1.0f;
        scl.y = 512.0f / m->scale;
        func_002150B0(&rot, md);
        if (m->command == 2 || m->command == 4) {
            func_001FA5C8(d + 0x190, d + 0x1D0, &rot, blend);
        } else {
            qcopy(d + 0x190, &rot);
        }
        *(char *)(d + 0x183) = 1;
        *(float *)(d + 0x18C) = 1.0f;
        *(u128_67e8 *)&one = 0;
        one.x = 1.0f;
        one.y = 1.0f;
        one.z = 1.0f;
        one.w = 1.0f;
        qcopy(d + 0x1A0, &one);
        qcopy(d + 0x1B0, &scl);
        if (func_001F9908(d + 0x1C0) != 0) {
            m->command = 4;
            *(int *)(d + 0x1C4) = 0;
        }
        if (m->command == 3) {
            if (func_L00_002E3640(m, d + 0xA0, d + 0x1E0) == 0) {
                func_L00_002E33D8(m, 3, 1);
            }
            ray.flags = 0x10001;
            ray.self = m;
            ray.i20 = 1;
            ray.f1C = 1.0f;
            ray.dir.x = func_001F9F90(m->rot.z);
            ray.dir.y = func_001F9FA8(m->rot.z);
            ray.dir.z = 1.0f;
            ray.dir.w = 5627.925f;
            ray.b19 = 1;
            ray.oclass = m->oclass;
            ray.b18 = 0;
            func_L00_001EFFF0_D7AF8(d + 0xD0, d + 0x1E0, 9, m, &ray);
        } else if (m->command == 2 || m->command == 4) {
            char *p;
            float r;
            char *muzzle = d + 0xD0;
            func_001F9C30(&ray, d + 0xA0, (*(float *)&D_L14_00161B14));
            func_001F9BD8(&ray, &ray, muzzle);
            p = func_L00_002757E8_D7AF8(&ray, D_L14_0015F660_D7AF8, 0x7F, m);
            if (p != 0) {
                char *q = p + 0x20;
                int n;
                *(float *)(p + 0xC) = ((*(float *)&D_L14_00161B18) - 1000.0f) * blend + 1000.0f;
                *(short *)(q + 0x14) = func_001FA898(blend * 111.0f + 16.0f);
                *(short *)(q + 0x16) = 1;
                n = func_001F9850(4);
                *(short *)(p + 0xA) = n;
                *(float *)(q + 0x10) = 1.0f / func_001FA888((short)n);
                *(int *)(q + 0x18) = 0x7F7F7F;
            }
            qcopy(&ray.self, muzzle);
            ((float *)&ray.self)[2] -= 0.5f;
            r = ((*(float *)&D_L14_00161B1C) - 0.1f) * blend + 0.1f;
            if (*(int *)(d + 0xF0) == -1) {
                *(int *)(d + 0xF0) = func_L00_0023F0D0_D7AF8(&ray.self, r, 0.0f, (*(float *)&D_L14_00161B20), (*(float *)&D_L14_00161B24), (*(float *)&D_L14_00161B28));
            }
            if (*(int *)(d + 0xF0) >= 0) {
                float *l = (float *)(D_L14_00180AC0 + (*(int *)(d + 0xF0) << 5));
                qcopy(l + 4, &ray.self);
                l[7] = r;
                l[0] = (*(float *)&D_L14_00161B20);
                l[1] = (*(float *)&D_L14_00161B24);
                l[2] = (*(float *)&D_L14_00161B28);
            }
        } else if (*(int *)(d + 0xF0) != -1) {
            func_L00_0023F1D0(*(int *)(d + 0xF0));
            *(int *)(d + 0xF0) = -1;
        }
    } else if (*(int *)(d + 0xF0) != -1) {
        func_L00_0023F1D0(*(int *)(d + 0xF0));
        *(int *)(d + 0xF0) = -1;
    }
    func_001F9BF0(&v[0], &D_0013F4D0, &m->pos);
    a = func_001FA790(func_L00_001FF860(v[0].x, v[0].y), yaw);
    if (a > 1.2217305f) {
        a = 1.2217305f;
    } else if (a < -1.2217305f) {
        a = -1.2217305f;
    }
    func_L00_001FFED8(d + 0x110, 2, a);
}
