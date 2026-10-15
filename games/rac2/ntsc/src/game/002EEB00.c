#include "include_asm.h"

extern int func_002ED688();

extern int func_002EE6A8();

extern float func_00282E48_4();

extern int func_00282948_1();

/* Append one 16-byte record to the level's queue at 0x226150 and submit it
   through the DMA helper func_0011AFE0.  The four queue globals live in a
   0x38-byte state block based at 0x1A7240; because they are reached with a
   constant offset from that base (a CONST address), cc1 gives the store a
   two-instruction length and refuses to drop it into the branch delay slot,
   which is what the retail body does as well.  */

typedef struct {
    int start;      /* +0x00 -> 0x1A7240 */
    int end;        /* +0x04 -> 0x1A7244 */
    int pad[10];    /* +0x08 .. +0x2F */
    int cursor;     /* +0x30 -> 0x1A7270 */
    int count;      /* +0x34 -> 0x1A7274 */
} LevelQueue5a29d35d;

extern LevelQueue5a29d35d D_001A7240 __attribute__((sda));

extern char D_L03_00226150[];

extern int func_0011AFE0(int *dma, int flag);

/* v2: guard reads p->count, loop counter is a second read (CSE -> copy). */
struct Elem {
    unsigned int f0;
    unsigned char pad0[31];
    unsigned char f35;
    unsigned char pad1[12];
    unsigned int f48;
    unsigned char pad2[28];
};

struct Node {
    struct Elem *elems;
    int count;
};

extern struct Node D_L05_00226A00 __attribute__((nosda));

extern short D_L05_00226700[][2];

/* Apply nonzero signed halfword overrides to indexed rows of selected objects. */
typedef struct {
    unsigned int first;
    unsigned char reserved04[0x1c];
    unsigned int second;
    unsigned char reserved24[0x0f];
    unsigned char key;
    unsigned char reserved34[0x1c];
} ListOverrideRow;

typedef struct {
    unsigned char reserved00[0x0f];
    unsigned char count;
    unsigned char reserved10[0x0c];
    ListOverrideRow *rows;
} ListOverrideObject;

typedef struct { short first; short second; } ListOverridePair;

extern int D_L00_0022DC00[];

extern ListOverrideObject *D_L00_00226700[];

extern ListOverridePair D_L00_0022D600[];

/* Picks a float threshold (1.0 or 8.0) from a callee status code, compares a probed float against it, and returns a boolean that also latches a trailing probe. */
int func_002EEB00(int *a0)
{
    int s2 = 0;
    int s1 = func_002ED688(a0);
    float f20 = 1.0f;

    if (func_002EE6A8(a0) == 2)
        f20 = 8.0f;

    if (s1 != 0) {
        if (func_00282E48_4((char *)a0 + 16, (char *)s1 + 96) < f20)
            s2 = func_002EE6A8(a0) != 8;

        s2 = func_00282948_1((char *)s1 + 304) != 0 ? 1 : s2;
    }

    return s2;
}

INCLUDE_ASM("asm/game", func_002EEBB0);

INCLUDE_ASM("asm/game", func_002EEBC8);

void func_002EED08(void)
{
    int i;
    D_001A7240.cursor = D_001A7240.start;
    D_001A7240.count = 0;
    for (i = 0; i < 64; i++) {
        *(int *)(D_L03_00226150 + i * 16) = 0;
        *(int *)(D_L03_00226150 + i * 16 + 4) = 0;
    }
}

/* shared native body of family 5a29d35dd7a7809a */
int func_002EED58(int p0, int p1, int p2, int p3)
{
    int args[4];
    int n, k;

    if (D_001A7240.end - (D_001A7240.cursor - D_001A7240.start) < p2 * 16)
        return -1;
    if (D_001A7240.count == 64)
        return -2;

    args[0] = p0;
    args[1] = D_001A7240.cursor;
    args[2] = p1 * 16;
    args[3] = 0;
    func_0011AFE0(args, 1);

    n = D_001A7240.count;
    k = n;
    n = n + 1;
    D_001A7240.count = n;
    *(int *)(D_L03_00226150 + k * 16) = D_001A7240.cursor;
    *(int *)(D_L03_00226150 + k * 16 + 4) = p2;
    *(int *)(D_L03_00226150 + k * 16 + 8) = p3;
    D_001A7240.cursor = D_001A7240.cursor + p2 * 16;
    return k;
}

typedef struct { int address, count, reserved[2]; } TransferRow16;
extern TransferRow16 D_L00_002254D0[];

