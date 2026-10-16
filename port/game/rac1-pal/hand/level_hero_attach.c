/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only equipment attachment, PAL 0020FC18..0021033C. Reconstructed
 * from the retail body and the sibling's nonmatching/shared candidate.
 * Pointer fields retain EE layout through hostgen. Not a PS2 match.
 */
typedef struct HeroAttachMoby {
    unsigned char pad00[0x10];
    float position[4];
    unsigned char pad20[4];
    void* model;
    unsigned char pad28[4];
    float scale;
    unsigned char state, draw;
    short distance;
    unsigned short flags;
    unsigned char pad36[2];
    unsigned int lighting[2];
    float angles[4];
    int frame, next_frame;
    unsigned char pad58[0x10];
    void *joints, *next_joints;
    unsigned char pad70[0x20];
    unsigned int color;
    unsigned char pad94[0x12];
    short class_id;
    unsigned char padA8[0x18];
    float basis[3][4];
} HeroAttachMoby;

/* 0x50 bytes a slot, at state +0x1070: retail reads primary +0x20, secondary +0x24, auxiliary
 * +0x2C, detached +0x3A (a thrown wrench) and item +0x48. */
typedef struct HeroAttachSlot {
    float point[4], angles[4];
    HeroAttachMoby *primary, *secondary;
    unsigned char pad28[4];
    HeroAttachMoby* auxiliary;
    unsigned char pad30[0xA], detached, pad3B[0xD];
    int item, pad4C;
} HeroAttachSlot;

typedef struct HeroAttachState {
    unsigned char pad00[0x80];
    float position[4], angles[4];
    unsigned char padA0[0xA20];
    float matrices[22][4][4];
    unsigned char pad1040[0x30];
    HeroAttachSlot slots[7];
    unsigned char pad12A0[0xAE0];
    float auxiliary_point[4];
    unsigned char pad1D90[0x2F0];
    HeroAttachMoby* hero;
} HeroAttachState;

typedef struct HeroAttachDefinition {
    unsigned char pad00[0xC];
    int matrix;
    unsigned char pad10[8];
    int euler;
    unsigned char pad1C[0x30];
} HeroAttachDefinition;

extern unsigned char ha_state[] __asm__("D_0013E633");
extern int ha_mode __asm__("D_L00_0015F6A8");
extern int ha_euler __asm__("D_L00_0015F770");
extern int ha_clock __asm__("D_L00_0015F6B0");
extern unsigned char ha_settings[] __asm__("D_0015EEB4");
extern HeroAttachDefinition ha_definitions[] __asm__("D_L00_00179BC0");
extern unsigned char ha_glove[] __asm__("D_L00_0017A6C0");
extern unsigned char ha_glove_out[] __asm__("D_L00_0017C580");
extern unsigned char ha_head[] __asm__("D_L00_0017A708");
extern unsigned char ha_head_other[] __asm__("D_L00_0015F788");
extern unsigned char ha_head_out[] __asm__("D_L00_0017C780");
extern float ha_boot_save[] __asm__("D_L00_0017AA60");
extern float ha_boot_scale[] __asm__("D_L00_0017A780");
extern unsigned char ha_boot_left[] __asm__("D_L00_0017A750");
extern unsigned char ha_boot_right[] __asm__("D_L00_0017A768");
extern unsigned char ha_boot_left_out[] __asm__("D_L00_0017C980");
extern unsigned char ha_boot_right_out[] __asm__("D_L00_0017CA80");
extern void ha_angles(void*, void*) __asm__("func_002153E8");
extern void ha_advance(void*) __asm__("func_L00_002514B8");
extern void ha_update(void*) __asm__("func_L00_00251E30");
extern int ha_is_glove(int) __asm__("func_L00_0020DB68");
extern int ha_is_head(int) __asm__("func_L00_0020DBB0");
extern int ha_is_boot(int) __asm__("func_L00_0020DBD8");
extern void ha_copy_basis(void*, void*) __asm__("func_001FA480");
extern void ha_normalize(void*) __asm__("func_00214F78");
extern void ha_rotate(void*, void*, void*) __asm__("func_001F9EC0");
extern float ha_length(void*) __asm__("func_001F9CE8");
extern float ha_angle(float, float) __asm__("func_L00_001FF860");
extern void ha_bounds(void*) __asm__("func_0020EEE8");
extern void ha_pose(void*, void*, void*, int, int) __asm__("func_L00_00209940");
extern void ha_weapon(void*, void*) __asm__("func_L00_0020F7F8");
extern void ha_cross(void*, void*, void*) __asm__("func_001F9CA0");
extern HeroAttachMoby* ha_spawn(int) __asm__("func_0020D348");
extern void ha_joint(void*, int, void*) __asm__("func_0020DAF8");
extern void ha_auxiliary(void) __asm__("func_L00_00208318");
extern int ha_ticks(int) __asm__("func_001F9850");
extern float ha_float(int) __asm__("func_001FA888");
extern float ha_sin(float) __asm__("func_001F9FA8");
extern int ha_truncate(float) __asm__("func_001FA898");
extern void ha_trap(void) __asm__("func_001F9978");

static void ha_copy4(float* to, float* from) {
    int k;
    for (k = 0; k < 4; ++k) to[k] = from[k];
}

