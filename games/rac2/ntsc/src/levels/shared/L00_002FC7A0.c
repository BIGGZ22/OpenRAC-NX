#include "include_asm.h"

extern void func_002B7B18(int value, char *target);

extern void func_002B7CE0(int value);

extern int D_L00_001BCAC0[];

extern int D_0014B540[];

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_3e18d05650488ad4_FallbackU32;

extern Rac2Native_3e18d05650488ad4_FallbackU32 D_001A7424;

extern int D_001A7420_1;

extern Rac2Native_3e18d05650488ad4_FallbackU32 D_001A7424_1;

extern int D_001A7420_2;

extern Rac2Native_3e18d05650488ad4_FallbackU32 D_001A7424_2;

extern int D_001A7420_3;

extern Rac2Native_3e18d05650488ad4_FallbackU32 D_001A7424_3;

extern int D_001A7420_4;

extern int D_L00_001A904C_1 __attribute__((sda));

extern void func_0034CE28(int value, int argument);

extern void func_L00_004427C8(int value, int argument);

extern void func_L00_004427D0(int value, int argument);

extern int D_L00_001A904C_2 __attribute__((sda));

extern void func_L00_00443DF8(int);

extern void func_L00_00443E18(int);

extern void func_0034E478(int);

extern void func_0034E4F8(int);

extern int func_003418C0(int a, int b);

extern void func_0034CE98(int value, int argument);

extern int D_L00_001A904C_7 __attribute__((sda));

extern int func_003387A0_1(int a, int b, int c);

extern int func_00338040_1(int a, int b, int c);

extern void func_L00_0042D9D0(int);

extern unsigned char *D_L05_001B2520 __attribute__((sda));

extern unsigned char *D_L05_001B2524 __attribute__((sda));

extern unsigned int D_L05_001B2048 __attribute__((sda));

extern unsigned int D_L05_001B2440 __attribute__((sda));

/* RAC2 family 0820eb1123250c0b - 192 bytes, 2 placements.
 * Slot0820eb11 allocator: find the first free of six 64-byte slots, fill it in and
 * link it at the head of the context's list.
 */

typedef struct Slot0820eb11 {
    short          f00;    /* +0x00 */
    short          f02;    /* +0x02 */
    unsigned char  f04;    /* +0x04 */
    char           pad05[7];
    unsigned char *f0C;    /* +0x0C */
    int            f10;    /* +0x10 */
    int            f14;    /* +0x14 */
    int            f18;    /* +0x18 */
    int            f1C;    /* +0x1C */
    char           pad20[32];
} Slot0820eb11;

                    /* 64 bytes */

typedef struct Mid {
    char           pad00[0x1C];
    char          *f1C;    /* +0x1C */
} Mid;

typedef struct Ctx {
    char           pad00[0x24];
    Mid           *f24;    /* +0x24 */
    char           pad28[0x28];
    int            f50;    /* +0x50 */
} Ctx;

extern Slot0820eb11 D_L03_001D23C0[6];

extern unsigned char D_L03_001CA5C0[];

extern void func_00283658_1(char *a, char *b);

extern void func_002A3F10_3(char *a, int b, int c, char *d);

extern void func_00283788_5(char *a, char *b, char *c);

extern void func_00282D30_21(char *a, char *b, float c);

extern void func_00282CC0_24(char *a, char *b, char *c);

void func_L00_002FC7A0(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002FC980);

INCLUDE_ASM("asm/levels", func_L00_002FCF18);

INCLUDE_ASM("asm/levels", func_L00_002FD308);

INCLUDE_ASM("asm/levels", func_L00_002FD400);

INCLUDE_ASM("asm/levels", func_L00_002FD480);

INCLUDE_ASM("asm/levels", func_L00_002FD928);

INCLUDE_ASM("asm/levels", func_L00_002FE268);

INCLUDE_ASM("asm/levels", func_L00_002FE2D8);

INCLUDE_ASM("asm/levels", func_L00_002FF6A8);

INCLUDE_ASM("asm/levels", func_L00_002FFAB8);

INCLUDE_ASM("asm/levels", func_L00_00300090);

INCLUDE_ASM("asm/levels", func_L00_00300378);

INCLUDE_ASM("asm/levels", func_L00_00300438);

