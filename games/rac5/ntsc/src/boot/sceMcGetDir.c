typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int port;
    int slot;
    int maxent;
    int unk0C;
    int unk10;
    char name[0x400];
} MemoryCardDirParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern MemoryCardDirParam MC_DIR;
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int sceSifWriteBackDCache(void *, int);
extern char *strncpy(char *, const char *, unsigned int);
extern void RPC_END(void);

int sceMcGetDir(int port, int slot, char *path, int maxent, int arg4, int arg5) {
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    if (path == 0 || path[0] == 0) {
        SignalSema(MC_SEMA[0]);
        return -0xD2;
    }
    MC_DIR.port = port;
    MC_DIR.slot = slot;
    MC_DIR.maxent = maxent;
    MC_DIR.unk0C = arg4;
    MC_DIR.unk10 = arg5;
    strncpy(MC_DIR.name, path, 0x3FF);
    MC_DIR.name[0x3FF] = 0;
    if (arg4 >= 0) {
        sceSifWriteBackDCache((void *)arg5, arg4 << 6);
    }
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0xD, 1, &MC_DIR, 0x414, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0xD;
    return 0;
}
