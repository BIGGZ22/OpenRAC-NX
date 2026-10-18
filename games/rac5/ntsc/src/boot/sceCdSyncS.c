/* sceCdSyncS - wait for the CD command RPC, or report its state. */

extern int sceSifCheckStatRpc(void *cd);
extern int scePrintf(const char *fmt);
extern int DelayThread(int usec);
extern int SCE_CD_debug;
extern char _sceCd_cd_scmd[];
extern char sceCdSyncS_debug_fmt[];

int sceCdSyncS(int mode) {
    if (mode == 0) {
        if (SCE_CD_debug > 0) scePrintf(sceCdSyncS_debug_fmt);
        while (sceSifCheckStatRpc(_sceCd_cd_scmd) != 0)
            DelayThread(4000);
        return 0;
    }
    return sceSifCheckStatRpc(_sceCd_cd_scmd);
}
