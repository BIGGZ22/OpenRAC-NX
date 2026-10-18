/* RAC5 boot — sceMcGetSlotMax, retail 0x01EC3DD8..0x01EC3E98 (192 bytes).
 * Authored 2026-10-17, wave 11 lot c1 from the retail disassembly.  The port is
 * written at request+4; the RPC is asynchronous (mode 0) and the slot count is
 * read from the receive buffer. */
typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int reserved_00;
    int port;
} MemoryCardSlotRequest;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern MemoryCardSlotRequest MC_REQ;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);

int sceMcGetSlotMax(int port) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ.port = port;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0x15, 0, &MC_REQ, 0x30, MC_OUTBUF, 4, 0, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    SignalSema(MC_SEMA[0]);
    return MC_OUTBUF[0];
}
