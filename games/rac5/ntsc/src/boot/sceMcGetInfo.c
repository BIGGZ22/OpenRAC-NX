typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int unk00;
    int unk04;
    int unk08;
    int unk0C;
    int unk10;
    int unk14;
    int unk18;
    void *unk1C;
} McInfoParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern McInfoParam MC_INFO;
extern char MC_XFER[];
extern int MC_OUTBUF[];
extern int G_INFO2C[];
extern int G_INFO30[];
extern int G_INFO34[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int sceSifWriteBackDCache(void *, int);
extern void RPC_INFO_END(void);

int sceMcGetInfo(int arg0, int arg1, int arg2, int arg3, int arg4) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    MC_INFO.unk04 = arg0;
    MC_INFO.unk08 = arg1;
    MC_INFO.unk1C = MC_XFER;
    if (arg2 != 0) {
        MC_INFO.unk14 = 1;
    } else {
        MC_INFO.unk14 = 0;
    }
    if (arg3 != 0) {
        MC_INFO.unk10 = 1;
    } else {
        MC_INFO.unk10 = 0;
    }
    if (arg4 != 0) {
        MC_INFO.unk0C = 1;
    } else {
        MC_INFO.unk0C = 0;
    }
    G_INFO2C[0] = arg2;
    G_INFO30[0] = arg3;
    G_INFO34[0] = arg4;
    sceSifWriteBackDCache(MC_XFER, 0xC0);
    rpc_result = sceSifCallRpc(&MC_CLIENT, 1, 1, &MC_INFO, 0x30, MC_OUTBUF, 4,
                               (void *)RPC_INFO_END, MC_XFER);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 1;
    return 0;
}
