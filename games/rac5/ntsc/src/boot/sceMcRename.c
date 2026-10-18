typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int port;
    int slot;
    int flags;
    int unk0C;
    void *unk10;
    char name[0x400];
} MemoryCardRenameParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern MemoryCardRenameParam MC_DIR;
extern char MC_RENAME_BUF[];
extern char MC_RENAME_NAME[];
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern char *strncpy(char *, const char *, unsigned int);
extern int FlushCache(int);
extern void RPC_END(void);

int sceMcRename(int port, int slot, char *name1, char *name2) {
    MemoryCardRenameParam *request;
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    if (name1 == 0 || name1[0] == 0 || name2 == 0) {
        SignalSema(MC_SEMA[0]);
        return -0xD2;
    }
    request = &MC_DIR;
    request->port = port;
    request->slot = slot;
    request->flags = 0x10;
    strncpy(request->name, name1, 0x3FF);
    request->name[0x3FF] = 0;
    strncpy(MC_RENAME_NAME, name2, 0x20);
    request->unk10 = MC_RENAME_NAME - 0x20;
    FlushCache(0);
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0xE, 1, request, 0x414, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0x13;
    return 0;
}
