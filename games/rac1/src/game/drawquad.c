#include "common.h"
#include "vector.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * drawquad.cpp in the original source; text 0x1F7C60-0x1F9810.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];

ASM_FUNC("asm/handwritten/text", func_001F7C60);

ASM_FUNC("asm/handwritten/text", func_001F7DD8);

float func_001F7E98(float *dst, float *a, float *b, float *c) {
    float t;
    __asm__ __volatile__(
        "vaddw.xyz $vf24,$vf0,$vf0w\n\t"
        "lqc2 $vf1,0(%2)\n\t"
        "lqc2 $vf2,0(%3)\n\t"
        "lqc2 $vf3,0(%4)\n\t"
        "vsub.xyz $vf4,$vf2,$vf1\n\t"
        "vsub.xyz $vf5,$vf3,$vf1\n\t"
        "vmul.xyz $vf6,$vf4,$vf4\n\t"
        "vmul.xyz $vf7,$vf4,$vf5\n\t"
        "vadday.x ACC,$vf6,$vf6y\n\t"
        "vmaddz.x $vf6,$vf24,$vf6z\n\t"
        "vadday.x ACC,$vf7,$vf7y\n\t"
        "vmaddz.x $vf7,$vf24,$vf7z\n\t"
        "vdiv Q,$vf7x,$vf6x\n\t"
        "vwaitq\n\t"
        "vaddq.x $vf8,$vf0,Q\n\t"
        "vminiw.x $vf7,$vf8,$vf0w\n\t"
        "qmfc2.ni $8,$vf8\n\t"
        "nop\n\t"
        "vmaxx.x $vf7,$vf7,$vf0x\n\t"
        "mtc1 $8,%0\n\t"
        "vmulax.xyz ACC,$vf4,$vf7x\n\t"
        "vmaddw.xyz $vf1,$vf1,$vf0w\n\t"
        "sqc2 $vf1,0(%1)"
        : "=f"(t) : "r"(dst), "r"(a), "r"(b), "r"(c) : "$8", "memory");
    return t;
}

ASM_FUNC("asm/handwritten/text", func_001F7EF8);

ASM_FUNC("asm/handwritten/text", func_001F84AC);

ASM_FUNC("asm/handwritten/text", func_001F852C);

ASM_FUNC("asm/handwritten/text", func_001F856C);

ASM_FUNC("asm/handwritten/text", func_001F8B6C);

ASM_FUNC("asm/handwritten/text", func_001F91B8);

typedef struct {
    Vec4f position;
    s16 active_count;
    s16 alpha;
    u8 pad14[4];
    f32 angle;
    f32 radius_scale;
} BillboardRecord;
typedef struct {
    u8 pad0[0x1A8];
    f32 depth_offset;
    u8 pad1AC[0x64];
    f32 projection_scale;
} BillboardViewContext;
struct DmaTag {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
struct TagPtr {
    struct DmaTag *p;
};
extern struct TagPtr D_00161000 MACRO_ADDR;
extern char D_001609E0[];
extern u8 D_00187180[];
extern BillboardViewContext D_0018CE00;
extern BillboardRecord D_0018EE00[];
extern void func_001F99B0(void *, s32, s32);
extern s64 func_001F4868_F9478(s32) __asm__("func_001F4868");
extern s32 func_001F9B20(Vec4f *);
extern void func_001F9BF0(Vec4f *, void *, void *);
extern void func_001F9C30(Vec4f *, Vec4f *, f32);
extern void func_001F9C60(Vec4f *, Vec4f *, void *);
extern f32 func_001F9CB8(Vec4f *);
extern void func_001F9EE8(Vec4f *, Vec4f *, void *);
extern f32 func_001F9F90(f32);
extern f32 func_001F9FA8(f32);
extern f32 func_001FA888(s32);
extern s32 func_001FA898(f32);
extern void func_00234C98(s32, u64);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/textbin/append_billboard_batch.c, append_billboard_batch. */
void append_billboard_batch(void) {
    Vec4f projected_position;
    Vec4f clip_position;
    f32 distance;
    f32 radius;
    s32 record_index;
    BillboardRecord *record;
    s64 color;
    s64 x;
    s32 y;
    s64 packed_position;
    s32 sine_offset;
    s32 cosine_offset;
    struct DmaTag *tag;
    s64 *packet_words;
    BillboardRecord *records;

    VU1_addGSregister(0x42, 0x8000000048);
    for (record_index = 0; record_index < 16; record_index++) {
        records = billboard_records;
        record = records + record_index;
        if (record->active_count <= 0) {
            continue;
        }
        FastVecSub(&projected_position, record, camera_position);
        projected_position.w = 1.0f;
        distance = FastVecLength(&projected_position);
        FastVecScale(&projected_position, &projected_position, 1024.0f);
        func_001F9EE8(&projected_position, &projected_position, camera_position - 0x100);
        multiply_vector_components(&clip_position, &projected_position,
                                   (char *)&view_context + 0x180);
        if (is_vector_outside_clip(&clip_position) != 0) {
            FastMemSet(record, 0, 0x20);
            continue;
        }
        FastVecScale(&projected_position, &projected_position,
                       view_context.projection_scale / projected_position.w);
        color = (record->alpha << 24) | 0x808080;
        x = truncate_float_to_s32(projected_position.x * 16.0f) + 0x8000;
        y = truncate_float_to_s32(projected_position.y * 16.0f) + 0x8000;
        packed_position = ((s64)truncate_float_to_s32(projected_position.z * 0.9997f +
                                                         view_context.depth_offset)
                           << 32) |
                          ((s64)y << 16) | x;
        if (distance > 18.0f) {
            distance = 18.0f;
        } else if (distance < 2.0f) {
            distance = 2.0f;
        }
        radius = record->radius_scale * (IntToFloat(record->alpha + 16) * 0.015625f) *
                 ((24.0f - distance) * 16.0f);
        sine_offset = truncate_float_to_s32(radius * FastSin(record->angle));
        cosine_offset = truncate_float_to_s32(radius * FastCos(record->angle));
        D_00161000.p->tag = 0x10000009;
        D_00161000.p->addr = 0;
        D_00161000.p->vif0 = 0;
        D_00161000.p->vif1 = 0x50000009;
        tag = D_00161000.p;
        D_00161000.p = tag + 1;
        qcopy(tag + 1, billboard_quad_header);
        packet_words = (s64 *)(tag + 2);
        D_00161000.p = tag + 2;
        packet_words[0] = 5;
        packet_words[1] = func_001F4868_F9478(0x13);
        packet_words[2] = 0x154;
        packet_words[3] = color;
        packet_words[4] = 0;
        packet_words[5] = packed_position + (cosine_offset << 16) + sine_offset;
        packet_words[6] = color;
        packet_words[7] = 0x200;
        packet_words[8] = packed_position + (-sine_offset << 16) + cosine_offset;
        packet_words[9] = color;
        packet_words[10] = 0x2000000;
        packet_words[11] = packed_position + (sine_offset << 16) - cosine_offset;
        packet_words[12] = color;
        packet_words[13] = 0x2000200;
        packet_words[14] = packed_position + (-cosine_offset << 16) - sine_offset;
        packet_words[15] = 0;
        D_00161000.p = (struct DmaTag *)((u8 *)D_00161000.p + 0x80);
    }
    VU1_addGSregister(0x42, 0x8000000044);
}
