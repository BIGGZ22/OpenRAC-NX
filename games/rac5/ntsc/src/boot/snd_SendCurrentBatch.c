/* 989snd: send the current command batch over SIF RPC.
   Adapted from the R&C1 decompilation (Lombyte upstream, MIT)
   src/audio/rpc/snd_send_current_batch.c to the RAC5 globals measured in the
   retail disassembly. Retail: snd_SendCurrentBatch 0x01E95798. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

extern s32 *batch_counts[2];        /* 0x01EF6A98 */
extern s32 batch_offsets[2];        /* 0x01EF6AA0 */
extern s32 *batch_replies[2];       /* 0x01EF6AB0 */
extern s32 batch_current;           /* 0x01EF6AB8 */
extern s32 snd_quiet_flag;          /* 0x01EF6AF0: printf is skipped when set */
extern u8 snd_rpc_record[];         /* 0x01FD3390 */
extern char snd_rpc_busy_msg[];     /* 0x01F202F8 */

extern void snd_PrepareReturnBuffer(void *obj, s32 count, s32 *ptr);
extern s32 sceSifCheckStatRpc(void *);
extern s32 printf(const char *, ...);
extern void FlushCache(s32);
extern s32 sceSifCallRpc(void *, u32, u32, void *, s32, void *, s32, void *, void *);

void snd_SendCurrentBatch(void) {
    s32 next;

    snd_PrepareReturnBuffer(batch_replies[batch_current], *batch_counts[batch_current],
                            batch_counts[batch_current]);
    while (sceSifCheckStatRpc(snd_rpc_record) != 0) {
        if (snd_quiet_flag == 0)
            printf(snd_rpc_busy_msg);
        FlushCache(0);
    }
    sceSifCallRpc(snd_rpc_record, 0x4D, 1, batch_counts[batch_current],
                  0x1000 - batch_offsets[batch_current], batch_replies[batch_current],
                  (*batch_counts[batch_current] << 2) + 8, 0, 0);
    next = batch_current != 1;
    batch_current = next;
    *batch_counts[batch_current] = 0;
    batch_offsets[batch_current] = 0xFFC;
}
