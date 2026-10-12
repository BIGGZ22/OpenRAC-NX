#include "include_asm.h"

extern void func_00291788(unsigned long long);

extern void func_00291640(unsigned long long);

/* Frozen Batch A A02-list-add-88; acknowledged claim 38ceeb041b6ba12996de10c402ca54295587db96246845b83b2ec894986b096f. */
typedef unsigned char BatchA_A02_u8;

typedef int BatchA_A02_s32;

typedef struct { char _0[0x10]; BatchA_A02_u8 n; char _11[3]; BatchA_A02_s32 off; char _18[4]; BatchA_A02_s32 ent[1]; } BatchA_A02_Blk;

typedef int s32;

typedef unsigned char u8;

extern void F46b43b72_FUN_002B7B18(int value, char *target);

extern void F46b43b72_FUN_002B7CE0(int value);

extern int F46b43b72_D_001B8840[];

extern int F46b43b72_D_0014B540[];

struct Slot { s32 w; s32 rest[4]; };

struct Table1 { char pad[19264]; struct Slot slots[48]; };

struct Table2 { char pad[52]; s32 slots[4]; };

extern struct Table1 Fee2b87d1_D_0014B540;

extern struct Table2 Fee2b87d1_D_00152CD0;

void func_00291758(unsigned long long param_1)
{
    func_00291788(param_1);
    func_00291640(param_1);
}

INCLUDE_ASM("asm/game", func_00291788);

INCLUDE_ASM("asm/game", func_00291850);

INCLUDE_ASM("asm/game", func_00291CA0);

INCLUDE_ASM("asm/game", func_00291CA8);

INCLUDE_ASM("asm/game", func_00291DE0);

INCLUDE_ASM("asm/game", func_00291EC0);

INCLUDE_ASM("asm/game", func_00292078);

INCLUDE_ASM("asm/game", func_00292328);

INCLUDE_ASM("asm/game", func_002923C0);

INCLUDE_ASM("asm/game", func_00292408);

INCLUDE_ASM("asm/game", func_002925E0);

INCLUDE_ASM("asm/game", func_002925F0);

INCLUDE_ASM("asm/game", func_002926B8);

INCLUDE_ASM("asm/game", func_00292888);

INCLUDE_ASM("asm/game", func_00292B20);

INCLUDE_ASM("asm/game", func_00292B88);

INCLUDE_ASM("asm/game", func_00292EB0);

INCLUDE_ASM("asm/game", func_00293000);

void func_00293260(int *a0, int a1){
  BatchA_A02_Blk *r;
  int i;
  a0 += a1;
  r = *(BatchA_A02_Blk **)((char *)a0 + 0x48);
  if (r->off) r->off = (int)r + r->off;
  for (i = 0; i < r->n; i++) r->ent[i] = (int)r + r->ent[i];
}

INCLUDE_ASM("asm/game", func_002932B8);

INCLUDE_ASM("asm/game", func_002933D0);

void func_002934B8(u8 *a0, u8 *a1) {
    *(u8 *)(a0 + 4) = *(u8 *)(a1 + 0);
    *(u8 *)(a0 + 5) = *(u8 *)(a1 + 1);
    *(u8 *)(a0 + 6) = *(u8 *)(a1 + 2);
    *(u8 *)(a0 + 7) = *(u8 *)(a1 + 3);
    *(u8 **)(a0 + 0x0) = a1 + *(s32 *)(a1 + 4);
    *(u8 **)(a0 + 0x20) = a1 + *(s32 *)(a1 + 8);
}

INCLUDE_ASM("asm/game", func_002934F8);

INCLUDE_ASM("asm/game", func_00293890);

INCLUDE_ASM("asm/game", func_002939B8);

INCLUDE_ASM("asm/game", func_002939D0);

INCLUDE_ASM("asm/game", func_00293A58);

INCLUDE_ASM("asm/game", func_00293A60);

INCLUDE_ASM("asm/game", func_00293CA0);

int func_00293CA8(int index)
{
    int j = index + 1;
    int *d = F46b43b72_D_001B8840;
    int *b = F46b43b72_D_0014B540;
    int *t = (int *)((char *)b + 25416);
    int n = d[12] * 332;
    int hold = d[28];
    int j4 = j * 4;
    int cur = *(int *)((char *)t + (n + index * 4));
    int next = *(int *)((char *)t + (j4 + n));

    n = next - cur;
    if (n > 0) {
        F46b43b72_FUN_002B7B18(hold, (char *)(cur + b[6341]));
        F46b43b72_FUN_002B7CE0(0);
    }
    return 1;
}

INCLUDE_ASM("asm/game", func_00293D30);

INCLUDE_ASM("asm/game", func_00293F98);

INCLUDE_ASM("asm/game", func_00294070);

INCLUDE_ASM("asm/game", func_002940C0);

INCLUDE_ASM("asm/game", func_00294130);

INCLUDE_ASM("asm/game", func_00294180);

INCLUDE_ASM("asm/game", func_002942A0);

INCLUDE_ASM("asm/game", func_00294398);

INCLUDE_ASM("asm/game", func_00294420);

INCLUDE_ASM("asm/game", func_002945E8);

s32 func_00294630(s32 value) {
    s32 i = 0;
    s32 *q;
    if (Fee2b87d1_D_0014B540.slots[0].w == value) goto after1;
    while (++i < 48) {
        if (Fee2b87d1_D_0014B540.slots[i].w == value) break;
    }
after1:
    if (i == 48) return 1;
    value = 0;
    if (Fee2b87d1_D_00152CD0.slots[0] == i) goto after2;
    while (++value < 3) {
        if (Fee2b87d1_D_00152CD0.slots[value] == i) break;
    }
after2:
    return value != 3;
}

INCLUDE_ASM("asm/game", func_002946C0);

INCLUDE_ASM("asm/game", func_00294988);
