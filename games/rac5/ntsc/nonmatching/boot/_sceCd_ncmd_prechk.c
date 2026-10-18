/* _sceCd_ncmd_prechk 0x01EC2888..0x01EC29E0 (344 bytes).
 *
 * The N-command (non-blocking) pre-check shared by the libcdvd routines: take
 * the command semaphore, remember the caller's socket, wait for the drive to
 * go idle, make sure the SIF RPC is up and the ncmd server bound, then leave
 * the semaphore held so the caller's RPC can run.
 *
 * Derived from the RAC1 decompilation (src/sdk/storage/cd_check_ncmd.c), which
 * is the same Sony libcdvd source under the same 2.9-ee compiler.  RAC5 drops
 * the ReferThreadStatus() bookkeeping that RAC1 keeps after the socket store,
 * and the retry spin is 0x10000 iterations, not 0x100000:
 *
 *   0x01EC289C  lui   $s0, 0x1F0 / lw $a0, -0x38D8($s0)   cd_sem
 *   0x01EC28B4  beq   $v1, $v0  (sem == PollSema(sem)) -> taken path
 *   0x01EC28C0  blez  $v1      verbose <= 0 -> return 0
 *   0x01EC28CC  lui $a0,0x1F2 / addiu 0x6418  "unsupported..."
 *   0x01EC28E8  addiu $a0, $zero, 0x1 / jal sceCdSync
 *   0x01EC28F4  sw $s1, -0x38E4($v0)   store the socket in the call's delay
 *   0x01EC2928  b 0x01EC2954           loop entry, jumps over the back-off
 *   0x01EC2960  ori $a1, 0x595         ncmd RPC id 0x80000595
 *   0x01EC296C  bgezl $v0, 0x01EC29BC  bind >= 0 -> test the bound flag
 *   0x01EC29BC  beqz $v0, 0x01EC2930   not bound -> back-off, retry
 *   0x01EC29C4  sw $zero, -0x38C4($s2) clear the "needs bind" flag
 *
 * The two identical back-off loops are the two `for` loops of the source; the
 * compiler emits them separately, which is what the retail stream shows. */

struct CdlCB {
    int pad_0[9];
    int unk24;                     /* +0x24: bound flag */
};

extern volatile int cd_sem;        /* 0x01EFC728 */
extern int cd_verbose;             /* 0x01EFC710 */
extern int cd_socket;              /* 0x01EFC71C */
extern int cd_bind_needed;         /* 0x01EFC73C, negative = needs bind */
extern struct CdlCB cd_ncmd_cli;   /* 0x01EFD8D0 */
extern char cd_msg_busy[];         /* 0x01F26418 */
extern char cd_msg_bind[];         /* 0x01F26440 */

extern void cmd_sem_init(void);
extern int PollSema(int);
extern int SignalSema(int);
extern int scePrintf(const char *, ...);
extern int sceCdSync(int);
extern void sceSifInitRpc(int);
extern int sceSifBindRpc(void *, unsigned int, int);

int _sceCd_ncmd_prechk(int arg0) {
    int spin;

    cmd_sem_init();
    if (cd_sem != PollSema(cd_sem)) {
        if (cd_verbose > 0) {
            scePrintf(cd_msg_busy, arg0, cd_socket);
        }
        return 0;
    }
    cd_socket = arg0;
    if (sceCdSync(1) != 0) {
        SignalSema(cd_sem);
        return 0;
    }
    sceSifInitRpc(0);
    if (cd_bind_needed >= 0) {
        return 1;
    }
    for (;;) {
        if (sceSifBindRpc(&cd_ncmd_cli, 0x80000595, 0) < 0) {
            if (cd_verbose > 0) {
                scePrintf(cd_msg_bind);
            }
            for (spin = 0x100000; spin != -1; spin--) {
            }
            continue;
        }
        if (cd_ncmd_cli.unk24 != 0) {
            break;
        }
        for (spin = 0x100000; spin != -1; spin--) {
        }
    }
    cd_bind_needed = 0;
    return 1;
}
