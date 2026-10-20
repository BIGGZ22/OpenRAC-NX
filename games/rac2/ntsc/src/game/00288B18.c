#include "include_asm.h"

extern void func_002889B8(unsigned char *);

void func_00288B18(void*);

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

/* The queue the caller passes to func_00288BB8: five slots of eight bytes from offset 8,
   the index the ring ends at at 0x30 and the index it is walked from at 0x34. */
typedef struct {
    u8 pad08[8];
    struct {
        u32 value;
        u32 pad04;
    } slots[5];
    s32 field30;
    s32 field34;
} BarlowUtilityQueueRing;

extern u32 D_00139768[];

void func_00288B18(void *owner)
{
    func_002889B8(owner);
}

/* Walks the ring from its current index to its end index and marks every request below
   0xB1 in the shared table of flags: bit (value & 0x1F) of the word at D_00139768[value >> 2]. */
void func_00288B38(BarlowUtilityQueueRing *ring)
{
    s32 i = ring->field34;

    while (i != ring->field30) {
        u32 value = ring->slots[i].value;

        if (value < 0xB1) {
            D_00139768[value >> 2] |= 1u << (value & 0x1F);
        }

        i = (i + 1 == 5) ? 0 : i + 1;
    }
}
