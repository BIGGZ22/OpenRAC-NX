/* Carried from port/game/rac1-pal/hand/overlays_shared_anim_002539C8.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): FUN_L00_00252990, the moby animation blender, in C.
 *
 * In the retail program this is hand-written assembly: it moves the moby's
 * joint tables and its current and next animation frame into the scratchpad
 * with the DMA controller, decodes them there with the vector unit and writes
 * one 0x30-byte record per joint (rotation quaternion, scale, translation) to
 * the moby's joint buffer. The port has no DMA controller and no vector unit,
 * so every transfer is a copy and every vector operation is written out. The
 * routine was written from the instructions of FUN_L00_00252990 and of
 * func_L00_002530F8, which it jumps into with its registers live (the blend of
 * two different animations) and which is part of it here.
 *
 * The scratchpad is used where the retail routine uses it, so what it leaves
 * there is the same (the callers that stash the class-distance table around
 * it rely on that):
 *   0x2000  the bone table, 16 bytes a bone      0x2800  joint id -> slot, 0x7F if none
 *   0x3000  the current frame's entries          0x3600  the next frame's entries
 *   0x3C00, 0x3D00  lists of the slots a frame touches
 *   slot k at 0x40 * k: +0x00 rotation, +0x10 scale, +0x20 translation, +0x30 the next
 *   frame's translation. +0x1C holds the scale entry's count field, +0x2C the
 *   address of a slot, +0x0C and +0x3C a marker that a joint is already listed.
 *
 * A frame is a header (+6 the length in quadwords, +0xA and +0xE the sizes of its
 * two lists) and 8-byte entries from +0x10: one rotation per bone of the table,
 * then the scale list, then the translation list. A list entry names its joint
 * in byte 6.
 */
#include "types.h"

float openrac_rsqrt(float a, float b);

#define SPR(off) (0x70000000 + (off))
#define SLOT(k) (SPR(0x40 * (k)))
#define WORD(addr) (*(int *)(addr))
#define FLOAT(addr) (*(float *)(addr))

/* A DMA transfer to the scratchpad, whole quadwords. */
static void to_spr(int spr, int mem, int quadwords) {
    int i;
    for (i = 0; i < quadwords; i++) {
        qcopy((void *)(spr + 16 * i), (void *)(mem + 16 * i));
    }
}

/* The rotation entry of a bone in the frame buffer at base, as four numbers over 32768. */
static void rotation(int base, int bone, float *q) {
    short *h = (short *)(base + bone * 8);
    q[0] = (float)h[0] * (1.0f / 32768.0f);
    q[1] = (float)h[1] * (1.0f / 32768.0f);
    q[2] = (float)h[2] * (1.0f / 32768.0f);
    q[3] = (float)h[3] * (1.0f / 32768.0f);
}

/* The scale list: three halfwords over 4096 into the slot of the entry's joint, and the entry's
 * top two bits less one. Returns the entry after the list. `second` is the next frame's list. */
static int scale_list(int e, int count, int second, int *list, int *listed, int blend) {
    while (count-- > 0) {
        unsigned char *entry = (unsigned char *)e;
        unsigned short *h = (unsigned short *)e;
        int slot = SLOT(*(unsigned char *)SPR(0x2800 + entry[6]));
        int flag = (entry[7] >> 6) - 1;
        float x = (float)h[0] * (1.0f / 4096.0f);
        float y = (float)h[1] * (1.0f / 4096.0f);
        float z = (float)h[2] * (1.0f / 4096.0f);
        int marker = blend && second ? WORD(slot + 0xC) : 0;
        int at = slot + (blend && !second ? 0x00 : 0x10);
        if (blend) {
            list[*listed] = slot;
            *listed += 1;
        }
        FLOAT(at) = x;
        FLOAT(at + 4) = y;
        FLOAT(at + 8) = z;
        FLOAT(at + 12) = 0.0f;
        WORD(slot + 0x1C) = flag;
        if (blend && !second) {
            WORD(slot + 0xC) = -4;
        }
        if (blend && second) {
            *listed += marker / 4;
        }
        e += 8;
    }
    return e;
}

/* The translation list: three signed halfwords into the slot of the entry's joint. */
static int translation_list(int e, int count, int second, int *list, int *listed, int blend) {
    while (count-- > 0) {
        unsigned char *entry = (unsigned char *)e;
        short *h = (short *)e;
        int slot = SLOT(*(unsigned char *)SPR(0x2800 + entry[6]));
        int at = slot + (blend && second ? 0x30 : 0x20);
        int kept = blend ? WORD(slot + (second ? 0x3C : 0x2C)) : 0;
        if (blend) {
            list[*listed] = slot;
            *listed += 1;
        }
        FLOAT(at) = (float)h[0];
        FLOAT(at + 4) = (float)h[1];
        FLOAT(at + 8) = (float)h[2];
        FLOAT(at + 12) = 1.0f;
        if (blend) {
            WORD(slot + (second ? 0x3C : 0x2C)) = kept;
            if (second) {
                *listed += kept / 4;
            } else {
                WORD(slot + 0x3C) = -4;
            }
        }
        e += 8;
    }
    return e;
}

