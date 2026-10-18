/* RAC5 boot — sceMcClose, retail 0x01EC3FF8..0x01EC40B4 (188 bytes) + 4-byte
 * alignment fill.  Authored 2026-10-17, wave 11 lot c1.
 * Source: lombyte-upstream sdk/storage/memory_card/sce_mc_close.c, adapted to
 * RAC5 data: MC client 0x01FDDF00 (lui 0x1FE / addiu -0x2100), semaphore
 * 0x01EFE2CC, status 0x01EFE2C8, request 0x01FDDF80, receive 0x01FDF4C0.
 * Two RAC1 details do not hold in RAC5: the RPC end callback is RPC_END
 * 0x01EC3DC0 (not null) and the failure path returns -0x5B (not the RPC code). */
typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern int MC_REQ[];
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern void RPC_END(void);

int sceMcClose(int file_descriptor) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ[0] = file_descriptor;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 3, 1, MC_REQ, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 3;
    return 0;
}
