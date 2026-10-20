/* func_002ED2B8: 160 bytes compiled (-O2 -G8, the src/game/002ED260.c profile), retail 172.
   Still differs: the second 16-byte copy (lq/sq at +0x10) is emitted as `sq $v1,0x10($a2)`. This
   cc1 folds cursor+16 into the store, where retail keeps table+16 in $a3 and source+16 in $t0.
   The table pointer also goes straight into $a2 (retail: $v0, then daddu), and the owner store
   sits before jr where retail has it in the delay slot. The 11 int/float field copies match in
   order. */
#include "include_asm.h"

#ifndef RAC2_T_U32_FE8672EA5
#define RAC2_T_U32_FE8672EA5
typedef unsigned int u32_Fe8672ea5;
#endif

extern u32_Fe8672ea5 D_L04_001AC18C __attribute__((sda));

typedef int RecordTi16 __attribute__((mode(TI)));

void func_002ED2B8(u32_Fe8672ea5 owner, u32_Fe8672ea5 source)
{
    u32_Fe8672ea5 cursor = D_L04_001AC18C;
    u32_Fe8672ea5 tail;
    u32_Fe8672ea5 source_tail;
    int i;

    if (cursor == 0) {
        return;
    }
    tail = cursor + 16;
    source_tail = source + 16;
    for (i = 0; i < 512; i++) {
        if (*(u32_Fe8672ea5 *)(cursor + 0x48) == 0) {
            *(u32_Fe8672ea5 *)(cursor + 0x38) = *(u32_Fe8672ea5 *)(source + 0x38);
            *(u32_Fe8672ea5 *)(cursor + 0x3C) = *(u32_Fe8672ea5 *)(source + 0x3C);
            *(u32_Fe8672ea5 *)(cursor + 0x30) = *(u32_Fe8672ea5 *)(source + 0x30);
            *(u32_Fe8672ea5 *)(cursor + 0x44) = *(u32_Fe8672ea5 *)(source + 0x44);
            *(float *)(cursor + 0x24) = *(float *)(source + 0x24);
            *(float *)(cursor + 0x20) = *(float *)(source + 0x20);
            *(float *)(cursor + 0x28) = *(float *)(source + 0x28);
            *(u32_Fe8672ea5 *)(cursor + 0x34) = *(u32_Fe8672ea5 *)(source + 0x34);
            *(u32_Fe8672ea5 *)(cursor + 0x2C) = *(u32_Fe8672ea5 *)(source + 0x2C);
            *(u32_Fe8672ea5 *)(cursor + 0x40) = *(u32_Fe8672ea5 *)(source + 0x40);
            *(u32_Fe8672ea5 *)(cursor + 0x4C) = *(u32_Fe8672ea5 *)(source + 0x4C);
            *(RecordTi16 *)cursor = *(RecordTi16 *)source;
            *(RecordTi16 *)tail = *(RecordTi16 *)source_tail;
            *(u32_Fe8672ea5 *)(cursor + 0x48) = owner;
            return;
        }
        cursor += 80;
        tail += 80;
    }
}
