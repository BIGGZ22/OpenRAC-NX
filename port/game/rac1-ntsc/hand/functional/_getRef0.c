/* Carried from the PAL decompilation's nonmatching/functional/func_00125F88.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
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
struct sceMpeg_cgetref0 {
    s32 width;
    s32 height;
    s32 frameCount;
    u8 pad_C[0x4];
    s64 pts;
    s64 dts;
    u64 flags;
    s64 pts2nd;
    s64 dts2nd;
    u64 flags2nd;
    struct MpegDecoder_cgetref0 *sys; /* private decoder state */
};
struct MpegRefImage_cgetref0 {
    u8 *data; /* picture base; _getRef0 adds macroblock index * 0x180 */
    s32 width;
    s32 height;
    s32 unkC;
    s32 unk10; /* non-zero: sized check in _isOutSizeOK;
                                           _getRef0 macroblock index x * unk10 + y */
    u8 pad_14[0x14];
    s32 status; /* 1: output by _dispRefImage; ClearMpegReferenceBuffer zeroes it */
    u8 pad_2C[0x18];
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s32 unk60;
};
typedef void (*MpegMcFunc_cgetref0)(void);
struct MpegMcFetch_cgetref0 {
    u8 *addr;
    s32 x;
    s32 n0;
    s32 n1;
    s32 step;
    u8 *p0;
    u8 *p1;
};
struct MpegMbBuffer_cgetref0 {
    u32 spr_base; /* SPR 0x70000000 or 0x70001B00; fetch n at + n * 0x600 */
    u32 ipu_out; /* spr_base + 0x1800; 0x300 bytes received from the IPU (_decMB0) */
    u8 *luma_ref[4]; /* reference picture luma source per fetch */
    u8 *chroma_ref[4]; /* reference picture chroma source per fetch */
    MpegMcFunc_cgetref0 luma_func[4]; /* D_00132D30[] copy routine per fetch */
    MpegMcFunc_cgetref0 chroma_func[4]; /* D_00132D50[] copy routine per fetch */
    struct MpegMcFetch_cgetref0 luma[4];
    struct MpegMcFetch_cgetref0 chroma[4];
    s32 unk128; /* read by _doMC */
    s32 fetch_count; /* queued fetches; loop count in _doMC */
    s32 unk130; /* read by _doMC */
    u8 pad_134[0x4];
    s32 unk138; /* _doMC skips the work when 0 */
    s32 unk13C; /* set 1 by _decMB0/_skipMB0, 0 by _slice0 */
};
struct MpegDecoder_cgetref0 {
    s32 unk0;
    s32 unk4; /* non-zero: _outputFrame displays a frame */
    s32 unk8;
    u8 pad_C[0x40];
    u8 ipu_dma_state; /* &ipu_dma_state passed to suspend_mpeg_decoder_ipu_dma */
    u8 pad_4D[0x33];
    s32 unk80;
    u8 pad_84[0x4];
    u64 unk88;
    u8 pad_90[0x4];
    s32 unk94;
    s32 unk98;
    s32 unk9C;
    s32 unkA0;
    s32 unkA4;
    s32 unkA8;
    s32 frame_base; /* sceMpeg.frameCount = frame_count - frame_base (_sceMpegFlush) */
    s32 unkB0; /* non-zero: _dispRefImage uses _csc_storeRefImage, else _cpr8 */
    s32 unkB4;
    s32 unkB8;
    s32 unkBC;
    s32 unkC0;
    s32 unkC4;
    s32 unkC8;
    s32 unkCC;
    s32 unkD0;
    s32 unkD4; /* 0 in _sequenceHeader, set by _pictureCodingExtension */
    s32 unkD8; /* output address (_cpr8 masks it to physical) */

    /* Output size limits, checked by _isOutSizeOK against the picture. */
    s32 out_max_width; /* >= picture width when out_max_height != 0 */
    s32 out_max_height; /* >= picture height; 0: check out_max_size */
    s32 out_max_size; /* >= unkC * unk10 of the picture */

