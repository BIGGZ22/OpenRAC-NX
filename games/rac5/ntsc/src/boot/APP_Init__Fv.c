extern void PS2HEAP_Init__Fv(void);
extern int sceSifInitRpc(int mode);
extern int ps2fioInit(void);
extern int ps2fioRebootIop(void *arg);
extern int sceSifSyncIop(void);
extern int asyncInit(void);
extern int ps2fioInitializeDevice(void *dev, void *name);
extern int ps2fioBuildFilesystem(void *dev, void *fs, int n);

extern char g_fsys_iop[];
extern char g_fsys_dev[];
extern char g_fsys_name[];
extern char g_fsys_fs[];

int APP_Init__Fv(void) {
    PS2HEAP_Init__Fv();
    sceSifInitRpc(0);
    ps2fioInit();
    do {
    } while (ps2fioRebootIop(g_fsys_iop) == 0);
    do {
    } while (sceSifSyncIop() == 0);
    sceSifInitRpc(0);
    asyncInit();
    if (ps2fioInitializeDevice(g_fsys_dev, g_fsys_name) < 0) {
        return 0;
    }
    ps2fioBuildFilesystem(g_fsys_dev, g_fsys_fs, 0xFA);
    return 1;
}
