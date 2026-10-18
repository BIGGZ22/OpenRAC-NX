/* RAC5 boot — sceMcUnformat, retail 0x01EC4CE0..0x01EC4DAC (204 bytes).
 * Authored 2026-10-17, wave 11 lot c1 from the retail disassembly; same shape
 * as sceMcFormat with RPC function 0x11.  RAC5 data as sceMcFormat. */
typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int reserved_00;
    int port;
    int slot;
} MemoryCardUnformatRequest;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern MemoryCardUnformatRequest MC_REQ;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern void RPC_END(void);

int sceMcUnformat(int port, int slot) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ.port = port;
    MC_REQ.slot = slot;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0x11, 1, &MC_REQ, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0x11;
    return 0;
}
