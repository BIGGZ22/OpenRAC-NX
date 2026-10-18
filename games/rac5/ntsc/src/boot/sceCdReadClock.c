/* RAC5 boot — sceCdReadClock, retail 0x01EC38C8..0x01EC3984 (188 bytes).
 * Authored 2026-10-17, wave 11 lot c1: lombyte sdk/library/scecdreadclock.c
 * without the RAC1 debug prints; RAC5 receive buffer 0x01EFD900, client
 * 0x01EFE280.  The 8-byte copy is unaligned, hence the packed struct. */
typedef unsigned long long u64;

struct unaligned64 {
    u64 v;
} __attribute__((packed));

extern int _sceCd_scmd_prechk(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int SignalSema(int);
extern int _sceCd_scmd_semid;
extern int _sceCd_cd_scmd[];
extern int CD_SCMD_RDATA[];

int sceCdReadClock(u64 *clock) {
    unsigned int result;

    if (_sceCd_scmd_prechk(0xF) == 0) {
        return 0;
    }
    if (sceSifCallRpc(_sceCd_cd_scmd, 1, 0, 0, 0, CD_SCMD_RDATA, 0x10, 0, 0) < 0) {
        SignalSema(_sceCd_scmd_semid);
        return 0;
    }
    *(struct unaligned64 *)clock =
        *(struct unaligned64 *)((unsigned int)(CD_SCMD_RDATA + 1) | 0x20000000);
    result = *(unsigned int *)((unsigned int)CD_SCMD_RDATA | 0x20000000);
    SignalSema(_sceCd_scmd_semid);
    return result;
}
