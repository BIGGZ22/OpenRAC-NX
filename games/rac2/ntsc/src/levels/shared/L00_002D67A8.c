#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

extern u8 D_00189E20_1[];

typedef struct { u8 prefix[0xaa]; short class_id; } ClassFilterObject;

typedef struct {
    u8 prefix[0x33c];
    ClassFilterObject *fallback;
    u8 middle[0x14f0];
    ClassFilterObject *primary;
    u8 trailing[0xa60];
    s32 mode;
} ClassFilterRoot;

typedef int BatchA_A07_s32;

extern BatchA_A07_s32 D_001A79F0[] __attribute__((sda));

extern int func_L00_002D6788(void);

extern int D_0018C0B4_1;

typedef unsigned int u32;

extern u32 D_0018C0B4;

extern u8 D_L24_001B8500[];

/* Measured partial layouts; original type and game roles are unknown. */
struct Rac2Native_5f824a0e369ccf7b_src_record {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned short s4;
    unsigned short s6;
    unsigned short s8;
    unsigned short s10;
    unsigned short s12;
    unsigned char e;
    unsigned char f;
};

struct Rac2Native_5f824a0e369ccf7b_dst_record {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned short s4;
    unsigned short s6;
    unsigned short s8;
    short s10;
    short s12;
    unsigned char e;
    unsigned char f;
};

extern struct Rac2Native_5f824a0e369ccf7b_dst_record D_L04_001B94C0[8] __attribute__((nosda));

typedef struct {
    u8 field0;
    u8 active;
    u8 middle[4];
    unsigned short count;
    u8 trailing[8];
} ConditionalResetSlot;

extern ConditionalResetSlot D_L00_001B8C00[8];

/* Filter primary or fallback object's class identifier by mode and signed selection */
s32 func_L00_002D67A8(s32 selection) {
    s32 found = 0;
    ClassFilterObject *primary = ((ClassFilterRoot *)D_00189E20_1)->primary;
    ClassFilterObject *fallback = ((ClassFilterRoot *)D_00189E20_1)->fallback;
    if (((ClassFilterRoot *)D_00189E20_1)->mode == 0x31) {
        if (primary) {
            switch (selection) {
                case 0: found = primary->class_id == 0xfbf; break;
                case 1: found = primary->class_id == 0x905; break;
                case 2: found = primary->class_id == 0xeef; break;
            }
        } else if (fallback && selection == 3) {
            found = fallback->class_id == 0xc20;
        }
    }
    return found;
}

/* A07-sda-multi-eq-44 freshly exact-verified under family reservation */
BatchA_A07_s32 func_L00_002D6848(void) {
    BatchA_A07_s32 v = D_001A79F0[0];
    return (v == 0x16) | (v == 0x2CE) | (v == 0x17);
}

unsigned int func_L00_002D6878(void)
{
    return 0;
}

/* returns 1 when a global state word is 49 and a global mode word is 4 or 11, otherwise 0; a callee returning nonzero short-circuits the result to 0 */
int func_L00_002D6880(void)
{
    int result = 0;
    int mode;
    if (func_L00_002D6788() != 0) goto out;
    if (D_0018C0B4_1 != 49) goto out;
    mode = *(int *)0x001A79F0;
    if (mode != 4 && mode != 11) goto out;
    result = 1;
out:
    return result;
}

/* returns the resident 32-bit field at core address 0x0018c0b4 */
u32 func_L00_002D68E8(void) {
    return D_0018C0B4;
}

INCLUDE_ASM("asm/levels", func_L00_002D68F8);

INCLUDE_ASM("asm/levels", func_L00_002D6900);

INCLUDE_ASM("asm/levels", func_L00_002D6940);

/* tests the byte at the indexed 16-byte table entry */
s32 func_L00_002D6978(s32 index) {
 return D_L24_001B8500[index*16]!=0;
}

/* Check space among eight destination records, copy bounded input records into inactive slots, adjust copied byte and halfword fields, and return a slot index or -1. */
int func_L00_002D6998(struct Rac2Native_5f824a0e369ccf7b_src_record *src)
{
    int n = 0;
    int i;
    int j;

    while (n < 8 && src[n].b0 != 255)
        n++;

    j = 0;
    for (i = 0; i < 8 && j < n; i++) {
        if (D_L04_001B94C0[i].b0 == 0)
            j++;
    }

    if (j != n)
        return -1;

    for (j = 0; j < n; j++) {
        i = 0;
        while (i < 8 && D_L04_001B94C0[i].b0 != 0)
            i++;
        if (i < 8) {
            D_L04_001B94C0[i].b0 = src[j].b0;
            D_L04_001B94C0[i].b1 = src[j].b1;
            D_L04_001B94C0[i].b2 = src[j].b2;
            D_L04_001B94C0[i].b3 = src[j].b3;
            D_L04_001B94C0[i].s4 = src[j].s4;
            D_L04_001B94C0[i].s6 = src[j].s6;
            D_L04_001B94C0[i].s8 = src[j].s8;
            D_L04_001B94C0[i].s10 = src[j].s10;
            D_L04_001B94C0[i].s12 = src[j].s12;
            D_L04_001B94C0[i].e = src[j].e - src[j].f;
            D_L04_001B94C0[i].f = src[j].f;
            if (D_L04_001B94C0[i].s10 + D_L04_001B94C0[i].s12 == 0)
                D_L04_001B94C0[i].s10++;
        }
    }
    return i;
}

/* For eight16-byte records, clear active byte1 and halfword6 only when active is nonzero */
void func_L00_002D6B70(void) {
    ConditionalResetSlot *slot = D_L00_001B8C00;
    s32 remaining = 7;
    do {
        if (slot->active) {
            slot->active = 0;
            slot->count = 0;
        }
        --remaining;
        ++slot;
    } while (remaining >= 0);
}

INCLUDE_ASM("asm/levels", func_L00_002D6BA8);
