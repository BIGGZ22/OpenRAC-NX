/* Carried from the PAL decompilation's nonmatching/game/func_001FAB40.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * functional: high. Read against the US assembly on 2026-10-18: buffer fields, globals, both env calls and all four packets agree; no change. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* SetupFS_AA_buffer: fills the full-screen anti-aliasing buffer record (display and storage sizes,
 * pixel formats, frame, storage and depth buffer pages), sets its display environment and its two
 * drawing environments, and builds the GS packets that copy, resample, draw and clear the frame in
 * sixteen strips.
 * Functional C for the native port, written from Lombyte's non-matching draft for the US version
 * (MIT: src/assembly/textbin/rendering/buffers/setup_fs_aa_buffer.c, setup_fs_aa_buffer) and the
 * PAL code, whose globals all sit 0x100 above the US ones. It does what the retail function does;
 * it does not match its bytes. */
typedef struct {
    unsigned long disp[5];      /* 0x000: the display environment (its framebuffer word at 0x10) */
    unsigned long pad028;
    unsigned long giftag0[2];   /* 0x030 */
    unsigned long draw0[16];    /* 0x040: a drawing environment (frame at +0, z buffer at +0x10) */
    unsigned long giftag1[2];   /* 0x0C0 */
    unsigned long draw1[16];    /* 0x0D0 */
    short display_width;        /* 0x150 */
    short display_height;
    short psm;
    short fbp0;
    short storage_width;        /* 0x158 */
    short storage_height;
    short storage_psm;
    short fbp1;
    short target_width;         /* 0x160 */
    short target_height;
    short target_psm;
    short target_fbp;
    short display_offset_x;     /* 0x168 */
    short display_offset_y;
    short zpsm;
    short zbp;
    int reserved170;            /* 0x170 */
} FsAa_FAB40_c1fa978;
extern FsAa_FAB40_c1fa978 buffer_FAB40 __asm__("D_00151780");
extern FsAa_FAB40_c1fa978 *active_FAB40 __asm__("D_0015EEB8");
extern int display_address_FAB40 __asm__("D_0015EE80");
extern int draw_address_FAB40 __asm__("D_0015EE84");
extern int depth_address_FAB40 __asm__("D_0015EE88");
extern unsigned long first_clear_FAB40[40] __asm__("D_0013CC90");
extern unsigned long second_clear_FAB40[40] __asm__("D_0013CDD0");
extern unsigned long draw_packet_FAB40[76] __asm__("D_00151900");
extern unsigned long transfer_packet_FAB40[82] __asm__("D_00151B60");
extern unsigned long resample_packet_FAB40[74] __asm__("D_00151DF0");
extern unsigned long clear_packet_FAB40[42] __asm__("D_00152040");
extern void set_def_disp_env_FAB40(void *, short, short, short, short, short) __asm__("func_00121AC8");
extern int set_def_draw_env_FAB40(void *, short, short, short, short, short) __asm__("func_00121FC8");
/* A GIF tag of eight loops, one register (A+D), end of packet. */
static __inline__ void gif_tag_FAB40_c1fa978(unsigned long *tag) {
    tag[0] = 8 | (1UL << 15) | (1UL << 60);
    tag[1] = 0xE;
}
/* The FRAME register word the copy and draw packets start with. */
static __inline__ unsigned long frame_word_FAB40_c1fa978(FsAa_FAB40_c1fa978 *b) {
    return ((unsigned long)b->fbp0 << 5) | ((unsigned long)((b->display_width >> 6) & 0x3F) << 14) |
           ((unsigned long)b->psm << 20) | 0xEA8000000UL;
}
/* Sixteen strips of two corners each over a height: the clear packets' body. */
static __inline__ void clear_strips_FAB40_c1fa978(unsigned long *p, int height) {
    int i, left = 0x6FF8, next = 0x71F8;
    for (i = 0; i < 16; i++) {
        *p++ = left | ((unsigned long)(0x7FF8 - (height << 3)) << 16);
        left += 0x200;
        *p++ = next | ((unsigned long)((height << 3) + 0x7FF8) << 16);
        next += 0x200;
    }
}