void func_L00_0020FC18(void) {
    HeroAttachState* g = (HeroAttachState*)(ha_state + 0xE1D);
    HeroAttachSlot* slot;
    HeroAttachMoby *m, *aux;
    HeroAttachDefinition* def;
    float (*matrix)[4];
    /* Retail sp+0x20..0x5F is ONE matrix; sp+0x50 is its translation row. */
    float joint[4][4], vec[4], saved[2][4];
    void *pose, *out;
    int i, j, kind, glove, head, boot, period, red, green, blue;
    float length, divisor, phase, wave;
    for (i = 0; i < 7; ++i) {
        if ((ha_mode == 2 || ha_mode == 6) && i != 4 && i != 5) continue;
        slot = &g->slots[i];
        for (j = 0; j < ((i == 1 || i == 3 || i == 4) ? 2 : 1); ++j) {
            m = j ? slot->secondary : slot->primary;
            if (m) {
                m->lighting[0] = g->hero->lighting[0];
                m->lighting[1] = g->hero->lighting[1];
                def = &ha_definitions[slot->item];
                kind = (i == 1 && j == 1) ? 3 : def->matrix;
                matrix = g->matrices[kind];
                if (slot->detached) {
                    ha_copy4(slot->point, matrix[3]);
                    ha_angles(matrix, slot->angles);
                    ha_advance(m);
                    ha_update(m);
                    m->flags &= 0xFFFB;
                } else {
                    ha_copy4(m->position, matrix[3]);
                    if (ha_euler && def->euler) {
                        ha_advance(m);
                        /* The update may change the global mode. */
                        if (!ha_euler || i != 0) ha_angles(matrix, m->angles);
                        else ha_weapon(m, matrix);
                        ha_update(m);
                        if (ha_settings[1]) ha_cross(m->basis[2], m->basis[0], m->basis[1]);
                        ha_bounds(m);
                        m->flags |= 6;
                    } else {
                        glove = ha_is_glove(i);
                        head = ha_is_head(i);
                        boot = ha_is_boot(i);
                        if (!glove && !head && !boot) ha_advance(m);
                        ha_update(m);
                        ha_copy_basis(m->basis, matrix);
                        if (!glove && !head && !boot) ha_normalize(m->basis);
                        vec[0] = 1.0f; vec[1] = 0.0f; vec[2] = 0.0f;
                        ha_rotate(vec, vec, m->basis);
                        length = ha_length(vec);
                        m->angles[1] = -ha_angle(length, vec[2]);
                        m->angles[2] = ha_angle(vec[0], vec[1]);
                        ha_bounds(m);
                        m->flags |= 6;
                        if (glove || head || boot) {
                            if (glove) {
                                pose = ha_glove; out = ha_glove_out;
                            } else if (head) {
                                pose = m->class_id == 0x1B1 ? ha_head : ha_head_other;
                                out = ha_head_out;
                            } else {
                                ha_copy4(saved[0], ha_boot_save);
                                ha_copy4(saved[1], ha_boot_save + 0xB0 / 4);
                                ha_boot_scale[0x398 / 4] = 1.0f;
                                ha_boot_scale[0x2E0 / 4] = 1.0f;
                                ha_boot_scale[0x2E4 / 4] = 1.0f;
                                ha_boot_scale[0x2E8 / 4] = 1.0f;
                                ha_boot_scale[0x390 / 4] = 1.0f;
                                ha_boot_scale[0x394 / 4] = 1.0f;
                                pose = j ? ha_boot_right : ha_boot_left;
                                out = j ? ha_boot_right_out : ha_boot_left_out;
                            }
                            ha_pose(pose, out, m->model, !glove && head && m->class_id == 0x1B1 ? 6 : 0, 0);
                            m->frame = m->next_frame = 0;
                            m->joints = m->next_joints = out;
                            if (!glove && !head) {
                                ha_copy4(ha_boot_save, saved[0]);
                                ha_copy4(ha_boot_save + 0xB0 / 4, saved[1]);
                            }
                        }
                    }
                }
            }
            if (i != 3 || j != 1 || !m) continue;
            aux = g->slots[3].auxiliary;
            if (!aux) {
                aux = ha_spawn(0x4B4);
                g->slots[3].auxiliary = aux;
                if (!aux) continue;
                aux->color = 0x801432D7u;
                ha_copy4(aux->position, g->position);
                ha_copy4(aux->angles, g->angles);
                aux->distance = 0x40;
                aux->draw = 1; aux->state = 0; aux->flags = 0;
                aux->scale *= 1.3f;
                ha_update(aux);
                if (!g->slots[3].auxiliary) continue;
            }
            aux = g->slots[3].auxiliary;
            if (aux->flags & 1) continue;
            ha_joint(m, 6, joint);
            ha_copy4(g->auxiliary_point, joint[3]);
            ha_auxiliary();
            ha_copy4(aux->position, joint[3]);
            ha_copy_basis(aux->basis, joint);
            ha_update(aux);
            aux->lighting[0] = g->hero->lighting[0];
            aux->lighting[1] = g->hero->lighting[1];
            period = ha_ticks(0x78);
            divisor = ha_float(period);
            if (!period) { ha_trap(); return; }
            phase = (float)(ha_clock % period) / divisor;
            phase = phase + phase;
            phase = phase * 3.14159265f;
            wave = ha_sin(phase + -3.14159265f);
            red = ha_truncate(wave * 90.0f) + 215;
            green = ha_truncate(wave * 50.0f) + 50;
            blue = ha_truncate(wave * 10.0f) + 20;
            if (red >= 256) red = 255;
            aux->color = 0x80000000u | ((unsigned int)blue << 16)
                | ((unsigned int)green << 8) | (unsigned int)red;
            aux->flags |= 0x10;
        }
    }
}
