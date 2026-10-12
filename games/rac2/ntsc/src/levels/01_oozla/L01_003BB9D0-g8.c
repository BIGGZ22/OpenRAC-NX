#include "include_asm.h"

extern int func_001163B0(void);

extern int func_002A7820(int, int);

extern void func_002A7750(void *, int, int);

extern void func_L01_003BFDC8(void *, int, int, int);

extern void func_00282978(void *);

extern void func_002AB8C0(void *, void *);

extern void func_002B0990(void *, void *, int, int, float);

/* Small data reached through $gp: these are only one instruction at -G8. */
extern unsigned short D_L01_001AFED4 __attribute__((sda));

extern unsigned short D_L01_001AFED8 __attribute__((sda));

extern unsigned short D_L01_001AFEDC __attribute__((sda));

extern float D_L01_001AFEFC __attribute__((sda));

typedef struct L01_003C0760_OWNED {
    char pad0[0x2DC];
    unsigned short f2DC;
} L01_003C0760_OWNED;

typedef struct L01_003C0760_OBJ {
    char pad0[0x68];
    L01_003C0760_OWNED *f68;
} L01_003C0760_OBJ;

void func_L01_003C0760(L01_003C0760_OBJ *a0)
{
    L01_003C0760_OBJ *p = a0;
    L01_003C0760_OWNED *q = p->f68;
    int r = func_002A7820(7, 12);

    func_L01_003BFDC8(p, 8, 0, r);
    func_002A7750(p, 3, 3);
    q->f2DC = D_L01_001AFED4;
}

typedef struct L01_003C07C8_OWNED {
    char pad0[0x2DA];
    unsigned short f2DA;
} L01_003C07C8_OWNED;

typedef struct L01_003C07C8_OBJ {
    char pad0[0x68];
    L01_003C07C8_OWNED *f68;
} L01_003C07C8_OBJ;

void func_L01_003C07C8(L01_003C07C8_OBJ *a0)
{
    L01_003C07C8_OBJ *p = a0;
    L01_003C07C8_OWNED *q = p->f68;
    int r = func_002A7820(9, 11);

    func_L01_003BFDC8(p, 13, 0, r);
    func_002A7750(p, 2, 3);
    q->f2DA = D_L01_001AFED8;
}

typedef struct L01_003C0830_OWNED {
    char pad0[0x2DE];
    unsigned short f2DE;
} L01_003C0830_OWNED;

typedef struct L01_003C0830_OBJ {
    char pad0[0x68];
    L01_003C0830_OWNED *f68;
    char pad1[0x9C - 0x6C];
    int f9C;
} L01_003C0830_OBJ;

void func_L01_003C0830(L01_003C0830_OBJ *a0)
{
    L01_003C0830_OBJ *p = a0;
    L01_003C0830_OWNED *q = p->f68;
    int r = func_002A7820(7, 11);

    func_L01_003BFDC8(p, 14, 0, r);
    func_002A7750(p, 4, 3);
    p->f9C = p->f9C | 0x30;
    q->f2DE = D_L01_001AFEDC;
}

typedef struct L01_003C06E8_OWNED {
    char pad0[0x2DC];
    unsigned short f2DC;
} L01_003C06E8_OWNED;

typedef struct L01_003C06E8_OBJ {
    char pad0[0x68];
    L01_003C06E8_OWNED *f68;
} L01_003C06E8_OBJ;

void func_L01_003C06E8(L01_003C06E8_OBJ *a0)
{
    L01_003C06E8_OBJ *p = a0;
    L01_003C06E8_OWNED *q = p->f68;
    int v = (func_001163B0() & 1) + 6;
    int r = func_002A7820(10, 15);

    func_L01_003BFDC8(p, v, 0, r);
    func_002A7750(p, 1, 3);
    q->f2DC = D_L01_001AFED4;
}

typedef struct L01_003C0140_OWNED {
    char pad0[0x68];
    void *f68;
} L01_003C0140_OWNED;

typedef struct L01_003C0140_OBJ {
    char pad0[0x68];
    L01_003C0140_OWNED *f68;
    char pad1[0x96 - 0x6C];
    unsigned short f96;
} L01_003C0140_OBJ;

void func_L01_003C0140(L01_003C0140_OBJ *a0)
{
    L01_003C0140_OBJ *p = a0;
    L01_003C0140_OWNED *q = p->f68;

    func_002AB8C0(a0, (char *)q + 0x50);
    if (p->f96 <= 0xFFFE) {
        p->f96 = p->f96 + 1;
    }
    func_00282978((char *)q + 0x2DE);
    func_00282978((char *)q + 0x2DA);
    func_00282978((char *)q + 0x2DC);
    func_002B0990(p, (char *)q + 0x220, 10, 1, D_L01_001AFEFC);
}
