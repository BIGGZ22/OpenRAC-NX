/* _sceCd_scmd_prechk 0x01EC2B88..0x01EC2CE0 (344 bytes).
 *
 * The S-command (synchronous) twin of _sceCd_ncmd_prechk: same semaphore,
 * same "already bound" short cut, but the idle wait is sceCdSyncS and the RPC
 * id is 0x80000593.
 *
 * RAC5 data (lui 0x1F0 + signed offsets, base 0x01F00000):
 *   -0x38D4 -> 0x01EFC72C  command semaphore
 *   -0x38F0 -> 0x01EFC710  verbose level
 *   -0x38E8 -> 0x01EFC718  last socket
 *   -0x38AC -> 0x01EFC754  bind-needed flag (negative)
 *   -0x1D80 -> 0x01EFE280  scmd client block, bound flag at +0x24
 *   0x01F26478 / 0x01F264A0  the two diagnostic format strings
 *
 * Byte-identical to _sceCd_ncmd_prechk except for those five constants and the
 * callee at 0x01EC2BF0, which is what pins this body to this exact source. */

struct CdlCB {
    int pad_0[9];
    int unk24;                     /* +0x24: bound flag */
};

extern volatile int scmd_sem;      /* 0x01EFC72C */
extern int cd_verbose;             /* 0x01EFC710 */
extern int scmd_socket;            /* 0x01EFC718 */
extern int scmd_bind_needed;       /* 0x01EFC754, negative = needs bind */
extern struct CdlCB scmd_cli;      /* 0x01EFE280 */
extern char scmd_msg_busy[];       /* 0x01F26478 */
extern char scmd_msg_bind[];       /* 0x01F264A0 */

extern void cmd_sem_init(void);
extern int PollSema(int);
extern int SignalSema(int);
extern int scePrintf(const char *, ...);
extern int sceCdSyncS(int);
extern void sceSifInitRpc(int);
extern int sceSifBindRpc(void *, unsigned int, int);

int _sceCd_scmd_prechk(int arg0) {
    int spin;

    cmd_sem_init();
    if (scmd_sem != PollSema(scmd_sem)) {
        if (cd_verbose > 0) {
            scePrintf(scmd_msg_busy, arg0, scmd_socket);
        }
        return 0;
    }
    scmd_socket = arg0;
    if (sceCdSyncS(1) != 0) {
        SignalSema(scmd_sem);
        return 0;
    }
    sceSifInitRpc(0);
    if (scmd_bind_needed >= 0) {
        return 1;
    }
    for (;;) {
        if (sceSifBindRpc(&scmd_cli, 0x80000593, 0) < 0) {
            if (cd_verbose > 0) {
                scePrintf(scmd_msg_bind);
            }
            for (spin = 0x100000; spin != -1; spin--) {
            }
            continue;
        }
        if (scmd_cli.unk24 != 0) {
            break;
        }
        for (spin = 0x100000; spin != -1; spin--) {
        }
    }
    scmd_bind_needed = 0;
    return 1;
}
