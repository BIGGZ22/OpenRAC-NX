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

extern void func_00292078(void *);
extern void func_00292328(void *);
extern u8 *D_L03_001DAB80[];
extern void func_002934B8(u8 *, u8 *);
extern unsigned char *D_001BAF38;

void func_002923C0(void *param)
{
    func_00292078(param);
    func_00292328(param);
    func_002934B8((u8 *)D_L03_001DAB80[0], D_001BAF38);
    *(int *)0x001A7290 = (int)param;
}


INCLUDE_ASM("asm/game", func_00292408);

INCLUDE_ASM("asm/game", func_002925E0);

INCLUDE_ASM("asm/game", func_002925F0);

INCLUDE_ASM("asm/game", func_002926B8);

INCLUDE_ASM("asm/game", func_00292888);

INCLUDE_ASM("asm/game", func_00292B20);

INCLUDE_ASM("asm/game", func_00292B88);

extern long r1_D_0019E7C0[];

extern long r1_D_0019E640[];

extern long r1_D_0019E7D8[];

/* rac1-decomp src/game/loaders.c: func_00203808 */
/* Fills four A+D qwords (low dwords only) at out: a TEX1-style word
   (MXL bits from the second word of row arg5 of the 3-dword table
   r1_D_0019E640, MMAG 1, MMIN arg2, K arg1), then arg3 | arg4 << 2 |
   arg5 << 24, then the row's first and third words. A negative arg5 has
   no row: -2 and -3 take the rows r1_D_0019E7C0 and r1_D_0019E7D8 (and 5 for
   the second word), -1 a fixed constant and 0. The row is read before
   the sign test, as in retail. The OR operands are named locals so fold
   does not move the 0x20 to the end of the chain. */
void func_00292EB0(long *out, int arg1, int arg2, int arg3, int arg4, int arg5) {
    long f0 = r1_D_0019E640[arg5 * 3];
    long f1 = r1_D_0019E640[arg5 * 3 + 1];
    long f2 = r1_D_0019E640[arg5 * 3 + 2];

    if (arg5 >= 0) {
        long t = ((long)arg2 << 6) | 0x20;

        *out = (f1 & 0x1C) | t | ((long)arg1 << 32);
        out += 2;
        *out = arg3 | ((long)arg4 << 2) | ((long)arg5 << 24);
        out += 2;
        *out = f0;
        out[2] = f2;
    } else if (arg5 < -1) {
        long *tpl = r1_D_0019E7C0;

        if (arg5 == -3) {
            tpl = r1_D_0019E7D8;
        }
        {
            long v = (long)arg2 << 6;
            long u = ((long)arg1 << 32) | 0x20;
            *out = v | u;
        }
        out += 2;
        *out = 5;
        out += 2;
        *out = tpl[0];
        out[2] = tpl[2];
    } else {
        {
            long v = (long)arg2 << 6;
            long u = ((long)arg1 << 32) | 0x20;
            *out = v | u;
        }
        out += 2;
        *out = 5;
        out += 2;
        *out = 0x80000004CC007FFBL;
        out[2] = 0;
    }
}

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

void func_002940C0(void)
{
    extern void func_00282A60(unsigned int *, unsigned int, int);
    struct Table2 *table = &Fee2b87d1_D_00152CD0;
    int i;
    func_00282A60((unsigned int *)table, 0, 0x35840);
    *(short *)((char *)table + 0x14) = -1;
    for (i = 2; i >= 0; --i)
        table->slots[i] = -1;
}


INCLUDE_ASM("asm/game", func_00294130);

INCLUDE_ASM("asm/game", func_00294180);

INCLUDE_ASM("asm/game", func_002942A0);

INCLUDE_ASM("asm/game", func_00294398);

INCLUDE_ASM("asm/game", func_00294420);

extern void func_00294398(void *a0, void *a1);

void func_002945E8(void *a0, void *a1)
{
    F46b43b72_FUN_002B7CE0(1);
    func_00294398(a0, a1);
    F46b43b72_FUN_002B7CE0(1);
}

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
