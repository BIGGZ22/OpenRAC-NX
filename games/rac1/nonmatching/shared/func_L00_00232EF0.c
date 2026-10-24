/* NON_MATCHING func_L00_00232EF0 -- src/overlays/shared/help_00232560.c
 * Best so far: BYTES 4/1312 (99.7% of the bytes match), checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-19: Lombyte port (FUN_L00_00232670, matched there). Hero animation frame advance: steps the hero moby'
 *   anim_time, walks frames, fires the sequence's sound triggers, keeps the looping voice slot in step. The +inf
 *   Lombyte writes as 1.0f/0.0f is the level literal D_L00_0015F7E8 here, declared extern const so GCC hoists the
 *   load like retail. BYTES 4/1312: the two hoisted loop invariants swap FP registers (retail: 0.0f in $f22, +inf 
 *   $f23; ours the reverse). Statement order (p2), a const array (p4), an early local for the zero (p7, p8), the z
 *   from the assignment's value (p10) and a local load before the loop (p6, size) do not move it. A pointer local 
 *   the +inf (p11) changes the frame. What would unblock it: a way to number the zero's pseudo before the +inf's
 *   without changing the two stores' order.
 */
#include "moby.h"
typedef struct {
    u8 pad0[0x10];
    u8 frame_count;     /* 0x10 */
    u8 unk11;
    u8 trigger_count;   /* 0x12 */
    u8 pad13[9];
    void *frames[1];    /* 0x1C; the sound triggers (sound | frame << 16) follow the frames */
} AnimSeq_32EF0;
typedef struct {
    u8 pad0[0x7E];
    s16 sound;          /* 0x7E */
    u8 pad80[8];
    Moby *owner;        /* 0x88 */
} VoiceSlot_32EF0;
extern f32 D_L00_0017BEF0[][25];
extern const f32 D_L00_0015F7E8;
extern int D_L00_0015F6A8 MACRO_ADDR;
extern int func_0022ED80(int, int, Moby *);
extern void func_L00_0028EBF0(int);
extern float func_0020D830(Moby *);

/* Hero animation frame advance: steps the hero moby's frame position (anim_time) by anim_speed * unkA94 or a
   scripted rate table (D_L00_0017BEF0[unkAA0]), moves to the next frame of the sequence each time it passes 1.0
   (wrapping, or jumping as unkAB0/unkAB4/unkAB8 request), fires the sequence's sound triggers crossed this frame,
   keeps the moby's looping sound (sound_slot) in step with unk7C, and records the distance moved (anim_frame, unkAAC).
   Adapted from Lombyte (GPL-3.0) for PAL: src/overlays/shared/ui/help/00232670.c, FUN_L00_00232670. */