void func_L00_00300610(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00300618);

INCLUDE_ASM("asm/levels", func_L00_003032F0);

INCLUDE_ASM("asm/levels", func_L00_00303388);

INCLUDE_ASM("asm/levels", func_L00_00303418);

INCLUDE_ASM("asm/levels", func_L00_00303530);

INCLUDE_ASM("asm/levels", func_L00_00303990);

/* Reads a record count from a per-level data root, indexes a fixed 332-byte-stride table with it, and when the int after position index is greater than the one at position index calls two helpers with the entry value added to a per-level base; returns 1. */
int func_L00_003046D0(int index)
{
    int j = index + 1;
    int *d = D_L00_001BCAC0;
    int *b = D_0014B540;
    int *t = (int *)((char *)b + 25416);
    int n = d[12] * 332;
    int hold = d[28];
    int j4 = j * 4;
    int cur = *(int *)((char *)t + (n + index * 4));
    int next = *(int *)((char *)t + (j4 + n));

    n = next - cur;
    if (n > 0) {
        func_002B7B18(hold, (char *)(cur + b[6341]));
        func_002B7CE0(0);
    }
    return 1;
}

INCLUDE_ASM("asm/levels", func_L00_00304AE8);

INCLUDE_ASM("asm/levels", func_L00_00305600);

INCLUDE_ASM("asm/levels", func_L00_00306648);

INCLUDE_ASM("asm/levels", func_L00_003080D0);

INCLUDE_ASM("asm/levels", func_L00_00308408);

INCLUDE_ASM("asm/levels", func_L00_00308538);

INCLUDE_ASM("asm/levels", func_L00_00308700);

INCLUDE_ASM("asm/levels", func_L00_00309140);

void func_L00_00309368(void)
{
}
extern void func_L00_00309AE8(void);
INCLUDE_ASM("asm/levels", func_L00_00309AE8);

extern void func_L00_00309B10(void);
INCLUDE_ASM("asm/levels", func_L00_00309B10);

extern void func_L00_00309B58(void);
INCLUDE_ASM("asm/levels", func_L00_00309B58);

extern void func_L00_00309B80(void);
INCLUDE_ASM("asm/levels", func_L00_00309B80);


INCLUDE_ASM("asm/levels", func_L00_00309DB0);

/* Call the registered handler with a table pointer built from one module base word plus the constant 36808, forwarding the incoming argument; do nothing when the base word is zero. */
void func_L00_0030C710(int argument)
{
    int value = D_L00_001A904C_1;

    if (value != 0)
        func_0034CE28(value + 0x376C8, argument);
}

INCLUDE_ASM("asm/levels", func_L00_0030C748);

/* Call the registered handler with a table pointer built from one module base word plus the constant 36808, forwarding the incoming argument; do nothing when the base word is zero. */
void func_L00_0030C788(int argument)
{
    int value = D_L00_001A904C_1;

    if (value != 0)
        func_L00_004427C8(value + 0x376C8, argument);
}

/* Call the registered handler with a table pointer built from one module base word plus the constant 36808, forwarding the incoming argument; do nothing when the base word is zero. */
void func_L00_0030C7C0(int argument)
{
    int value = D_L00_001A904C_1;

    if (value != 0)
        func_L00_004427D0(value + 0x376C8, argument);
}

INCLUDE_ASM("asm/levels", func_L00_0030C7F8);

/* Reads a word from a shared global; when it is non-zero, calls a one-argument helper with that word plus a fixed byte offset. */
void func_L00_0030C930(void)
{
    int value = D_L00_001A904C_2;

    if (value != 0)
        func_L00_00443DF8(value + 0x36F28);
}

/* Reads a word from a shared global; when it is non-zero, calls a one-argument helper with that word plus a fixed byte offset. */
void func_L00_0030CF60(void)
{
    int value = D_L00_001A904C_2;

    if (value != 0)
        func_L00_00443E18(value + 0x36F28);
}

void func_L00_0030D1A8(void)
{
}

/* Reads a word from a shared global; when it is non-zero, calls a one-argument helper with that word plus a fixed byte offset. */
void func_L00_0030D1B0(void)
{
    int value = D_L00_001A904C_2;

    if (value != 0)
        func_0034E478(value + 0x36F28);
}

