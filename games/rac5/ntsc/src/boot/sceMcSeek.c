/* RAC5 boot — sceMcSeek, retail 0x01EC40B8..0x01EC4194 (220 bytes) + 4-byte
 * alignment fill.  Authored 2026-10-17, wave 11 lot c1.
 * Source: lombyte-upstream sdk/storage/memory_card/sce_mc_seek.c, adapted to
 * RAC5 data (client 0x01FDDF00, sema 0x01EFE2CC, status 0x01EFE2C8, request
 * 0x01FDDF80, receive 0x01FDF4C0).  Retail uses RPC_END 0x01EC3DC0 and the
 * failure path returns -0x5B. */
typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int file_descriptor;
    unsigned char reserved_04[0xC];
    int offset;
    int origin;
} MemoryCardSeekRequest;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern MemoryCardSeekRequest MC_REQ;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern void RPC_END(void);

int sceMcSeek(int file_descriptor, int offset, int origin) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_REQ.file_descriptor = file_descriptor;
    MC_REQ.offset = offset;
    MC_REQ.origin = origin;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 4, 1, &MC_REQ, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 4;
    return 0;
}
