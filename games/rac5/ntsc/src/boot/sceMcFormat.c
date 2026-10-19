/* RAC5 boot — sceMcFormat, retail 0x01EC48D0..0x01EC499C (204 bytes) + fill.
 * Authored 2026-10-17, wave 11 lot c1 from the retail disassembly; the
 * lombyte sce_mc_format.c is the same RPC shape, re-pointed at RAC5 data.
 * RAC5: client 0x01FDDF00, sema 0x01EFE2CC, status 0x01EFE2C8, request
 * 0x01FDDF80 (port at +4, slot at +8), receive 0x01FDF4C0, end 0x01EC3DC0. */
typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int reserved_00;
    int port;
    int slot;
} MemoryCardFormatRequest;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern MemoryCardFormatRequest MC_REQ;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern void RPC_END(void);

int sceMcFormat(int port, int slot) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ.port = port;
    MC_REQ.slot = slot;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0x10, 1, &MC_REQ, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0x10;
    return 0;
}
