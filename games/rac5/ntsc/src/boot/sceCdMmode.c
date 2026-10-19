/* RAC5 boot — sceCdMmode, retail 0x01EC37F0..0x01EC38C0 (208 bytes).
 * Authored 2026-10-17, wave 11 lot c1: lombyte sdk/library/scecdmmode.c
 * re-pointed at RAC5: the SCMD descriptor supplies fno 0x01EFE2A8, ssize
 * 0x01EFE2AC and rsize 0x01EFE2B0; data buffer 0x01EFDD40, receive 0x01EFD900,
 * client 0x01EFE280. */
extern int _sceCd_scmd_prechk(int);
extern void sceSifWriteBackDCache(void *, int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int SignalSema(int);
extern int _sceCd_scmd_semid;
extern int _sceCd_cd_scmd[];
extern int CD_SCMD_DATA[];
extern int CD_SCMD_RDATA[];
extern int CD_SCMD_CMD[];
extern int CD_SCMD_SSIZE[];
extern int CD_SCMD_RSIZE[];

int sceCdMmode(int arg0) {
    register unsigned char *state;
    register int mode;
    int result;

    state = (unsigned char *)CD_SCMD_DATA;
    mode = arg0;
    if (_sceCd_scmd_prechk(0x22) == 0) {
        return 0;
    }
    *(int *)state = mode;
    sceSifWriteBackDCache(state, 4);
    if (sceSifCallRpc(_sceCd_cd_scmd, CD_SCMD_CMD[0], 0, state, CD_SCMD_SSIZE[0],
                      CD_SCMD_RDATA, CD_SCMD_RSIZE[0], 0, 0) < 0) {
        SignalSema(_sceCd_scmd_semid);
        return 0;
    }
    result = *(int *)((unsigned int)CD_SCMD_RDATA | 0x20000000);
    SignalSema(_sceCd_scmd_semid);
    return result;
}
