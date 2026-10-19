/* RAC5 boot — sceMcEnd, retail 0x01EC3D10..0x01EC3DBC (172 bytes).
 * Authored 2026-10-17, wave 11 lot c1 from the retail disassembly.  Removes the
 * RPC queue/client at 0x01FDF500 / 0x01FDF518, deletes both semaphores and
 * invalidates the command semaphore. */
extern int MC_SEMA[];        /* 0x01EFE2CC */
extern int MC_SYNC_SEMA[];   /* 0x01EFE2D0 */
extern int MC_THREAD_ID[];   /* 0x01EFE2D4 */
extern int sceMcSync(int, int *, int *);
extern int TerminateThread(int);
extern int DeleteThread(int);
extern int DeleteSema(int);
extern int sceSifRemoveRpc(void *, void *);
extern int sceSifRemoveRpcQueue(void *);
extern int MC_RPC_QUEUE[];   /* 0x01FDF500 */
extern int MC_RPC_CLIENT[];  /* 0x01FDF518 */

int sceMcEnd(void) {
    if (MC_SEMA[0] >= 0) {
        sceMcSync(0, 0, 0);
        if (MC_THREAD_ID[0] >= 0) {
            TerminateThread(MC_THREAD_ID[0]);
            DeleteThread(MC_THREAD_ID[0]);
            MC_THREAD_ID[0] = -1;
            sceSifRemoveRpc(MC_RPC_CLIENT, MC_RPC_QUEUE);
            sceSifRemoveRpcQueue(MC_RPC_QUEUE);
        }
        DeleteSema(MC_SEMA[0]);
        MC_SEMA[0] = -1;
        DeleteSema(MC_SYNC_SEMA[0]);
    }
    return 1;
}
