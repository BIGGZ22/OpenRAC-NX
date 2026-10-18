/* 989snd: flush completed commands, CD requests and stop requests.
   Adapted from the R&C1 decompilation (Lombyte upstream, MIT)
   src/audio/snd_flush_sound_commands.c to the RAC5 globals measured in the
   retail disassembly. Retail: snd_FlushSoundCommands 0x01E94700. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

struct SndCallback {
    void (*fn)(s32, long);
    s32 pad;
    long arg;
};

extern s32 snd_pending;              /* 0x01EF6A78 */
extern void (*snd_stop_cb)(s32);          /* 0x01EF6A88 */
extern s32 snd_stop_request;         /* 0x01EF6A8C */
extern s32 snd_stop_state;           /* 0x01EF6A90 */
extern s32 snd_abort;                /* 0x01EF6A94 */
extern s32 *batch_counts[2];         /* 0x01EF6A98 */
extern struct SndCallback *batch_commands[2]; /* 0x01EF6AA8 */
extern s32 *batch_replies[2];        /* 0x01EF6AB0 */
extern s32 batch_current;            /* 0x01EF6AB8 */
extern s32 snd_batch_hold;           /* 0x01EF6ABC */
extern s32 snd_cd_pending;           /* 0x01EF6AC0 */
extern struct SndCallback snd_cd_read_cb;  /* 0x01EF6AC8 */
extern void (*snd_abort_cb)(s32, long);    /* 0x01EF6ADC */
extern long snd_abort_arg;                 /* 0x01EF6AE0 */
extern u32 snd_cd_reply;             /* 0x01FEF740 */
extern s32 snd_abort_reply;          /* 0x01FEF784 */

extern s32 snd_GotReturns(void);
extern void snd_SendCurrentBatch(void);
extern s32 snd_StreamSafeCdSync(s32);
extern void InvalidDCache(void *, void *);
extern void FlushCache(s32);

s32 snd_FlushSoundCommands(void) {
    s32 i;
    s32 idx;
    s32 ready;

    if (snd_pending != 0 && (ready = snd_GotReturns()) != 0) {
        if (snd_abort != 0) {
            if (snd_abort_cb != 0) {
                snd_abort_cb(snd_abort_reply, snd_abort_arg);
            }
            snd_abort_cb = 0;
            snd_abort = 0;
        } else if (ready == 1) {
            idx = batch_current != 1;
            for (i = 0; i < *batch_counts[idx]; i++) {
                if (batch_commands[idx][i].fn != 0) {
                    batch_commands[idx][i].fn(batch_replies[idx][i + 1],
                                              batch_commands[idx][i].arg);
                }
            }
        }
    }
    if (snd_cd_pending != 0) {
        InvalidDCache(&snd_cd_reply, &snd_cd_reply + 1);
        if (snd_cd_reply != 0xFFFFFFFFu) {
            FlushCache(0);
            if (snd_cd_read_cb.fn != 0) {
                void (*fn)(s32, long) = snd_cd_read_cb.fn;
                long arg = snd_cd_read_cb.arg;
                snd_cd_read_cb.fn = 0;
                snd_cd_read_cb.arg = 0;
                fn(snd_cd_reply, arg);
            }
            snd_cd_reply = 0;
            snd_cd_pending = 0;
        }
    }
    if (snd_pending == 0) {
        if (*batch_counts[batch_current] != 0 && snd_batch_hold == 0) {
            snd_SendCurrentBatch();
        }
    }
    if (snd_stop_request != 0) {
        snd_StreamSafeCdSync(1);
        if (snd_stop_state != 0) {
            snd_stop_request = 0;
            snd_stop_state = 0;
            if (snd_stop_cb != 0) {
                snd_stop_cb(1);
            }
        }
    }
    return snd_pending != 0 || snd_cd_pending != 0;
}
