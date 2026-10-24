/* sceFsInit, the file-I/O RPC library bring-up in cod/015180: adapted from
   Lombyte (GPL-3.0) src/assembly/sdk/library/sceFsInit.c (its NON_MATCHING body),
   which is the recovered source for this function. 516 bytes, matching retail
   instruction for instruction - same frame, same six saves in the same 16-byte
   slots, same relocations derived from the retail bytes - except the ORDER of
   the first two prologue saves. Retail emits `sd s0,0x20($sp); lui s0,%hi;
   sd s1,0x30($sp); sd ra,0x70($sp); addiu s1,s0,%lo`; this cc1 (both the
   `sdk -O2 -G2` and the game compiler) emits `sd s1,0x30; sd s0,0x20;
   sd ra,0x70; lui s0; sd s4...; addiu s1,s0,%lo`. Retail's save walk starts at
   s0, this build's starts at s1, i.e. the walk is rotated by one register;
   nothing else in the 516 bytes differs. Unblocked by a compiler knob that
   starts the GPR save walk at GP_REG_FIRST (the ascending-order recipe this
   repo applies to the game cc1 is not in this sdk cc1), not by a source shape:
   the pointer's position in the statement list, its declaration order, the
   `s32 *` vs `struct FsRequest *` type, `-mno-split-addresses` and the
   `-g2 -gstabs` profile were all tried. */
#include "common.h"
#include "structs.h"

/* sceFsInit: brings up the file-I/O RPC library. Adapted from Lombyte (GPL-3.0)
 * src/assembly/sdk/library/sceFsInit.c, whose NON_MATCHING body is the
 * recovered source. RAC2 addresses replace Lombyte's (every one of them is
 * Lombyte's minus 0x17F00): D_00140100 is the library's command block,
 * D_00140080 its RPC queue, D_0013FE80 the 0x20-entry slot array at 0x10
 * bytes each, D_0013E940 the outbound request, D_0013F5C0/D_0013F600 the
 * reply and the two heap pointers, D_001400C0 the receive handler's data. */

struct FsRequest {
    int field0;
    int field4;
};

struct FsQueue {
    u8 pad0[0x24];
    int reply_ready;
};

struct FsSemaSlot {
    u8 pad0[4];
    int value;
    u8 pad1[8];
};

struct FsWord4 {
    u8 bytes[4];
};

extern u8 D_0013FE80[];
extern int D_0013472C;
extern int D_00134730;
extern int D_00134738;
extern struct FsRequest D_0013E940[];
extern u8 D_0013F5C0[];
extern u8 D_0013F600[];
extern struct FsQueue D_00140080;
extern u8 D_001400C0[];
extern struct FsWord4 D_001400A8[];
extern s32 D_00140100[];

extern void func_0011CF78(s32 mode);
extern s32 func_0011F5E0(void);
extern s32 func_0011F628(void);
extern s32 func_0011CB90(s32 cmd, void *handler, void *data);
extern s32 func_0011D450(void *queue, s32 rpc, s32 mode);
extern s32 func_0011D868(void);
extern s32 func_0011AC60(s32 sema);
extern s32 func_0011AC40(s32 sema);
extern s32 func_0011D620(void *, s32, s32, void *, s32, void *, s32, s32, s32);
extern void func_0011D9C0(void);
extern void func_0011DDD8(void);

s32 func_0011DE08(void) {
    s32 irq_state;
    s32 countdown;
    struct FsRequest *rpc_command;
    struct FsSemaSlot *sema_slot;
    u8 *sema_end;
    struct FsWord4 response_word;
    s32 *init_request;

    func_0011CF78(0);
    D_00140100[0] = 0;
    init_request = D_00140100;
    init_request[1] = 0;
    irq_state = func_0011F5E0();
    func_0011CB90(0x80000011, func_0011D9C0, D_001400C0);
    func_0011CB90(0x80000013, func_0011DDD8, D_00140100);

    if (irq_state != 0) {
        func_0011F628();
    }

    for (;;) {
        if (func_0011D450(&D_00140080, 0x80000001, 0) < 0) {
            return -1;
        }
        if (D_00140080.reply_ready != 0) {
            break;
        }
        countdown = 0x100000;
        do {
            countdown--;
        } while (countdown != -1);
    }

    func_0011D868();
    func_0011AC60(D_00134738);
    sema_slot = (struct FsSemaSlot *)D_0013FE80;
    sema_end = &D_0013FE80[0x200];
    while ((u8 *)sema_slot < sema_end) {
        sema_slot->value = 0;
        sema_slot++;
    }
    func_0011AC40(D_00134738);

    rpc_command = &D_0013E940[0];
    rpc_command->field0 = (s32)&D_0013F600[0];
    rpc_command->field4 = (s32)&D_0013F600[0x440];
    if (func_0011D620(&D_00140080, 0xFF, 0, rpc_command, 8, (s32)D_0013F5C0, 8, 0, 0) < 0) {
        return 0xFFFEFFFF;
    }

    D_001400A8[0] = *(struct FsWord4 *)((s32)D_0013F5C0 | 0x20000000);
    response_word = *(struct FsWord4 *)(((s32)D_0013F5C0 + 4) | 0x20000000);
    D_00134730 = (*(s32 *)&response_word == 2);
    D_0013472C = 1;
    return 0;
}
