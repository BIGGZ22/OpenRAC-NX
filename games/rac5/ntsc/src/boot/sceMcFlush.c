/* RAC5 boot — sceMcFlush, retail 0x01EC4AC0..0x01EC4B7C (188 bytes).
 * Authored 2026-10-17, wave 11 lot c1 from the retail disassembly; same shape
 * as sceMcClose with RPC function 0x0A.  RAC5 data as sceMcClose. */
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

int sceMcFlush(int file_descriptor) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ[0] = file_descriptor;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0x0A, 1, MC_REQ, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0x0A;
    return 0;
}