/* Over a list of slots, the x, y and z of the vector at `from` taken to the one at `from + 0x10`
 * by the weight f, into the vector at `to` (the fourth field of the slot is kept). The list is
 * closed with the address of slot 120, which the next-slot reads run on to. */
static void blend_list(int *list, int count, int from, int to, float f) {
    float w = 1.0f - f;
    int k, p;
    float a[3], b[3];
    list[count] = SPR(0x1E00);
    if (count <= 0) {
        return;
    }
    p = list[0];
    for (k = 0; k < 3; k++) {
        a[k] = FLOAT(p + from + 4 * k);
        b[k] = FLOAT(p + from + 0x10 + 4 * k);
    }
    for (k = 0; k < count; k++) {
        int current = p;
        float na[3], nb[3];
        int i;
        p = list[k + 1];
        for (i = 0; i < 3; i++) {
            na[i] = FLOAT(p + from + 4 * i);
            nb[i] = FLOAT(p + from + 0x10 + 4 * i);
        }
        for (i = 0; i < 3; i++) {
            FLOAT(current + to + 4 * i) = a[i] * w + b[i] * f;
            a[i] = na[i];
            b[i] = nb[i];
        }
    }
}

void FUN_L00_00252990(void *anim) {
    int a = (int)anim;
    int bones = *(int *)(a + 0x14);
    int frame_a = *(int *)(a + 0x38);
    int frame_b = *(int *)(a + 0x3C);
    int n = *(unsigned short *)(a + 2);
    unsigned char *order = (unsigned char *)*(int *)(a + 0xC);
    int bone_count = *(unsigned char *)(bones + 8);
    int weight_bits = *(int *)(a + 0x24);
    float f = *(float *)(a + 0x24);
    int i, k;
    int count_a, scales_a, translations_a;

    /* the bone table, and the map from joint id to slot (0x7F: none) */
    to_spr(SPR(0x2000), *(int *)(bones + 0x18), bone_count);
    frame_a += 0x10;
    frame_b += 0x10;
    for (i = 0; i < 128; i += 4) {
        WORD(SPR(0x2800 + i)) = 0x7F7F7F7F;
    }
    for (i = 0; i < n; i++) {
        *(unsigned char *)SPR(0x2800 + order[i]) = (unsigned char)i;
    }

    /* every slot starts as scale 1, and the dummy slot (127) too */
    k = n;
    i = 0;
    do {
        int s;
        for (s = 0; s < 2; s++) {
            FLOAT(SLOT(i + s)) = 1.0f;
            FLOAT(SLOT(i + s) + 4) = 1.0f;
            FLOAT(SLOT(i + s) + 8) = 1.0f;
            FLOAT(SLOT(i + s) + 12) = 0.0f;
            FLOAT(SLOT(i + s) + 0x10) = 1.0f;
            FLOAT(SLOT(i + s) + 0x14) = 1.0f;
            FLOAT(SLOT(i + s) + 0x18) = 1.0f;
            FLOAT(SLOT(i + s) + 0x1C) = 0.0f;
        }
        i += 2;
        k -= 2;
    } while (k > 0);
    for (i = 0; i < 2; i++) {
        FLOAT(SPR(0x1FC0 + 0x10 * i)) = 1.0f;
        FLOAT(SPR(0x1FC0 + 0x10 * i) + 4) = 1.0f;
        FLOAT(SPR(0x1FC0 + 0x10 * i) + 8) = 1.0f;
        FLOAT(SPR(0x1FC0 + 0x10 * i) + 12) = 0.0f;
    }

    /* each slot starts with the bone's own translation */
    for (i = 0; i < n; i++) {
        int s = SLOT(i);
        qcopy((void *)(s + 0x20), (void *)SPR(0x2000 + 16 * order[i]));
        qcopy((void *)(s + 0x30), (void *)SPR(0x2000 + 16 * order[i]));
        WORD(s + 0x3C) = 0;
    }
    qzero((void *)SPR(0x1FE0));
    qzero((void *)SPR(0x1FF0));

    /* the current frame */
    count_a = *(short *)(frame_a - 0xA);
    scales_a = *(short *)(frame_a - 6);
    translations_a = *(short *)(frame_a - 2);
    to_spr(SPR(0x3000), frame_a, count_a);

    if (weight_bits == 0) {
        int e = SPR(0x3000) + bone_count * 8;
        e = scale_list(e, scales_a, 0, 0, 0, 0);
        translation_list(e, translations_a, 0, 0, 0, 0);
        for (i = 0; i < n; i++) {
            int slot = SLOT(i);
            float q[4];
            rotation(SPR(0x3000), order[i], q);
            FLOAT(slot) = q[0];
            FLOAT(slot + 4) = q[1];
            FLOAT(slot + 8) = q[2];
            FLOAT(slot + 12) = q[3];
            WORD(slot + 0x2C) = SLOT(order[i]);
        }
    } else {
        int *list1 = (int *)SPR(0x3C00);
        int *list2 = (int *)SPR(0x3D00);
        int listed1 = 0, listed2 = 0;
        int count_b = *(short *)(frame_b - 0xA);
        int scales_b = *(short *)(frame_b - 6);
        int translations_b = *(short *)(frame_b - 2);
        int e;

        to_spr(SPR(0x3600), frame_b, count_b);

        e = SPR(0x3000) + bone_count * 8;
        e = scale_list(e, scales_a, 0, list1, &listed1, 1);
        translation_list(e, translations_a, 0, list2, &listed2, 1);
        e = SPR(0x3600) + bone_count * 8;
        e = scale_list(e, scales_b, 1, list1, &listed1, 1);
        translation_list(e, translations_b, 1, list2, &listed2, 1);
        blend_list(list1, listed1, 0x00, 0x10, f);
        blend_list(list2, listed2, 0x20, 0x20, f);

        if (*(unsigned char *)(a + 0x22) == *(unsigned char *)(a + 0x23)
            && *(unsigned char *)(a + 0x20) + 1 == *(unsigned char *)(a + 0x21)) {
            /* the two frames are neighbours of one sequence: a straight blend */
            float w = 1.0f - f;
            for (i = 0; i < n; i++) {
                int slot = SLOT(i);
                float p[4], q[4];
                rotation(SPR(0x3000), order[i], p);
                rotation(SPR(0x3600), order[i], q);
                FLOAT(slot) = p[0] * w + q[0] * f;
                FLOAT(slot + 4) = p[1] * w + q[1] * f;
                FLOAT(slot + 8) = p[2] * w + q[2] * f;
                FLOAT(slot + 12) = p[3] * w + q[3] * f;
                WORD(slot + 0x2C) = SLOT(order[i]);
            }
        } else {
            /* two different animations: the blend of the shorter way round, made a unit length
             * again (func_L00_002530F8) */
            float w = 1.0f - f;
            for (i = 0; i < n; i++) {
                int slot = SLOT(i);
                float p[4], q[4], m[4], s[4];
                float dot, squares, scale;
                int j;
                rotation(SPR(0x3000), order[i], p);
                rotation(SPR(0x3600), order[i], q);
                for (j = 0; j < 4; j++) {
                    p[j] = p[j] * w;
                    q[j] = q[j] * f;
                    m[j] = p[j] * q[j];
                    s[j] = p[j] + q[j];
                }
                dot = ((m[1] + m[0]) + m[2]) + m[3];
                if ((*(unsigned int *)&dot >> 31) != 0) {
                    for (j = 0; j < 4; j++) {
                        s[j] = p[j] - q[j];
                    }
                }
                squares = ((s[0] * s[0] + s[1] * s[1]) + s[2] * s[2]) + s[3] * s[3];
                scale = openrac_rsqrt(1.0f, squares);
                FLOAT(slot) = s[0] * scale;
                FLOAT(slot + 4) = s[1] * scale;
                FLOAT(slot + 8) = s[2] * scale;
                FLOAT(slot + 12) = s[3] * scale;
                WORD(slot + 0x2C) = SLOT(order[i]);
            }
        }
    }

    /* the first 0x30 bytes of each slot, packed, and out to the moby's joint buffer */
    for (i = 0; i < n; i++) {
        int from = SLOT(i);
        int to = SPR(0x30 * i);
        int q0[4], q1[4], q2[4];
        qcopy(q0, (void *)from);
        qcopy(q1, (void *)(from + 0x10));
        qcopy(q2, (void *)(from + 0x20));
        qcopy((void *)to, q0);
        qcopy((void *)(to + 0x10), q1);
        qcopy((void *)(to + 0x20), q2);
    }
    for (i = 0; i < n * 3; i++) {
        qcopy((void *)(*(int *)(a + 0x10) + 16 * i), (void *)SPR(16 * i));
    }
}
