#include "include_asm.h"

void func_00288B38(int);

void func_00288B18(void*);

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

/* Ordinary integer controller and queue-launch algorithm; static proposals. */
typedef struct {
    u8 prefix[0x30];
    s32 field30;
    s32 field34;
    s32 field38;
    s32 field3c;
    s32 field40;
    s32 field44;
} BarlowUtilityQueueView;

extern s32 D_L04_001A8F00 __attribute__((sda));

extern s32 func_00288A00(BarlowUtilityQueueView *, s32 *, s32 *);

extern s32 func_002B49E0(s32, s32, u32, u32, u8 *);

extern void func_L00_002F4180(BarlowUtilityQueueView *);

void func_00288BB8(void *param)
{
  func_00288B38((int)param);
  *(unsigned int *)param = 0;
  func_00288B18(param);
}
extern s32 func_00288BE8(BarlowUtilityQueueView *queue);