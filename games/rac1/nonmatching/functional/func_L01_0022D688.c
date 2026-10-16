/* functional: high. Exact logic of retail func_L01_0022D688 (hero ledge probe).
 * Probes for a reachable ledge ahead of the hero and records its edge.
 * Ported from NTSC FUN_L01_0022d090 (src/overlays/shared/ui/help/00226f10.c) and verified against PAL assembly.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

typedef int OvlQuad_22D688 __attribute__((mode(TI)));

typedef struct {
    float f[4];
} Vec4_22D688;

typedef struct {
    u8 pad0[0x18];
    void *moby;       /* 0x18: hit moby */
    s32 unk1C;
    union {
        OvlQuad_22D688 q;
        float f[4];
    } point;          /* 0x20: hit point */
    u8 pad30[0x10];
    float normal_x;   /* 0x40 */
    float normal_y;   /* 0x44 */
    float normal_z;   /* 0x48 */
} CollisionHit_22D688;

typedef struct {
    u8 pad0[0x1E];
    u16 flags1E;      /* 0x1E */
    u8 pad20[0x1C];
    u32 flags3C;      /* 0x3C */
} MobyData_22D688;

typedef struct {
    u8 pad0[0x80];
    Vec4_22D688 pos;        /* 0x80 */
    Vec4_22D688 rot;        /* 0x90 */
    u8 padA0[0x40];
    Vec4_22D688 velocity;   /* 0xE0 */
    u8 padF0[0x20];
    Vec4_22D688 unk110;     /* 0x110 */
    u8 pad120[0x90];
    s16 unk1B0;             /* 0x1B0 */
    u8 pad1B2[0x82];
    float unk234;           /* 0x234 */
    u8 pad238[0xA0];
    float ground_z;         /* 0x2D8 */
    u8 pad2DC[0x2C];
    s16 unk308;             /* 0x308 */
    u8 pad30A[0x114];
    s16 unk41E;             /* 0x41E */
    u8 pad420[0x80];
    float unk4A0;           /* 0x4A0 */
    u8 pad4A4[0x2C];
    Vec4_22D688 unk4D0;     /* 0x4D0 */
    float unk4E0;           /* 0x4E0 */
    float unk4E4;           /* 0x4E4 */
    s32 unk4E8;             /* 0x4E8 */
    u8 pad4EC[0x8];
    u32 unk4F4;             /* 0x4F4 */
    void *unk4F8;           /* 0x4F8 */
    u8 pad4FC[0x444];
    float unk940;           /* 0x940 */
    u8 pad944[0x164];
    float anim_frame;       /* 0xAA8 */
    u8 padAAC[0x15D4];
    void *moby;             /* 0x2080 */
    s32 current_state;      /* 0x2084 */
    u8 pad2088[4];
    s32 control_mode;       /* 0x208C */
    u8 pad2090[0x14];
    u8 unk20A4;             /* 0x20A4 */
} Hero_22D688;

extern Hero_22D688 G_hero_22D688 __asm__("D_0013F450");
extern CollisionHit_22D688 D_L01_00174340_22D688 __asm__("D_L01_00174340");
extern float D_0015EE6C_22D688 __asm__("D_0015EE6C");

extern int func_001F9850_22D688(int) __asm__("func_001F9850");
extern void func_L00_0025A540_22D688(void *probe, void *pos, void *unk110, int frames, float launch_scale) __asm__("func_L00_0025A540");
extern float func_001F9F90_22D688(float) __asm__("func_001F9F90");
extern float func_001F9FA8_22D688(float) __asm__("func_001F9FA8");
extern int func_L00_001EFFF0_22D688(void *from, void *to, int mask, void *ignore, int) __asm__("func_L00_001EFFF0");
extern int func_L00_001F3958_22D688(void) __asm__("func_L00_001F3958");
extern float func_001F9CE8_22D688(void *) __asm__("func_001F9CE8");
extern float func_L00_001FF860_22D688(float, float) __asm__("func_L00_001FF860");
extern MobyData_22D688 *func_L00_0025D390_22D688(void *) __asm__("func_L00_0025D390");
extern int func_L00_001F1D20_22D688(float edge_offset, float r, void *head, int mask, void *moby) __asm__("func_L00_001F1D20");
extern float func_001FA748_22D688(float, float) __asm__("func_001FA748");
extern float func_001FA850_22D688(float, float) __asm__("func_001FA850");

