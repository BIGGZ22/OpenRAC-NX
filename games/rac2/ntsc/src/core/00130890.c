#include "common.h"
#include "structs.h"

extern void r1_func_00127378(int arg0);

extern void func_00130288(void *, void *);

extern int r1_func_00128A58(void *, int);

extern char r1_D_00153C00[];

extern char r1_D_00153C28[];

extern void r1_func_00113968(void);

extern void r1_func_00114438(void *, void *);

/* rac1-decomp src/core/0012AC80.c: func_0012CA70 */
/*
 * _sequenceExtension (libmpeg.a:init.o): ISO/IEC 13818-2 6.2.2.3
 * sequence_extension(), matching mpeg2decode's gethdr.c almost line for
 * line but reading two combined bitfields with r1_func_00128A58: a 28-bit
 * fetch covers profile_and_level_indication(8) + progressive_sequence(1)
 * + chroma_format(2) + horizontal_size_extension(2) +
 * vertical_size_extension(2) + bit_rate_extension(12) + marker_bit(1); a
 * 16-bit fetch covers vbv_buffer_size_extension(8) + low_delay(1) +
 * frame_rate_extension_n(2) + frame_rate_extension_d(5) (the last three
 * read and dropped). chroma_format must be 1 (4:2:0) and
 * profile_and_level_indication one of three literal bytes (0x48/0x58/
 * 0x44); either failing reports through func_00130288. Ends by folding
 * the extension bits into sequenceHeader's fields exactly as the
 * reference decoder's
 * `horizontal_size = (horizontal_size_extension<<12) | (horizontal_size&0xfff)`
 * et al.
 */
void func_00130890(void *arg0) {
    char *s = (char *)arg0;
    unsigned int v1, v2;
    unsigned int chroma_format, h_ext, v_ext, bitrate_ext;
    unsigned int progressive, profile, vbv_ext;

    *(int *)(s + 0x848) = 1;
    r1_func_00127378(0);

    v1 = r1_func_00128A58(s, 0x1C);
    bitrate_ext = (v1 >> 1) & 0xFFF;
    chroma_format = (v1 >> 17) & 3;
    v_ext = (v1 >> 13) & 3;
    h_ext = (v1 >> 15) & 3;

    *(unsigned int *)(s + 0x140) = chroma_format;
    if (chroma_format != 1) {
        func_00130288(s, r1_D_00153C00);
    }

    progressive = (v1 >> 19) & 1;
    *(unsigned int *)(s + 0x13C) = progressive;
    profile = v1 >> 20;

    v2 = r1_func_00128A58(s, 0x10);
    vbv_ext = v2 >> 8;

    if (profile != 0x48 && profile != 0x58 && profile != 0x44) {
        func_00130288(s, r1_D_00153C28);
    }

    *(int *)(s + 0x124) = (h_ext << 12) | (*(int *)(s + 0x124) & 0xFFF);
    *(int *)(s + 0x128) = (v_ext << 12) | (*(int *)(s + 0x128) & 0xFFF);
    *(int *)(s + 0x134) += bitrate_ext << 18;
    *(int *)(s + 0x138) += vbv_ext << 10;
}

/* rac1-decomp src/core/0012AC80.c: func_0012CBA0 */
/*
 * MPEG-2 sequence_display_extension(): video_format (3 bits), then
 * colour_description (1) and, when set, colour_primaries,
 * transfer_characteristics and matrix_coefficients (8 each; the last is
 * kept at +0x144); display_horizontal_size (14) to +0x148, a marker
 * bit, display_vertical_size (14) to +0x14C.
 *
 * Plain C, exact under 2.9-ee. (Under 2.95.3 it needed the marker bit's
 * read to go through a void view of the reader, `r1_func_00128A58_v`: as a
 * value call it reset $v0's readers and the +0x148 store lost the
 * scheduler tie to the next call's argument setup, 8/140.)
 */
void func_001309C0(void *arg0) {
    char *s = (char *)arg0;

    r1_func_00128A58(s, 3);
    if (r1_func_00128A58(s, 1) != 0) {
        r1_func_00128A58(s, 8);
        r1_func_00128A58(s, 8);
        *(int *)(s + 0x144) = r1_func_00128A58(s, 8);
    }
    *(int *)(s + 0x148) = r1_func_00128A58(s, 0xE);
    r1_func_00128A58(s, 1);
    *(int *)(s + 0x14C) = r1_func_00128A58(s, 0xE);
}

/* rac1-decomp src/core/00113A70.c: func_00113AC8 */
/* Retail is a bare tail jump: Sony's 2.9-ee, which builds this SDK object,
   emits it for a void function that ends in a call. */
void func_00130A50(void *arg0) {
    r1_func_00114438(arg0, r1_func_00113968);
}
extern void func_00130A60(void *arg0);
extern char D_0013BE88[];

void func_00130A60(void *object) {
    func_00130288(object, D_0013BE88);
}

extern void func_00130A70(void *arg0);
extern char D_0013BEA0[];

void func_00130A70(void *object) {
    func_00130288(object, D_0013BEA0);
}

extern void func_00130A80(void *arg0);
extern char D_0013BED8[];

void func_00130A80(void *object) {
    func_00130288(object, D_0013BED8);
}


INCLUDE_ASM("asm/core", func_00130A8C);