/* Reads a word from a shared global; when it is non-zero, calls a one-argument helper with that word plus a fixed byte offset. */
void func_L00_0030D1E0(void)
{
    int value = D_L00_001A904C_2;

    if (value != 0)
        func_0034E4F8(value + 0x36F28);
}

INCLUDE_ASM("asm/levels", func_L00_0030D210);

INCLUDE_ASM("asm/levels", func_L00_0030D240);

INCLUDE_ASM("asm/levels", func_L00_0030D280);

INCLUDE_ASM("asm/levels", func_L00_0030D2B0);

INCLUDE_ASM("asm/levels", func_L00_0030D2E0);

INCLUDE_ASM("asm/levels", func_L00_0030D368);

INCLUDE_ASM("asm/levels", func_L00_0030D3A8);

INCLUDE_ASM("asm/levels", func_L00_0030D3D8);

INCLUDE_ASM("asm/levels", func_L00_0030D418);

INCLUDE_ASM("asm/levels", func_L00_0030D448);

INCLUDE_ASM("asm/levels", func_L00_0030D478);

INCLUDE_ASM("asm/levels", func_L00_0030D4B8);

INCLUDE_ASM("asm/levels", func_L00_0030D4E8);

INCLUDE_ASM("asm/levels", func_L00_0030D528);

INCLUDE_ASM("asm/levels", func_L00_0030D558);

INCLUDE_ASM("asm/levels", func_L00_0030D598);

