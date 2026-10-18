/* sceSifRebootIop 0x01ED89C0..0x01ED8AD0 (272 bytes).
 *
 * Retail (SCUS_976.15) disassembly b03/disasm/sceSifRebootIop.txt:
 * length check (len + 11 > 80 -> scePrintf(fmt, img); return 0),
 * sceSifInitRpc(0), sceSifExitRpc(), prefix+image concat into param[80],
 * then sceSifResetIop(param, 0).
 * Source: lombyte-upstream src/sdk/rpc/sce_sif_reboot_iop.c, adapted to the
 * RAC5 revision (sceSifExitRpc instead of InitializeRpcCommand; data
 * addresses 0x01F27A80/0x01F27A90).
 * Date: 2026-10-17 (w11/b03).
 */
extern char str_reboot_prefix[];
extern char str_reboot_fmt[];
extern int scePrintf(const char *, ...);
extern void sceSifInitRpc(unsigned int);
extern void sceSifExitRpc(void);
extern int sceSifResetIop(const char *, int);

int sceSifRebootIop(const char *img) {
    const char *prefix = str_reboot_prefix;
    char param[80];
    const char *p;
    char *d;

    p = img;
    while (*p) {
        p++;
    }
    if ((unsigned int)(p + 11 - img) > 80) {
        scePrintf(str_reboot_fmt, img);
        return 0;
    }
    sceSifInitRpc(0);
    sceSifExitRpc();
    d = param;
    while (*prefix) {
        *d++ = *prefix++;
    }
    while (*img) {
        *d++ = *img++;
    }
    *d = 0;
    return sceSifResetIop(param, 0);
}
