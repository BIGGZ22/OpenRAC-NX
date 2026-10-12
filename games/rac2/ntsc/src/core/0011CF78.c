#include "common.h"
#include "structs.h"
#include <stdarg.h>

typedef struct {
    int pid;
    void *pkt_table;
    int pkt_table_len;
    int unk0C;
    int unk10;
    void *rdata_table;
    int rdata_table_len;
    void *client_table;
    int client_table_len;
    int rdata_table_idx;
} SifRpcData;

extern int r1_D_0012FD08 NOT_SDA;

extern char r1_D_00155100[];

extern char r1_D_00155900[];

extern char r1_D_00156100[];

extern SifRpcData r1_D_00156900_d __asm__("r1_D_00156900");

extern int r1_func_0011D960(void);

           /* DI */
extern void r1_func_0011D9A8(void);

          /* EI */
extern void r1_func_0011A780(void);

          /* sceSifInitCmd */
extern int r1_func_00118E70(int);

            /* sceSifGetReg */
extern int r1_func_00118E60(int, int);

       /* sceSifSetReg */
extern void func_0011D238(char *);

extern void func_0011D3A0(void *, void *);

extern void func_0011D590(void *, void *);

extern void func_0011D2F0(void *, void *);

extern char r1_D_00156900[];

typedef signed int s32_1B4C8;

extern s32_1B4C8 func_0011CD20();

/* rac1-decomp src/core/00119D88.c: func_0011AE20 */
/* sceSifInitRpc: once, set up the RPC tables and command handlers, then
   tell the IOP (SIF register 0x80000002) and wait for it. Adapted from
   Lombyte (MIT) for PAL. */
void func_0011CF78(int mode) {
    int *packet;

    r1_func_0011D960();
    if (r1_D_0012FD08) {
        r1_func_0011D9A8();
        return;
    }
    r1_D_0012FD08 = 1;
    r1_func_0011D9A8();
    r1_func_0011A780();
    r1_func_0011D960();
    r1_D_00156900_d.pkt_table = (void *)((unsigned int)r1_D_00155100 | 0x20000000);
    r1_D_00156900_d.pkt_table_len = 32;
    r1_D_00156900_d.unk0C = 0;
    r1_D_00156900_d.unk10 = 0;
    r1_D_00156900_d.rdata_table = (void *)((unsigned int)r1_D_00155900 | 0x20000000);
    r1_D_00156900_d.rdata_table_len = 32;
    r1_D_00156900_d.client_table = (void *)((unsigned int)r1_D_00156100 | 0x20000000);
    r1_D_00156900_d.client_table_len = 32;
    r1_D_00156900_d.rdata_table_idx = 0;
    r1_D_00156900_d.pid = 1;
    func_0011CB90(0x80000008, (int)func_0011D238, (int)&r1_D_00156900_d);
    func_0011CB90(0x80000009, (int)func_0011D3A0, (int)&r1_D_00156900_d);
    func_0011CB90(0x8000000A, (int)func_0011D590, (int)&r1_D_00156900_d);
    func_0011CB90(0x8000000C, (int)func_0011D2F0, (int)&r1_D_00156900_d);
    r1_func_0011D9A8();
    if (r1_func_00118E70(0x80000002)) {
        return;
    }
    packet = (int *)(r1_D_00155100 + 64);
    packet[3] = 1;
    func_0011CD20(0x80000002, (int)packet, 16, 0, 0, 0);
    while (func_0011C8B0(0) == 0) {
    }
    r1_func_00118E60(0x80000002, 1);
}
