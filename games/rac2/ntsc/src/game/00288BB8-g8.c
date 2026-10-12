#include "include_asm.h"

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;

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

/* launches one queued request using mode-dependent word arguments and resets the queue after a negative submission status */
s32 func_00288BE8(BarlowUtilityQueueView *queue) {
    if (queue->field38 != 0) {
        s32 request[2];
        s32 status;
        s32 mode;
        queue->field3c = 0;
        queue->field44 = 1;
        request[0] = 0;
        request[1] = 0;
        func_00288A00(queue, &request[0], &request[1]);
        status = 0;
        mode = 1;
        if ((u32)(D_L04_001A8F00 - 1) < 2) mode = 2;
        queue->field40 = mode;
        if (request[1] == 0)
            status = func_002B49E0(1, mode, 0, request[0], 0);
        else if (request[1] == 1)
            status = func_002B49E0(2, mode, request[0], 0, 0);
        if (status < 0) func_L00_002F4180(queue);
        return status == 0;
    }
    return 0;
}
