#include "common.h"

/* _dispRefImageField, the field-picture twin of _dispRefImage: adapted from
   Lombyte (GPL-3.0) src/sdk/library/disprefimagefield.c. The unit saves $s0..$s6
   and $ra in 16-byte slots while the stores stay `sd`, the same profile as the
   neighbouring func_0012D808, so it is built with `sdk -O2 -g2 -gstabs`.
   Lombyte's names map to RAC2 as: _getPtsDtsFlags = func_0012DAC0,
   _isOutSizeOK = func_0012D768, _cpr8 = func_0012D808,
   _csc_storeRefImage = func_0012E608, FinishMpegReferenceImage = func_0012DA98. */

struct MpegPictureBuffer {
    u8 pad0[0x10];
    s32 macroblock_height;
    u8 pad14[4];
    s64 presentation_timestamp;
    s64 decoding_timestamp;
    s32 status;
    s32 coding_type;
    s32 structure;
    s32 progressive_sequence;
    s32 progressive_frame;
    s32 top_field_first;
    s32 repeat_first_field;
    s32 horizontal_offset0;
    s32 horizontal_offset1;
    s32 horizontal_offset2;
    s32 vertical_offset0;
    s32 vertical_offset1;
    s32 vertical_offset2;
    s32 display_width;
    s32 display_height;
};

union MpegTimestampSlot {
    s64 value;
    struct {
        s32 low;
        s32 high;
    } words;
};

struct MpegPictureTransfer {
    u8 pad0[0x10];
    union MpegTimestampSlot first_timestamp;
    s64 first_decode_timestamp;
    s64 first_flags;
    union MpegTimestampSlot second_timestamp;
    s64 second_decode_timestamp;
    s64 second_flags;
};

struct MpegFieldTransferState {
    u8 pad0[0x80];
    s32 last_timestamp;
    u8 pad84[4];
    s64 timestamp_step;
    u8 pad90[0x20];
    s32 decoder_initialized;
    s32 horizontal_offset0;
    s32 horizontal_offset1;
    u8 padbc[4];
    s32 vertical_offset0;
    s32 vertical_offset1;
    u8 padc8[4];
    s32 display_width;
    s32 display_height;
    u8 padd4[0xa0];
    s32 picture_structure;
    u8 pad178[0x6e0];
    struct MpegPictureTransfer *transfer;
};

extern void func_0012DAC0(struct MpegFieldTransferState *, struct MpegPictureBuffer *, s64 *,
                          s64 *, s64 *);
extern s32 func_0012D768(struct MpegFieldTransferState *, struct MpegPictureBuffer *);
extern void func_0012D808(struct MpegFieldTransferState *, struct MpegPictureBuffer *);
extern void func_0012E608(struct MpegFieldTransferState *, struct MpegPictureBuffer *);
extern void func_0012DA98(struct MpegFieldTransferState *);

void func_0012DD60(struct MpegFieldTransferState *state, struct MpegPictureBuffer *first,
                   struct MpegPictureBuffer *second) {
    s64 first_flags, second_flags;
    s32 timestamp;
    s32 second_timestamp;
    s32 display_height;
    struct MpegPictureBuffer *primary;
    struct MpegPictureBuffer *secondary;
    struct MpegPictureTransfer *transfer;
    struct MpegPictureTransfer *second_transfer;
    struct MpegPictureTransfer *final_transfer;
    s32 flags;
    flags = 0;
    if (state->picture_structure == 2) {
        primary = first;
        secondary = second;
        flags = 0x40;
    } else {
        primary = second;
        secondary = first;
    }
    transfer = state->transfer;
    func_0012DAC0(state, primary, &transfer->first_timestamp.value,
                  &transfer->first_decode_timestamp, &transfer->first_flags);
    second_transfer = state->transfer;
    timestamp = second_transfer->first_timestamp.words.low;
    state->timestamp_step = 1;
    state->last_timestamp = timestamp;
    func_0012DAC0(state, secondary, &second_transfer->second_timestamp.value,
                  &second_transfer->second_decode_timestamp, &second_transfer->second_flags);
    final_transfer = state->transfer;
    second_timestamp = final_transfer->second_timestamp.words.low;
    state->timestamp_step = 1;
    state->last_timestamp = second_timestamp;
    first_flags = final_transfer->first_flags | flags;
    state->display_width = primary->display_width;
    second_flags = final_transfer->second_flags;
    final_transfer->first_flags = first_flags;
    second_flags |= flags;
    display_height = primary->display_height;
    final_transfer->second_flags = second_flags;
    state->display_height = display_height;
    state->horizontal_offset0 = primary->horizontal_offset0;
    state->horizontal_offset1 = secondary->horizontal_offset1;
    state->vertical_offset0 = primary->vertical_offset0;
    state->vertical_offset1 = secondary->vertical_offset1;
    if (func_0012D768(state, first) && first->status == 1 && second->status == 1) {
        first->macroblock_height *= 2;
        if (state->decoder_initialized)
            func_0012E608(state, first);
        else
            func_0012D808(state, first);
        first->macroblock_height >>= 1;
        func_0012DA98(state);
    }
}
