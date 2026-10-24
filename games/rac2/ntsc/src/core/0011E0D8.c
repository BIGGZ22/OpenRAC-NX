#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_func_00118C70(void *);

   /* CreateSema */
extern void r1_func_00118C80(int);

     /* DeleteSema */
extern int r1_func_00118CB0(int);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118C70();

extern s32_1B4C8 r1_func_00118CB0();

extern int r1_D_0012FDA0;

extern char r1_D_00157E80[];

extern int r1_D_0012FD94;

typedef char *va_list_1BF80;

struct FsOpenRequest {
    s32 completion_semaphore;
    void *result;
    s32 result_size;
    s32 flags;
    s32 mode;
    char path[0x400];
    s32 slot;
};

struct SemaphoreParameters {
    s32 count;
    s32 max_count;
    s32 init_count;
    s32 wait_threads;
    s32 attr;
    s32 option;
};

struct SifFileSlot {
    s32 fd;
    s32 flags;
    s32 reserved8;
    s32 reservedC;
};

extern s32 r1_D_0012FD94_1BF80[] __asm__("r1_D_0012FD94");

extern s32 r1_D_0012FDA0_1BF80[] __asm__("r1_D_0012FDA0");

extern struct FsOpenRequest r1_D_00156980_1BF80 __asm__("r1_D_00156980");

extern u8 r1_D_001575C0_1BF80[] __asm__("r1_D_001575C0");

extern struct SifFileSlot r1_D_00157E80_1BF80[] __asm__("r1_D_00157E80");

extern u8 r1_D_00158080_1BF80[] __asm__("r1_D_00158080");

extern s32 r1_func_00118C70_1BF80(struct SemaphoreParameters *) __asm__("r1_func_00118C70");

extern s32 r1_func_00118C80_1BF80(s32) __asm__("r1_func_00118C80");

extern s32 r1_func_0011BC70_1BF80(void) __asm__("func_0011DDC8");

extern s32 r1_func_00118C90(s32);

extern s32 r1_func_00118CB0(s32);

extern s32 r1_func_0011BC40_1BF80(s32) __asm__("func_0011DD98");

extern s32 func_0011E010(void);

extern struct SifFileSlot *r1_func_0011B770_1BF80(void) __asm__("func_0011D8C8");

extern s32 r1_func_0011BCB0(void);

extern s32 r1_func_0011B4C8(void *, s32, s32, void *, s32, void *, s32, void *, void *);

typedef struct {
    int sema;
    int result;
    int size;
    int handle;
    int slot;
} CloseState;

extern CloseState r1_D_00156980;

extern char r1_D_001575C0[];

extern char r1_D_00158080[];

extern int r1_func_0011B4C8();

/* rac1-decomp src/core/00119D88.c: func_0011BF80 */
/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/library/sceopen.c, sceOpen. */
s32 func_0011E0D8(const u8 *path, s32 flags, ...)
{
    struct SemaphoreParameters semaphore_parameters;
    va_list_1BF80 arguments;
    s32 result;
    s32 path_index;
    s32 slot_mutex;
    s32 mode;
    s32 completion_semaphore;
    s32 slot_index;
    s32 return_value;
    struct SifFileSlot *file_slot;
    struct FsOpenRequest *request;

    request = &r1_D_00156980_1BF80;
    /* The six remaining EE argument registers occupy eight bytes each. */
    arguments = __builtin_next_arg(flags) - 0x30;
    r1_func_0011BC40_1BF80(0);
    if (r1_D_0012FD94_1BF80[0] == 0) {
        r1_func_0011BCB0();
    }
    if (func_0011E010() != 0) {
        r1_func_0011BC70_1BF80();
        return -0x10004;
    }
    file_slot = r1_func_0011B770_1BF80();
    if (file_slot == 0) {
        r1_func_0011BC70_1BF80();
        return -0x13;
    }
    mode = *(s32 *)arguments;
    for (path_index = 0; path_index < 0x400; path_index++) {
        if ((request->path[path_index] = path[path_index]) == 0) {
            break;
        }
    }
    if (path_index == 0x400) {
        request->path[0x3FF] = 0;
    }
    slot_index = file_slot - r1_D_00157E80_1BF80;
    request->flags = (s32) (flags & 0x6FFFFFFF);
    request->mode = mode;
    semaphore_parameters.max_count = 1;
    request->slot = slot_index;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = r1_func_00118C70_1BF80(&semaphore_parameters);
    request->result = &result;
    request->completion_semaphore = completion_semaphore;
    request->result_size = 4;
    if (r1_func_0011B4C8(r1_D_00158080_1BF80, 0, 0, &r1_D_00156980_1BF80, 0x418, r1_D_001575C0_1BF80, 4, 0, 0) < 0) {
        r1_func_00118C80_1BF80(completion_semaphore);
        r1_func_0011BC70_1BF80();
        return -0xB;
    }
    return_value = *(u32 *)((u32) r1_D_001575C0_1BF80 | 0x20000000);
    r1_func_0011BC70_1BF80();
    if (return_value == 0) {
        r1_func_00118C80_1BF80(completion_semaphore);
        return -0xB;
    }
    r1_func_00118CB0(completion_semaphore);
    r1_func_00118C80_1BF80(completion_semaphore);
    if (result < 0) {
        r1_func_00118CB0(r1_D_0012FDA0_1BF80[0]);
        file_slot->flags = 0;
        r1_func_00118C90(r1_D_0012FDA0_1BF80[0]);
        return result;
    }
    return_value = slot_index;
    r1_func_00118CB0(r1_D_0012FDA0_1BF80[0]);
    slot_mutex = r1_D_0012FDA0_1BF80[0];
    file_slot->fd = result;
    file_slot->flags = (s32) (file_slot->flags | flags);
    r1_func_00118C90(slot_mutex);
    return return_value;
}