    u8 pad_E8[0x10];
    s32 unkF8; /* 1 -> 2 in _outputFrame */
    s32 frame_buffers[3]; /* reserved in _sequenceHeader, passed to _initRefImages */
    u8 pad_108[0x10];
    s32 frame_count; /* +1 per decoded frame (_decodeOrSkipFrame) */
    s32 unk11C;
    s32 second_field; /* toggled per field picture (_decodeOrSkipFrame); set at
                                           _lastFrame: "the second field is missing" */

    /* sequence_header (_sequenceHeader) */
    u32 horizontal_size; /* 12 bits */
    s32 vertical_size; /* 12 bits; > 2800 is an error */
    s32 mb_width; /* (horizontal_size + 15) >> 4 */
    s32 mb_height;
    u32 bit_rate; /* 18 bits */
    s32 vbv_buffer_size; /* 10 bits */
    s32 unk13C; /* 0: mb_height rounded to 32-line pairs (_sequenceHeader) */
    s32 unk140;

    /* sequence_display_extension */
    s32 matrix_coefficients; /* 3rd 8-bit colour_description field */
    s32 display_horizontal_size; /* 14 bits */
    s32 display_vertical_size; /* 14 bits */

    s32 picture_coding_type; /* 1 I, 2 P (_skipMB0 zeroes motion), 3 B (_outputFrame) */
    s32 unk154;
    s32 unk158;
    s32 unk15C;
    s32 unk160;

    /* picture_coding_extension */
    s32 f_code_forward_h;
    s32 f_code_forward_v;
    s32 f_code_backward_h;
    s32 f_code_backward_v;
    s32 picture_structure; /* 3: frame picture */
    s32 top_field_first;
    s32 frame_pred_frame_dct;
    s32 concealment_motion_vectors;
    s32 repeat_first_field;
    s32 progressive_frame;

    u8 pad_18C[0x20];
    s32 unk1AC;
    s32 dc_reset; /* DCR bit of the IPU BDEC command (_decMB0); set by _skipMB0 */
    s32 quantiser_scale_code; /* 5 bits from the slice header (_sliceB); QSC of BDEC */
    struct MpegRefImage_cgetref0 *ref_images[12]; /* chosen by picture_structure / picture_coding_type in _outputFrame */
    u8 pad_1E8[0x3A8];
    struct MpegMbBuffer_cgetref0 mb_buf[2];
    s32 mb_buf_index; /* toggled per slice in _slice0 */
    u8 pad_814[0x8];
    u8 *mc_work; /* _getRef0: destination of the queued fetches */
    s32 unk820;
    u8 pad_824[0x1C];

    /* quant_matrix_extension / sequence_header */
    s32 load_intra_quantiser_matrix; /* set -> IPU SETIQ intra (0x50000000) */
    s32 load_non_intra_quantiser_matrix;/* set -> IPU SETIQ non-intra (0x58000000) */
    s32 unk848; /* 0: _sequenceHeader loads frame-picture defaults */
    s32 unk84C;
    s32 unk850;
    s32 unk854;
    struct sceMpeg_cgetref0 *mpeg; /* owning sceMpeg (_dispRefImage writes its pts/dts/flags) */
};
/* functional: medium. libmpeg motion-compensation fetch setup (one 16x(h) luma and one chroma fetch descriptor for the current macroblock): splits the block by the 16-pixel (8 chroma) cache lines, fills the two fetch records at d+0x590+idx*0x140 (+0x48/+0xB8 + n*0x1C), the handler pointers (D_00132E30 / D_00132E50 by half-pel code) and the three reference addresses, then bumps the fetch count at d+0x6BC+idx*0x140. Written from the assembly; not run in the capture (hand-checked). */
extern int D_00132F30_25F88[] __asm__("D_00132E30");
extern int D_00132F50_25F88[] __asm__("D_00132E50");



