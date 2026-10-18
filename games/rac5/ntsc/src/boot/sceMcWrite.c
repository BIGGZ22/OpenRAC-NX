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
    int origin;
    char *data;
    int unk1C;
    unsigned char data2[0x400];
} McWriteParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern McWriteParam MC_INFO;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int FlushCache(int);
extern void RPC_END(void);

int sceMcWrite(int fd, unsigned char *buf, int size) {
    unsigned int i;
    int root_off;
    int base;
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_INFO.fd = fd;
    if (size < 0x11) {
        MC_INFO.origin = size;
        MC_INFO.byte_count = 0;
        MC_INFO.data = 0;
    } else {
        base = (int)buf - 0x10;
        root_off = (((int)buf - 1) & 0xFFFFFFF0) - base;
        MC_INFO.byte_count = size - root_off;
        MC_INFO.origin = root_off;
        MC_INFO.data = buf + root_off;
    }
    for (i = 0; i < MC_INFO.origin; i++) {
        MC_INFO.data2[i] = buf[i];
    }
    FlushCache(0);
    rpc_result = sceSifCallRpc(&MC_CLIENT, 6, 1, &MC_INFO, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 6;
    return 0;
}