void func_L00_00232EF0(void) {
    Moby *m = ((Hero *)D_0013F450)->moby;
    s32 same;
    u8 frame0;
    f32 t0;
    f32 dt;
    f32 prev;

    Hero_ClearSpeedTargets();
    ((Hero *)D_0013F450)->unkA98 &= ~3;
    same = m->seq == m->prev_seq;
    frame0 = m->frame;
    t0 = m->anim_time;
    if (m->seq == m->prev_seq) {
        m->anim_time = t0 + ((Hero *)D_0013F450)->anim_speed * ((Hero *)D_0013F450)->unkA94;
    } else if (((Hero *)D_0013F450)->unkAA0 >= 0) {
        m->anim_time = D_L00_0017BEF0[((Hero *)D_0013F450)->unkAA0][((Hero *)D_0013F450)->unkAA4++];
    } else {
        m->anim_time = t0 + ((Hero *)D_0013F450)->unkA94;
    }
    if (0.99f < m->anim_time && m->anim_time < 1.01f)
        m->anim_time = 1.0f;
    if (-0.01f < m->anim_time && m->anim_time < 0.01f)
        m->anim_time = 0.0f;
    dt = m->anim_time - t0;
    while (1.0f <= m->anim_time) {
        if (m->seq != m->prev_seq) {
            m->seq = m->prev_seq;
            m->anim_trigger_count = ((AnimSeq_32EF0 *)m->pclass->seqs[m->seq])->trigger_count;
        }
        ((Hero *)D_0013F450)->unkAA0 = -1;
        ((Hero *)D_0013F450)->unkA98 |= 1;
        m->frame = m->prev_frame;
        m->cur_frame_data = m->prev_frame_data;
        m->prev_frame++;
        if (((Hero *)D_0013F450)->unkAB8 != 0) {
            ((Hero *)D_0013F450)->unkAB8 = 0;
            ((Hero *)D_0013F450)->unkA94 = D_L00_0015F7E8; /* +inf, the level's literal pool */
            /* the just-cleared int, converted: retail holds this 0.0f in a register across the loop,
               which a literal 0.0f (stored from $zero, as in the next branch) does not give */
            m->anim_time = ((Hero *)D_0013F450)->unkAB8;
            m->prev_frame = ((Hero *)D_0013F450)->unkAB4;
            m->prev_frame_data = ((AnimSeq_32EF0 *)m->pclass->seqs[m->prev_seq])->frames[m->prev_frame];
            HeroClearQueuedSequence();
            HeroSyncSecondaryAnim();
        } else if (((Hero *)D_0013F450)->unkAB0 != -1 && ((Hero *)D_0013F450)->unkAB4 < m->prev_frame) {
            ((Hero *)D_0013F450)->unkA94 = 0.33333334f;
            m->anim_time = 0.0f;
            m->prev_frame = ((Hero *)D_0013F450)->unkAB0;
            m->prev_frame_data = ((AnimSeq_32EF0 *)m->pclass->seqs[m->prev_seq])->frames[m->prev_frame];
            HeroSyncSecondaryAnim();
        } else {
            if (m->prev_frame >= ((AnimSeq_32EF0 *)m->pclass->seqs[m->prev_seq])->frame_count) {
                m->prev_frame = 0;
                ((Hero *)D_0013F450)->unkA98 |= 2;
            }
            m->prev_frame_data = ((AnimSeq_32EF0 *)m->pclass->seqs[m->prev_seq])->frames[m->prev_frame];
            m->anim_time -= 1.0f;
            m->anim_time /= ((Hero *)D_0013F450)->unkA94;
            ((Hero *)D_0013F450)->unkA94 = *(f32 *)m->cur_frame_data;
            m->anim_time *= ((Hero *)D_0013F450)->unkA94;
        }
    }
    if (same && m->anim_trigger_count != 0) {
        s32 hi = (m->frame << 4) + truncate_float_to_s32(m->anim_time * 16.0f);
        s32 lo = (frame0 << 4) + truncate_float_to_s32(t0 * 16.0f);
        AnimSeq_32EF0 *seq = ((AnimSeq_32EF0 *)m->pclass->seqs[m->seq]);
        s32 *trig = (s32 *)&seq->frames[seq->frame_count];
        s32 i;
        for (i = 0; i < seq->trigger_count; i++, trig++) {
            s32 at = *trig >> 16;
            if (lo < at && !(hi < at)) {
                func_0022ED80(*trig & 0xFFFF, 0, m);
                break;
            }
        }
    }
    if (m->sound_slot != 0xFF) {
        VoiceSlot_32EF0 *v = (VoiceSlot_32EF0 *)(D_0013E633 + 0x1D + m->sound_slot * 0x70);
        if (v->owner != m) {
            m->sound_slot = 0xFF;
        } else if (v->sound != m->unk7C) {
            func_L00_0028EBF0(m->sound_slot);
            m->sound_slot = 0xFF;
        }
    } else if (m->unk7C != 0xFF && D_L00_0015F6A8 != 2 && D_L00_0015F6A8 != 6) {
        m->sound_slot = func_0022ED80(m->unk7C, 4, m);
    }
    HeroSyncPairAnimSpeed();
    HeroSyncMappedSpeed();
    prev = ((Hero *)D_0013F450)->anim_frame;
    ((Hero *)D_0013F450)->anim_frame = func_0020D830(m);
    if (prev <= ((Hero *)D_0013F450)->anim_frame)
        ((Hero *)D_0013F450)->unkAAC = ((Hero *)D_0013F450)->anim_frame - prev;
    else
        ((Hero *)D_0013F450)->unkAAC = dt;
    ((Hero *)D_0013F450)->unkA9C = m->seq != m->prev_seq;
}
