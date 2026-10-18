typedef struct {
    unsigned char reserved_00[0x24];
    int is_initialized;
} MemoryCardRpcClient;

typedef struct {
    int port;
    int slot;
    int flags;
    int maxent;
    int unk10;
    char name[0x400];
} MemoryCardNameParam;

extern MemoryCardRpcClient MC_CLIENT;
extern int MC_SEMA[];
extern int MC_STATUS;
extern unsigned char MC_NAMEBUF[];
extern int MC_OUTBUF[];
extern int PollSema(int);
extern int SignalSema(int);
extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern char *strncpy(char *, const char *, unsigned int);
extern void RPC_END(void);

int sceMcDelete(int port, int slot, char *name) {
    MemoryCardNameParam *request;
    int rpc_result;

    if (MC_CLIENT.is_initialized == 0) {
        return -0x64;
    }
    if (PollSema(MC_SEMA[0]) < 0) {
        return -0xC8;
    }
    if (name == 0 || name[0] == 0) {
        SignalSema(MC_SEMA[0]);
        return -0xD2;
    }
    strncpy((char *)MC_NAMEBUF, name, 0x3FF);
    request = (MemoryCardNameParam *)(MC_NAMEBUF - 0x14);
    request->port = port;
    request->slot = slot;
    request->name[0x3FF] = 0;
    request->flags = 0;
    rpc_result = sceSifCallRpc(&MC_CLIENT, 0xF, 1, request, 0x414, MC_OUTBUF, 4,
                               (void *)RPC_END, 0);
    if (rpc_result != 0) {
        SignalSema(MC_SEMA[0]);
        return -0x5B;
    }
    MC_STATUS = 0xF;
    return 0;
}