void _getRef0(struct MpegDecoder_cgetref0 *d, struct MpegRefImage_cgetref0 *ref, int sfield, int dfield, int yofs, int h,
                   int bx, int by, int dx, int dy, int fieldpred, int avg) {
    char *dd = (char *)d;
    int idx = *(int *)(dd + 0x810);
    int off = idx * 0x140;
    int mc_work = *(int *)(dd + 0x81C);
    int n = *(int *)(dd + 0x6BC + off);
    char *E = dd + 0x590 + off;
    int *fa = (int *)(E + n * 0x1C + 0x48);
    int *fb = (int *)(E + n * 0x1C + 0xB8);
    int t, r6, r8, r20, r21, S, low, bit8, bit9, code, p, base, v;
    int a8s, a9s, c6, c8, c10, c7, lowc, bitc8, bitc9, codeC, h2, r4c;
    int ent0;

    t = dy >> 1;
    if (fieldpred != 0) {
        t = t << 1;
    }
    r6 = t + by + (yofs + sfield);
    r8 = (dx >> 1) + bx;
    r20 = r8 >> 4;
    r21 = r6 >> 4;
    S = ref->unk10 * r20 + r21;
    fa[1] = r8 - (r20 << 4);
    fa[0] = mc_work + ((dfield + yofs) << 5);
    low = r6 - (r21 << 4);
    bit9 = dy & 1;
    bit8 = dx & 1;
    if (bit9 != 0) {
        if (low + (h << fieldpred) < 0x10) {
            fa[2] = h;
            fa[3] = 0;
        } else {
            v = ((0x10 >> fieldpred) - (low >> fieldpred)) - 1;
            fa[2] = v;
            fa[3] = h - v;
        }
    } else {
        if (low + (h << fieldpred) < 0x11) {
            fa[2] = h;
            fa[3] = 0;
        } else {
            v = (0x10 >> fieldpred) - (low >> fieldpred);
            fa[2] = v;
            fa[3] = h - v;
        }
    }
    ent0 = *(int *)(dd + 0x590 + off);
    base = ent0 + n * 0x600;
    a8s = dx + (int)((unsigned int)dx >> 31);
    a9s = dy + (int)((unsigned int)dy >> 31);
    code = (avg << 2) | (bit8 << 1) | bit9;
    fa[5] = base + (low << 4);
    fa[4] = 0x10 << fieldpred;
    fa[6] = base + (low << 4) + 0x300;

    /* chroma */
    h2 = h >> 1;
    c8 = (a8s >> 2) + (bx >> 1);
    bitc8 = (a8s >> 1) & 1;
    c7 = a9s >> 1;
    c6 = ((a9s >> 2) << (fieldpred != 0 ? 1 : 0)) + (by >> 1) + ((yofs >> 1) + sfield);
    c10 = c6 >> 3;
    p = c8 >> 3;
    fb[0] = mc_work + (((dfield + (yofs >> 1)) << 4) + 0x200);
    lowc = c6 - (c10 << 3);
    fb[1] = c8 - (p << 3);
    bitc9 = c7 & 1;
    if (bitc9 != 0) {
        if (lowc + (h2 << fieldpred) < 8) {
            fb[2] = h2;
            fb[3] = 0;
        } else {
            v = ((8 >> fieldpred) - (lowc >> fieldpred)) - 1;
            fb[2] = v;
            fb[3] = h2 - v;
        }
    } else {
        if (lowc + (h2 << fieldpred) < 9) {
            fb[2] = h2;
            fb[3] = 0;
        } else {
            v = (8 >> fieldpred) - (lowc >> fieldpred);
            fb[2] = v;
            fb[3] = h2 - v;
        }
    }
    fb[4] = 8 << fieldpred;
    r4c = ((p - r20) << 1) + (c10 - r21);
    codeC = (avg << 2) | (bitc8 << 1) | bitc9;
    *(int *)(dd + n * 4 + off + 0x5B8) = D_00132F30_25F88[code];
    base = r4c * 0x180 + base;
    fb[5] = base + (lowc << 3) + 0x100;
    fb[6] = base + (lowc << 3) + 0x400;
    *(int *)(dd + n * 4 + off + 0x598) = (int)ref->data + S * 0x180;
    *(int *)(dd + n * 4 + off + 0x5C8) = D_00132F50_25F88[codeC];
    *(int *)(dd + n * 4 + off + 0x5A8) = (int)ref->data + (S + ref->unk10) * 0x180;
    *(int *)(dd + 0x6BC + off) = *(int *)(dd + 0x6BC + off) + 1;
}
