/* PS2 SDK sceSifSyncIop, retail 0x01ED8970 (80 bytes). */
extern int sceSifGetReg(int no);
extern int sceResetttyinit(void);
extern int sceSifExecNotifyHandler(int no, int mode);

int sceSifSyncIop(void) {
    if (sceSifGetReg(4) & 0x40000) {
        sceResetttyinit();
        sceSifExecNotifyHandler(1, 1);
        sceSifExecNotifyHandler(0, 1);
        return 1;
    }
    return 0;
}
