typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int fd;
    int unk04;
    int unk08;
    int byte_count;
    int unk10;
    int unk14;
    void *dest;
    void *transfer;
} McReadParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern McReadParam MC_INFO;
extern char MC_XFER[];
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int sceSifWriteBackDCache(void *, int);
extern void RPC_READ_END(void);

int sceMcRead(int fd, void *buf, int count) {
    McReadParam *request;
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    request = &MC_INFO;
    request->fd = fd;
    request->dest = buf;
    request->byte_count = count;
    request->transfer = MC_XFER;
    sceSifWriteBackDCache(buf, count);
    sceSifWriteBackDCache(MC_XFER, 0xC0);
    rpc_result = sceSifCallRpc(&MC_CLIENT, 5, 1, request, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_READ_END, MC_XFER);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 5;
    return 0;
}
