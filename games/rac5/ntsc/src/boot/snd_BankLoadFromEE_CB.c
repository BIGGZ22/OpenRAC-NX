/* 989snd: EE-side bank load callback.
   Adapted from the R&C1 decompilation (Lombyte upstream, MIT)
   src/audio/banks/snd_bank_load_from_ee_cb.c to the RAC5 globals measured in
   the retail disassembly. Retail: snd_BankLoadFromEE_CB 0x01E94AE8. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef long long s64;

struct SndCallback {
    void (*fn)(s32, long);
    s32 pad;
    long arg;
};

extern s32 snd_bank_state;           /* 0x01EF6A80 */
extern s32 snd_quiet_flag;           /* 0x01EF6AF0 */
extern s32 snd_cd_pending;           /* 0x01EF6AC0 */
extern struct SndCallback snd_cd_read_cb; /* 0x01EF6AC8 */
extern u32 snd_cd_reply;             /* 0x01FEF740 */
extern s32 snd_bank_cmd;             /* 0x01FEF7C0 */
extern u8 snd_bank_rpc_record[];     /* 0x01FD33B8 */
extern char snd_bank_busy_msg[];     /* 0x01F20478 */
extern char snd_rpc_busy_msg[];      /* 0x01F202F8 */

extern s32 printf(const char *, ...);
extern void snd_FlushSoundCommands(void);
extern void FlushCache(s32);
extern void SyncDCache(void *, void *);
extern s32 sceSifCheckStatRpc(void *);
extern s32 sceSifCallRpc(void *, s32, s32, void *, s32, void *, s32, void *, void *);

void snd_BankLoadFromEE_CB(s32 cmd, s32 arg, s64 data) {
    snd_bank_state = 0;
    if (snd_cd_pending != 0) {
        if (snd_quiet_flag == 0)
            printf(snd_bank_busy_msg);
        return;
    }
    snd_cd_reply = 0xFFFFFFFF;
    SyncDCache(&snd_cd_reply, &snd_cd_reply + 1);
    snd_cd_read_cb.fn = (void (*)(s32, long))arg;
    snd_cd_read_cb.arg = data;
    snd_bank_cmd = cmd;
    while (sceSifCheckStatRpc(snd_bank_rpc_record) != 0) {
        if (snd_quiet_flag == 0)
            printf(snd_rpc_busy_msg);
        snd_FlushSoundCommands();
        FlushCache(0);
    }
    snd_cd_pending = 1;
    sceSifCallRpc(snd_bank_rpc_record, 0x57, 1, &snd_bank_cmd, 4, &snd_cd_reply, 4,
                  0, 0);
}
