#include "include_asm.h"

typedef unsigned int u32;

typedef int s32;

extern s32 D_L24_001B1B70[] __attribute__((sda));

extern u32 D_L24_001B1B74[] __attribute__((sda));

typedef unsigned char u8;

INCLUDE_ASM("asm/game", func_002DDEC8);

INCLUDE_ASM("asm/game", func_002DE0D0);

/* The chunk table: a key and its flags, one pair per chunk. */
typedef struct {
    u32 key;
    u32 flags;
} Chunk;

extern int D_001B1F50[] __attribute__((sda));
extern s32 D_001F2840[];

/* Fills the chunk table at D_001B1F50 with (key, flags) pairs: the keys step
   by 0x11800 for the small chunks and by 0x4F000 for the large ones, and the
   flags say which. With a parameter it lays out four groups of chunks, without
   one only the first; the rest of the table is cleared. The retail function
   returns nothing, so it is void here (002D5E98.c declares it int and drops
   the result). */
void func_002DE3A0(s32 param)
{
    s32 *state = D_001F2840;
    u32 key1 = state[0x118 / 4];
    u32 key2 = state[0x11C / 4];
    s32 n1, n2, n3, n4;
    s32 idx, lim;

    if (!param) {
        n1 = 1;
        n2 = 0;
        n3 = 0;
        n4 = 0;
    } else {
        n1 = 2;
        n2 = 1;
        n3 = 2;
        n4 = 1;
    }
    idx = 0;
    lim = n1;
    while (idx < lim) {
        ((Chunk *)D_001B1F50)[idx].key = key1;
        ((Chunk *)D_001B1F50)[idx].flags = 0;
        key1 += 0x11800;
        idx++;
    }
    lim += n2;
    while (idx < lim) {
        ((Chunk *)D_001B1F50)[idx].key = key2;
        ((Chunk *)D_001B1F50)[idx].flags = 0;
        key2 += 0x11800;
        idx++;
    }
    lim += n3;
    while (idx < lim) {
        ((Chunk *)D_001B1F50)[idx].key = key1;
        ((Chunk *)D_001B1F50)[idx].flags = 1;
        key1 += 0x4F000;
        idx++;
    }
    lim += n4;
    while (idx < lim) {
        ((Chunk *)D_001B1F50)[idx].key = key2;
        ((Chunk *)D_001B1F50)[idx].flags = 1;
        key2 += 0x4F000;
        idx++;
    }
    while (idx < 5) {
        ((Chunk *)D_001B1F50)[idx].flags = 0;
        ((Chunk *)D_001B1F50)[idx].key = 0;
        idx++;
    }
}

INCLUDE_ASM("asm/game", func_002DE550);

INCLUDE_ASM("asm/game", func_002DE618);

extern int D_001B1F50[] __attribute__((sda));
extern int D_001B1F54[] __attribute__((sda));

int func_002DE6F0(int key)
{
    int count = 0;
    unsigned int large = 0x4F000;
    int *flags = D_001B1F54;
    int *keys = D_001B1F50;
    do {
        count++;
        if (*keys == key)
            return (*flags & 1) ? large : 0x11800;
        flags += 2;
        keys += 2;
    } while (count < 5);
    return -1;
}

/* authored C for independently verified level-native body */
s32 func_002DE750(s32 key) {
 s32 count=0;
 u32 *flags=D_L24_001B1B74;
 s32 *keys=D_L24_001B1B70;
 do {
  count++;
  if(*keys!=key) {flags+=2;keys+=2;continue;}
  *flags|=4;return 0;
 }while(count<5);
 return 1;
}

int func_002DE7A0(int key)
{
    int count = 0;
    int mask = -5;
    int *flags = D_001B1F54;
    int *keys = D_001B1F50;
    do {
        count++;
        if (*keys != key) {
            flags += 2;
            keys += 2;
            continue;
        }
        *flags &= mask;
        return 0;
    } while (count < 5);
    return 1;
}

INCLUDE_ASM("asm/game", func_002DE7F0);

void func_002DE810(unsigned short *a0,u8 *a1) {
 a0[0]=0;
 a0[1]=*(unsigned short *)(a1+0x24);
 a0[2]=0;
 a0[3]=*(unsigned short *)(a1+0x20);
 a0[4]=*(s32 *)(a1+0x20)>>1;
 a0[5]=*(s32 *)(a1+0x24)>>1;
 a0[8]=0x10;a0[9]=0;
}

INCLUDE_ASM("asm/game", func_002DE850);

typedef struct {
    char b[8];
} CdClock;

extern int func_001256D8(CdClock *);
extern void func_00131A98(unsigned char *);
extern void func_002980D0(void);
extern void func_00297670(void *);
extern void func_0029ADA0(int);
extern unsigned char D_001A7360[8] __attribute__((sda));
extern unsigned char D_0018D278[];
extern int D_001A79F0 __attribute__((sda));
extern unsigned char D_001393E0[];
extern void func_00298E08(int, int);

void func_002DE858(int value, int mode)
{
    unsigned char *state = D_001393E0;
    func_001256D8((CdClock *)D_001A7360);
    func_00131A98(D_001A7360);
    func_002980D0();
    func_00297670(D_0018D278 + (D_001A79F0 << 11));
    func_0029ADA0(value);
    *(int *)(state + 0x174) = value;
    *(short *)(state + 0x18) = mode;
    *(int *)(state + 0x148) = 0;
    if (*(int *)(state + 0x164) < 0) {
        *(int *)(state + 0x168) = 0;
        *(int *)(state + 0x164) = 19;
    }
}

void func_002DE900(int value, int mode)
{
    unsigned char *state;
    func_00298E08(value, mode);
    func_001256D8((CdClock *)D_001A7360);
    func_00131A98(D_001A7360);
    func_0029ADA0(value);
    state = D_001393E0;
    *(int *)(state + 0x148) = 0;
    *(short *)(state + 0x18) = mode;
    *(int *)(state + mode * 0x1C + 0x30) = 0;
    *(int *)(state + 0x174) = value;
    if (*(int *)(state + 0x164) < 0) {
        *(int *)(state + 0x168) = 0;
        *(int *)(state + 0x164) = 0x13;
    }
}

INCLUDE_ASM("asm/game", func_002DE990);

INCLUDE_ASM("asm/game", func_002DF050);

void func_002DF158(s32 object, s32 amount)
{
    extern s32 D_001F2840[];
    extern void func_002A0300(s32, s32);
    if (D_001F2840[0x168 / 4] != 0) {
        D_001F2840[0x16C / 4] += amount;
        func_002A0300(object, amount);
    }
}

/* func_002DF190 is matched in 002DDEC8-g8.c (-G8 small data). */
