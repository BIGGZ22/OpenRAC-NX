typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef void VOID;
#define NULL ((void *)0)

/* sceSifInitRpc - RAC5 boot 0x01ED3D88, adapted from RAC2 core
 * src/core/0011CF78.c:func_0011CF78 (itself from rac1-decomp
 * src/core/00119D88.c:func_0011AE20, Lombyte MIT).
 * Data and call addresses are RAC5's, read from the retail disassembly. */

typedef struct {
    s32 pid;
    void *pkt_table;
    s32 pkt_table_len;
    s32 unk0C;
    s32 unk10;
    void *rdata_table;
    s32 rdata_table_len;
    void *client_table;
    s32 client_table_len;
    s32 rdata_table_idx;
} SifRpcData;

extern s32 D_01EFEF54;
extern u8 D_01FE40C0[];
extern u8 D_01FE48C0[];
extern u8 D_01FE50C0[];
extern SifRpcData D_01FE58C0;

extern s32 DIntr(void);
extern s32 EIntr(void);
extern void sceSifInitCmd(void);
extern u32 sceSifGetReg(u32 reg);
extern u32 sceSifSetReg(u32 reg, u32 val);
extern u32 sceSifAddCmdHandler(u32 cid, void *handler, void *data);
extern u32 sceSifSendCmd(u32 cid, void *packet, s32 size, void *src, void *dst, s32 dsize);
extern u32 sceSifGetSreg(u32 reg);

extern void H_request_end(void);
extern void H_request_bind(void);
extern void H_request_rdata(void);
extern void H_request_000A(void);

void sceSifInitRpc(u32 mode) {
    s32 *packet;

    DIntr();
    if (D_01EFEF54) {
        EIntr();
        return;
    }
    D_01EFEF54 = 1;
    EIntr();
    sceSifInitCmd();
    DIntr();
    D_01FE58C0.pkt_table = (void *)((u32)D_01FE40C0 | 0x20000000);
    D_01FE58C0.pkt_table_len = 32;
    D_01FE58C0.unk0C = 0;
    D_01FE58C0.unk10 = 0;
    D_01FE58C0.rdata_table = (void *)((u32)D_01FE48C0 | 0x20000000);
    D_01FE58C0.rdata_table_len = 32;
    D_01FE58C0.client_table = (void *)((u32)D_01FE50C0 | 0x20000000);
    D_01FE58C0.client_table_len = 32;
    D_01FE58C0.rdata_table_idx = 0;
    D_01FE58C0.pid = 1;
    sceSifAddCmdHandler(0x80000008, (void *)H_request_end, &D_01FE58C0);
    sceSifAddCmdHandler(0x80000009, (void *)H_request_bind, &D_01FE58C0);
    sceSifAddCmdHandler(0x8000000A, (void *)H_request_000A, &D_01FE58C0);
    sceSifAddCmdHandler(0x8000000C, (void *)H_request_rdata, &D_01FE58C0);
    EIntr();
    if (sceSifGetReg(0x80000002)) {
        return;
    }
    packet = (s32 *)(D_01FE40C0 + 64);
    packet[3] = 1;
    sceSifSendCmd(0x80000002, packet, 16, 0, 0, 0);
    while (sceSifGetSreg(0) == 0) {
    }
    sceSifSetReg(0x80000002, 1);
}