void FUN_001fa978(int display_width, int display_height, int storage_width, int storage_height,
                   int display_offset_x, int display_offset_y) {
    FsAa_FAB40_c1fa978 *b;
    unsigned long *p;
    int i, left, next, right;

    active_FAB40 = &buffer_FAB40;
    buffer_FAB40.display_width = display_width;
    buffer_FAB40.display_height = display_height;
    buffer_FAB40.storage_width = storage_width;
    buffer_FAB40.storage_height = storage_height;
    buffer_FAB40.display_offset_x = display_offset_x;
    buffer_FAB40.display_offset_y = display_offset_y;
    buffer_FAB40.reserved170 = 0;
    buffer_FAB40.storage_psm = 0;
    buffer_FAB40.psm = 0;
    buffer_FAB40.target_psm = 0;
    buffer_FAB40.zpsm = 0x31;
    buffer_FAB40.fbp1 = display_address_FAB40 >> 13;
    buffer_FAB40.fbp0 = draw_address_FAB40 >> 13;
    buffer_FAB40.zbp = depth_address_FAB40 >> 13;
    set_def_disp_env_FAB40(&buffer_FAB40, 0, storage_width, storage_height, display_offset_x,
                           display_offset_y);
    b = active_FAB40;
    b->disp[2] = (b->disp[2] & ~0x1FFUL) | (buffer_FAB40.fbp1 & 0x1FF);
    set_def_draw_env_FAB40(b->draw0, buffer_FAB40.psm, buffer_FAB40.display_width,
                           buffer_FAB40.display_height, 3, buffer_FAB40.zpsm);
    b->draw0[0] = (b->draw0[0] & ~0x1FFUL) | (buffer_FAB40.fbp0 & 0x1FF);
    b->draw0[2] = (unsigned long)buffer_FAB40.zbp | ((unsigned long)(buffer_FAB40.zpsm & 0xF) << 24);
    gif_tag_FAB40_c1fa978(b->giftag0);
    set_def_draw_env_FAB40(b->draw1, buffer_FAB40.storage_psm, buffer_FAB40.storage_width,
                           buffer_FAB40.storage_height, 0, 0);
    b->draw1[2] = 1UL << 32;
    b->draw1[0] = (b->draw1[0] & ~0x1FFUL) | (buffer_FAB40.fbp1 & 0x1FF);
    gif_tag_FAB40_c1fa978(b->giftag1);

    p = transfer_packet_FAB40;
    p[0] = 0x408B400000000001UL;
    p[1] = 0xEEEE;
    p[2] = 0x30000;
    p[3] = 0x47;
    p[4] = 5;
    p[5] = 8;
    p[6] = 0x100000261UL;
    p[7] = 0x14;
    p[8] = frame_word_FAB40_c1fa978(b);
    p[9] = 6;
    p[10] = 0x4400000000008010UL;
    p[11] = 0x5353;
    p = &transfer_packet_FAB40[12];
    for (i = 0; i < 16; i++) {
        *p++ = i * b->display_width;
        *p++ = (i * b->storage_width + (0x8000 - (b->storage_width << 3))) |
               ((unsigned long)(0x7FF8 - (b->storage_height << 3)) << 16);
        *p++ = (i + 1) * b->display_width | ((unsigned long)b->display_height << 20);
        *p++ = ((i + 1) * b->storage_width + (0x8000 - (b->storage_width << 3))) |
               ((unsigned long)((b->storage_height << 3) + 0x7FF8) << 16);
    }
    p[0] = 0x4400000000008001UL;
    transfer_packet_FAB40[77] = 0x4410;
    p[2] = 0x181;
    p[3] = 0x80000000UL;
    transfer_packet_FAB40[80] = 0x6FF8 | ((unsigned long)(0x7FF8 - (b->storage_height << 3)) << 16);
    p[5] = 0x6FF8 | ((unsigned long)((b->storage_height << 3) + 0x7FF8) << 16);

    p = resample_packet_FAB40;
    p[0] = 0x308B400000000001UL;
    p[1] = 0xEEE;
    p[2] = 0x30000;
    p[3] = 0x47;
    p[4] = 0x100000261UL;
    p[5] = 0x14;
    p[6] = frame_word_FAB40_c1fa978(b);
    p[7] = 6;
    p[8] = 0x4400000000008010UL;
    p[9] = 0x5353;
    p = &resample_packet_FAB40[10];
    left = 0x6FF8;
    next = 0x71F8;
    for (i = 0; i < 16; i++) {
        *p++ = i * b->display_width;
        *p++ = left | ((unsigned long)(0x7FF8 - (b->storage_height << 3)) << 16);
        left += 0x200;
        *p++ = (i + 1) * b->display_width | 0x1A000000UL;
        *p++ = next | ((unsigned long)((b->storage_height << 3) + 0x7FF8) << 16);
        next += 0x200;
    }

    p = draw_packet_FAB40;
    p[0] = 0x408B400000000001UL;
    p[1] = 0xEEEE;
    p[2] = 0x30000;
    p[3] = 0x47;
    p[4] = 5;
    p[5] = 8;
    p[6] = 0x100000261UL;
    p[7] = 0x14;
    p[8] = frame_word_FAB40_c1fa978(b);
    p[9] = 6;
    p[10] = 0x4400000000008010UL;
    p[11] = 0x5353;
    p = &draw_packet_FAB40[12];
    left = 0x7000;
    next = 0x200;
    right = 0x7200;
    for (i = 0; i < 16; i++) {
        *p++ = i * 0x200;
        *p++ = left | ((unsigned long)(0x8000 - (b->display_height << 3)) << 16);
        left += 0x200;
        *p++ = next | ((unsigned long)b->display_height << 20);
        next += 0x200;
        *p++ = right | ((unsigned long)((b->display_height << 3) + 0x7FF0) << 16);
        right += 0x200;
    }

    p = clear_packet_FAB40;
    p[0] = 0x1000000000000001UL;
    p[1] = 0xE;
    p[2] = 0x30000;
    p[3] = 0x47;
    p[4] = 0x2400000000008001UL;
    p[5] = 0x10;
    p[6] = 0x106;
    p[7] = 0x80008000UL;
    p[8] = 0x2400000000008010UL;
    p[9] = 0x44;
    clear_strips_FAB40_c1fa978(&clear_packet_FAB40[10], b->display_height);
    clear_strips_FAB40_c1fa978(&first_clear_FAB40[8], b->display_height);
    clear_strips_FAB40_c1fa978(&second_clear_FAB40[8], b->storage_height);
}