static inline void qcopy_22D688(void *dst, const void *src) {
    *(OvlQuad_22D688 *)dst = *(const OvlQuad_22D688 *)src;
}

static inline void circle_offset_22D688(Vec4_22D688 *v, float ang, float r) {
    r += r;
    v->f[0] += func_001F9F90_22D688(ang) * r;
    v->f[1] += func_001F9FA8_22D688(ang) * r;
}

void func_L01_0022D688(void) {
    Vec4_22D688 probe, top, low, from, head, wall, to, edge;
    float drop, fwd, up, reach, step, top_margin, max_slope, edge_offset;
    float wall_yaw, launch_scale, ang, five, one, limit, inc, d, half;
    int found, hit, i, j, material, allowed, moby_allowed, frames;
    MobyData_22D688 *moby_data;

    if (G_hero_22D688.unk308 == 2)
        return;
    G_hero_22D688.unk4E8 = 0;
    if (G_hero_22D688.unk1B0 != 0)
        return;

    allowed = 0;
    if (G_hero_22D688.unk20A4 == 0) {
        if ((G_hero_22D688.control_mode == 4 &&
             (G_hero_22D688.unk41E != 0 || G_hero_22D688.velocity.f[2] < D_0015EE6C_22D688 * 3.5f)) ||
            G_hero_22D688.control_mode == 2 || G_hero_22D688.current_state == 8 ||
            G_hero_22D688.current_state == 0x1A || G_hero_22D688.current_state == 0x1B) {
            allowed = 1;
        }
        if (G_hero_22D688.current_state == 0xE && G_hero_22D688.anim_frame < 37.0f) {
            allowed = 0;
        }
    }
    if (!allowed)
        return;

    drop = -0.7f;
    fwd = 0.3f;
    up = 1.5f;
    reach = 1.15f;
    edge_offset = 0.45f;
    launch_scale = G_hero_22D688.unk940;
    if (G_hero_22D688.control_mode == 4) {
        launch_scale = G_hero_22D688.unk4A0;
    }
    frames = func_001F9850_22D688(2);
    step = 0.25f;
    top_margin = 0.15f;

    func_L00_0025A540_22D688(&probe, &G_hero_22D688.pos, &G_hero_22D688.unk110, frames, launch_scale);
    found = 0;
    probe.f[0] += func_001F9F90_22D688(G_hero_22D688.rot.f[2]) * G_hero_22D688.unk234;
    probe.f[1] += func_001F9FA8_22D688(G_hero_22D688.rot.f[2]) * G_hero_22D688.unk234;

    for (i = 0; !found && i < 4; i++) {
        probe.f[0] += func_001F9F90_22D688(G_hero_22D688.rot.f[2]) * fwd * step;
        probe.f[1] += func_001F9FA8_22D688(G_hero_22D688.rot.f[2]) * fwd * step;
        qcopy_22D688(&top, &probe);
        top.f[2] += up + top_margin;
        qcopy_22D688(&low, &probe);
        low.f[2] += reach;
        if (func_L00_001EFFF0_22D688(&top, &low, 4, G_hero_22D688.moby, 0)) {
            material = func_L00_001F3958_22D688();
            if (material != 9 && material != 12 && material != 0) {
                if (func_L00_001FF860_22D688(D_L01_00174340_22D688.normal_z,
                                             func_001F9CE8_22D688(&D_L01_00174340_22D688.normal_x)) < 0.34906584f) {
                    found = 1;
                }
            }
        }
    }
    if (!found)
        return;

    if (D_L01_00174340_22D688.moby != 0) {
        moby_allowed = 0;
        moby_data = func_L00_0025D390_22D688(D_L01_00174340_22D688.moby);
        if (moby_data != 0 && (moby_data->flags1E & 1)) {
            moby_allowed = 1;
        } else if (moby_data != 0) {
            moby_allowed = moby_data->flags3C & 1;
        }
        if (!moby_allowed)
            return;
    }

    G_hero_22D688.unk4F8 = D_L01_00174340_22D688.moby;
    if (G_hero_22D688.pos.f[2] + up < D_L01_00174340_22D688.point.f[2])
        return;

    qcopy_22D688(&top, &D_L01_00174340_22D688.point);
    qcopy_22D688(&low, &D_L01_00174340_22D688.point);
    low.f[2] += 0.05f;
    qcopy_22D688(&from, &G_hero_22D688.pos);
    from.f[2] = low.f[2];
    if (func_L00_001EFFF0_22D688(&from, &low, 4, G_hero_22D688.moby, 0))
        return;

    qcopy_22D688(&head, &D_L01_00174340_22D688.point);
    head.f[2] += 0.6f;
    if (func_L00_001F1D20_22D688(edge_offset, 0.6f, &head, 4, G_hero_22D688.moby)) {
        G_hero_22D688.unk4F4 |= 1;
    } else {
        G_hero_22D688.unk4F4 &= ~1;
    }

    ang = func_L00_001FF860_22D688(top.f[0] - G_hero_22D688.pos.f[0], top.f[1] - G_hero_22D688.pos.f[1]);
    wall_yaw = G_hero_22D688.ground_z;
    max_slope = top.f[2];
    G_hero_22D688.unk4E0 = max_slope;
    max_slope -= wall_yaw;
    if (max_slope < 1.8f)
        return;

    five = 5.0f;
    one = 1.0f;
    hit = 0;
    for (j = 0; j < 5; j++) {
        qcopy_22D688(&wall, &G_hero_22D688.pos);
        qcopy_22D688(&to, &top);
        circle_offset_22D688(&to, ang, G_hero_22D688.unk234);
        to.f[2] += drop * (one - (float)j / five);
        wall.f[2] = to.f[2];
        if (func_L00_001EFFF0_22D688(&wall, &to, 2, 0, 0)) {
            hit = 1;
            G_hero_22D688.unk4E4 = func_L00_001FF860_22D688(D_L01_00174340_22D688.normal_x,
                                                            D_L01_00174340_22D688.normal_y);
            break;
        }
    }
    if (!hit)
        return;

    if (func_001FA850_22D688(G_hero_22D688.rot.f[2],
                            func_001FA748_22D688(G_hero_22D688.unk4E4, 3.14159274f)) < 1.13446403f) {
        limit = 2.4f;
        inc = 0.03f;
        d = 0.0f;
        half = 0.5f;
        do {
            qcopy_22D688(&to, &top);
            to.f[2] = top.f[2];
            to.f[0] += func_001F9F90_22D688(G_hero_22D688.unk4E4) * d;
            to.f[1] += func_001F9FA8_22D688(G_hero_22D688.unk4E4) * d;
            qcopy_22D688(&wall, &to);
            to.f[2] += half;
            wall.f[2] -= half;
            if (!func_L00_001EFFF0_22D688(&to, &wall, 4, G_hero_22D688.moby, 0)) {
                qcopy_22D688(&edge, &to);
                edge.f[2] = top.f[2];
                d = inc * half;
                d = -d;
                edge.f[0] += func_001F9F90_22D688(G_hero_22D688.unk4E4) * d;
                edge.f[1] += func_001F9FA8_22D688(G_hero_22D688.unk4E4) * d;
                qcopy_22D688(&G_hero_22D688.unk4D0, &edge);
                G_hero_22D688.unk4D0.f[0] += func_001F9F90_22D688(G_hero_22D688.unk4E4) * edge_offset;
                wall_yaw = func_001F9FA8_22D688(G_hero_22D688.unk4E4) * edge_offset;
                G_hero_22D688.unk4E8 = 1;
                G_hero_22D688.unk4D0.f[1] = wall_yaw + G_hero_22D688.unk4D0.f[1];
                wall_yaw = -1.43f;
                G_hero_22D688.unk4D0.f[2] += wall_yaw;
                return;
            }
            d += inc;
        } while (d < limit);
    }
}
