/* RAC5 boot — sceSifBindRpc, retail 0x01ED43D0..0x01ED451C (332 bytes)
 * then an unlabeled helper at 0x01ED4520.  Authored 2026-10-17, wave 11 lot
 * c1: lombyte sdk/rpc/sce_sif_bind_rpc.c re-pointed at RAC5 data and calls.
 * RAC5: request pool 0x01FE58C0, semaphore name 0x01F277A0, allocator
 * func_1ED3F50, release func_1ED3FF8. */
typedef struct {
    int unk0;
    int unk4;
    int unk8;
    unsigned char pad_C[0x4];
    int unk10;
    unsigned char pad_14[0x10];
    int unk24;
} SifClient;

typedef struct {
    unsigned char pad_0[0x14];
    int unk14;
    int unk18;
    int unk1C;
    int unk20;
} SifRequest;

extern int D_01FE58C0[];
extern char SIF_BIND_SEMA_NAME[];
extern int CreateSema(int *);
extern int DeleteSema(int);
extern int WaitSema(int);
extern int func_1ED3F50(int *);
extern int func_1ED3FF8(SifRequest *);
extern int sceSifSendCmd(int, SifRequest *, int, int, int, int);

int sceSifBindRpc(SifClient *client, int fno, int mode) {
    int sema_param[6];
    SifRequest *request;
    int sema;
    int sent;

    client->unk10 = 0;
    client->unk24 = 0;
    request = (SifRequest *)func_1ED3F50(D_01FE58C0);
    if (request == 0) {
        return -1;
    }
    client->unk4 = request->unk18;
    client->unk0 = (int)request;
    request->unk20 = fno;
    request->unk14 = (int)request;
    request->unk1C = (int)client;
    if (!(mode & 1)) {
        sema_param[1] = 1;
        sema_param[2] = 0;
        sema_param[5] = (int)SIF_BIND_SEMA_NAME;
        sema = CreateSema(sema_param);
        client->unk8 = sema;
        if (sema < 0) {
            func_1ED3FF8(request);
            return -3;
        }
        sent = sceSifSendCmd(0x80000009, request, 0x40, 0, 0, 0);
        if (sent == 0) {
            func_1ED3FF8(request);
            DeleteSema(client->unk8);
            return -2;
        }
        WaitSema(client->unk8);
        DeleteSema(client->unk8);
        return 0;
    } else {
        client->unk8 = -1;
        sent = sceSifSendCmd(0x80000009, request, 0x40, 0, 0, 0);
        if (sent != 0) {
            return 0;
        }
        func_1ED3FF8(request);
        return -2;
    }
}