/* Reads a bound global word; when it is non-zero it calls a helper with that value plus a fixed base offset as the first argument and the function's own argument as the second, otherwise returns 0. */
int func_L00_0030D5C8(int a0)
{
    int v = D_L00_001A904C_1;

    if (v != 0) {
        return func_003418C0(v + 0x3BE08, a0);
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0030D608);

/* Reads a bound global word; when it is non-zero it calls a helper with that value plus a fixed base offset as the first argument and the function's own argument as the second, otherwise returns 0. */
int func_L00_0030D638(int a0)
{
    int v = D_L00_001A904C_1;

    if (v != 0) {
        return func_003418C0(v + 0x3BE08, a0);
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0030D678);

INCLUDE_ASM("asm/levels", func_L00_0030D6A8);

INCLUDE_ASM("asm/levels", func_L00_0030D6E8);

INCLUDE_ASM("asm/levels", func_L00_0030D718);

INCLUDE_ASM("asm/levels", func_L00_0030D758);

INCLUDE_ASM("asm/levels", func_L00_0030D788);

INCLUDE_ASM("asm/levels", func_L00_0030D7C8);

INCLUDE_ASM("asm/levels", func_L00_0030D808);

INCLUDE_ASM("asm/levels", func_L00_0030D838);

INCLUDE_ASM("asm/levels", func_L00_0030D878);

INCLUDE_ASM("asm/levels", func_L00_0030D8A8);

INCLUDE_ASM("asm/levels", func_L00_0030D8E8);

INCLUDE_ASM("asm/levels", func_L00_0030D918);

INCLUDE_ASM("asm/levels", func_L00_0030D958);

INCLUDE_ASM("asm/levels", func_L00_0030D988);

INCLUDE_ASM("asm/levels", func_L00_0030D9C8);

INCLUDE_ASM("asm/levels", func_L00_0030D9F8);

INCLUDE_ASM("asm/levels", func_L00_0030DA38);

INCLUDE_ASM("asm/levels", func_L00_0030DA68);

INCLUDE_ASM("asm/levels", func_L00_0030DAA8);

INCLUDE_ASM("asm/levels", func_L00_0030DAD8);

INCLUDE_ASM("asm/levels", func_L00_0030DB18);

INCLUDE_ASM("asm/levels", func_L00_0030DB48);

INCLUDE_ASM("asm/levels", func_L00_0030DB88);

INCLUDE_ASM("asm/levels", func_L00_0030DBB8);

INCLUDE_ASM("asm/levels", func_L00_0030DBF8);

INCLUDE_ASM("asm/levels", func_L00_0030DC30);

INCLUDE_ASM("asm/levels", func_L00_0030DC58);

/* Call the registered handler with a table pointer built from one module base word plus the constant 36808, forwarding the incoming argument; do nothing when the base word is zero. */
void func_L00_0030DC80(int argument)
{
    int value = D_L00_001A904C_1;

    if (value != 0)
        func_0034CE98(value + 0x376C8, argument);
}

INCLUDE_ASM("asm/levels", func_L00_0030DCB8);

INCLUDE_ASM("asm/levels", func_L00_0030DCF8);

/* Reads a bound global word; when it is zero it returns its first argument, otherwise it calls a helper with that value plus a fixed base offset and both of its arguments. */
int func_L00_0030DD70(int a0, int a1)
{
    int v = D_L00_001A904C_7;

    if (v == 0) return a0;
    return func_003387A0_1(v + 0x36F28, a0, a1);
}

/* Reads a bound global word; when it is zero it returns its first argument, otherwise it calls a helper with that value plus a fixed base offset and both of its arguments. */
int func_L00_0030DDB0(int a0, int a1)
{
    int v = D_L00_001A904C_7;

    if (v == 0) return a0;
    return func_00338040_1(v + 0x36F28, a0, a1);
}

/* Reads a word from a shared global; when it is non-zero, calls a one-argument helper with that word plus a fixed byte offset. */
void func_L00_0030DDF0(void)
{
    int value = D_L00_001A904C_2;

    if (value != 0)
        func_L00_0042D9D0(value + 0x36F28);
}

void func_L00_0030DE20(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_0030DF80);

INCLUDE_ASM("asm/levels", func_L00_0030DFE0);

INCLUDE_ASM("asm/levels", func_L00_0030E010);

/* shared native body of family e7046bc951256c7e */
void func_L00_00310048(void)
{
    unsigned char *p = D_L05_001B2520;
    int seen = 0;
    unsigned int limit;
    D_L05_001B2440 = 0;
    while (p < D_L05_001B2524) {
        limit = D_L05_001B2048;
        do {
            if ((p[32] >= 254 && *(unsigned int *)(p + 160) <= limit) || seen) {
                D_L05_001B2440 = D_L05_001B2440 + 1;
                seen = (p[32] == 0xff) ? 1 : seen;
            }
            p += 256;
        } while (p < D_L05_001B2524);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00310630);

INCLUDE_ASM("asm/levels", func_L00_00310748);

INCLUDE_ASM("asm/levels", func_L00_00310838);

/* shared native body of family 0820eb1123250c0b */
Slot0820eb11 *func_L00_00310BC8(Ctx *ctx, int index)
{
    int i;
    Slot0820eb11 *slot;
    unsigned char *p;

    for (i = 0; i < 6; i++) {
        if (D_L03_001D23C0[i].f04 == 0) {
            break;
        }
    }
    slot = &D_L03_001D23C0[i];
    slot->f04 = 1;
    slot->f00 = index;
    slot->f10 = (int)(D_L03_001CA5C0 + i * 5376);
    slot->f14 = (int)ctx->f24;
    p = (unsigned char *)*(unsigned int *)(ctx->f24->f1C + (short)index * 4 + 4);
    slot->f02 = p[2];
    slot->f0C = p + (p[0] + 4);
    slot->f1C = ctx->f50;
    ctx->f50 = (int)slot;
    return slot;
}

INCLUDE_ASM("asm/levels", func_L00_00310C88);

INCLUDE_ASM("asm/levels", func_L00_00310D30);

INCLUDE_ASM("asm/levels", func_L00_00311080);

INCLUDE_ASM("asm/levels", func_L00_003110E8);

/* scales a float field by 1/1024, fills a 64-byte stack buffer from the object, then applies three per-element helper calls over n 64-byte records. */
void func_L00_003116A8(char *a0, int n, int a2, char *a3)
{
    char buf[64];
    float f20;
    int i;

    f20 = *(float *)(a0 + 44) * 0.0009765625f;
    func_00283658_1(buf, a0 + 192);
    func_002A3F10_3(a0, n, a2, a3);
    for (i = 0; i < n; i++) {
        func_00283788_5(a3 + i * 64, buf, a3 + i * 64);
        func_00282D30_21(a3 + i * 64 + 48, a3 + i * 64 + 48, f20);
        func_00282CC0_24(a3 + i * 64 + 48, a3 + i * 64 + 48, a0 + 16);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00311DC0);

INCLUDE_ASM("asm/levels", func_L00_00314D40);