int func_002EEE50(int tag, unsigned int index, int start, int count)
{
    int request[4];
    if (index >= 64 || D_L00_002254D0[index].count == 0)
        return -3;
    if (D_L00_002254D0[index].count < start + count)
        return -1;
    request[0] = tag;
    request[1] = D_L00_002254D0[index].address + start * 16;
    request[2] = count * 16;
    request[3] = 0;
    func_0011AFE0(request, 1);
    return 0;
}

INCLUDE_ASM("asm/game", func_002EEED0);

typedef struct StashBlock_r002EEFE0 {
    /* 0x00 */ int   ram;      /* IOP stash RAM address */
    /* 0x04 */ int   qwc;      /* quadword count (16 bytes per unit) */
    /* 0x08 */ int   comment;  /* char* debug comment / tag */
    /* 0x0C */ int   pad;
} StashBlock_r002EEFE0;

extern StashBlock_r002EEFE0 r1_D_001DD568[];

extern int func_002EEFE0(unsigned int);

/* rac1-decomp src/game/stash.c: func_00234350 */
int func_002EEFE0(unsigned int arg0) {
    if (arg0 >= 0x40) {
        return -3;
    }
    return r1_D_001DD568[arg0].qwc;
}

INCLUDE_ASM("asm/game", func_002EF010);

INCLUDE_ASM("asm/game", func_002EF080);

INCLUDE_ASM("asm/game", func_002EF1E8);

/* shared native body of family 9328256ba62fe2c4 */
void func_002EF310(void)
{
    struct Node *p = &D_L05_00226A00;

    if (p->elems != 0) {
        do {
            struct Elem *e = p->elems;

            if (p->count > 0) {
                int n = p->count;

                do {
                    short *t = D_L05_00226700[e->f35];
                    if (t[0] != 0) e->f0 = (e->f0 & 0xFFFFC000u) | (unsigned int)t[0];
                    if (t[1] != 0) e->f48 = (e->f48 & 0xFFFFC000u) | (unsigned int)t[1];
                    e++;
                } while (--n);
            }
            p++;
        } while (p->elems != 0);
    }
}

INCLUDE_ASM("asm/game", func_002EF3B0);

ASM_FUNC("asm/game", func_002EF480);

ASM_FUNC("asm/game", func_002F00A8);

ASM_FUNC("asm/game", func_002F0218);

INCLUDE_ASM("asm/game", func_002F0468);

INCLUDE_ASM("asm/game", func_002F0B78);

INCLUDE_ASM("asm/game", func_002F0D00);

void func_002F0E08(void)
{
    extern int D_001A72D0 __attribute__((sda));
    extern void func_002F3300(int);
    extern void func_002FC748(void);
    func_002F3300(D_001A72D0);
    func_002FC748();
}

/* apply nonzero signed halfword overrides to selected object rows; observed count0F, rows1C, stride50 and key33 */
void func_002F0E38(void)
{
    int *selected = D_L00_0022DC00;
    while (*selected >= 0) {
        ListOverrideObject *object = D_L00_00226700[*selected];
        int i = 0;
        ListOverrideRow *row = object->rows;
        while (i < object->count) {
            ListOverridePair *pair = &D_L00_0022D600[row->key];
            if (pair->first != 0)
                row->first = (row->first & 0xffffc000u) | pair->first;
            if (pair->second != 0)
                row->second = (row->second & 0xffffc000u) | pair->second;
            i++;
            row++;
        }
        selected++;
    }
}

INCLUDE_ASM("asm/game", func_002F0EF8);

INCLUDE_ASM("asm/game", func_002F0F90);

INCLUDE_ASM("asm/game", func_002F0FA0);

INCLUDE_ASM("asm/game", func_002F1010);

INCLUDE_ASM("asm/game", func_002F1E60);

INCLUDE_ASM("asm/game", func_002F1E88);

INCLUDE_ASM("asm/game", func_002F2610);

INCLUDE_ASM("asm/game", func_002F2758);

INCLUDE_ASM("asm/game", func_002F29E4);

INCLUDE_ASM("asm/game", func_002F2A0C);

INCLUDE_ASM("asm/game", func_002F2A38);

INCLUDE_ASM("asm/game", func_002F32AC);

INCLUDE_ASM("asm/game", func_002F32D4);

INCLUDE_ASM("asm/game", func_002F3300);

INCLUDE_ASM("asm/game", func_002F35B0);

INCLUDE_ASM("asm/game", func_002F3CF8);

INCLUDE_ASM("asm/game", func_002F3D20);

INCLUDE_ASM("asm/game", func_002F3E40);

INCLUDE_ASM("asm/game", func_002F3E78);

INCLUDE_ASM("asm/game", func_002F4F78);

INCLUDE_ASM("asm/game", func_002F4FA0);

INCLUDE_ASM("asm/game", func_002F5118);
