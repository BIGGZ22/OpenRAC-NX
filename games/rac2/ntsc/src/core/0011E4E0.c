#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern void r1_func_0011AD70(void *addr, s32 size);

extern int r1_func_00118C70(void *);

   /* CreateSema */
extern void r1_func_00118C80(int);

     /* DeleteSema */
extern int r1_func_00118CB0(int);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118C70();

extern s32_1B4C8 r1_func_00118CB0();

extern int r1_D_0012FDA4;

extern char r1_D_00157E80[];

extern int r1_D_0012FD94;

struct SifFileSlot {
    s32 fd;
    s32 flags;
    s32 reserved8;
    s32 reservedC;
};

extern s32 r1_func_00118C90(s32);

extern s32 r1_func_00118CB0(s32);

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

struct SemaphoreParameters2 {
    s32 count;
    s32 max_count;
    s32 init_count;
    s32 wait_threads;
    u32 attr;
    u32 option;
};

extern volatile s32 r1_D_0012FD10[];

struct SifClient {
    u8 pad[0x28];
};

struct FsReadRequest {
    s32 completion_semaphore;
    void *result;
    s32 result_size;
    s32 fd;
    void *buffer;
    s32 length;
    s32 reserved18;
    s32 slot;
};

extern struct FsReadRequest r1_D_00156980_1C5C0 __asm__("r1_D_00156980");

extern s32 r1_D_0012FD94_1C5C0[] __asm__("r1_D_0012FD94");

extern s32 r1_D_0012FDA4_1C5C0[] __asm__("r1_D_0012FDA4");

extern u8 r1_D_001575C0_1C5C0[] __asm__("r1_D_001575C0");

extern struct SifFileSlot r1_D_00157E80_1C5C0[] __asm__("r1_D_00157E80");

extern struct SifClient r1_D_00158080_1C5C0 __asm__("r1_D_00158080");

extern struct SifFileSlot *r1_func_0011B7F8_1C5C0(s32 fd) __asm__("func_0011D950");

extern s32 r1_func_0011BC40_1C5C0(s32) __asm__("func_0011DD98");

extern s32 r1_func_0011BC70_1C5C0(void) __asm__("func_0011DDC8");

extern s32 r1_func_00118C70_1C5C0(struct SemaphoreParameters2 *) __asm__("r1_func_00118C70");

extern s32 r1_func_00118C80_1C5C0(s32) __asm__("r1_func_00118C80");

extern void r1_func_0011AD70(void *, s32);

/* rac1-decomp src/core/00119D88.c: func_0011C5C0 */
/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/sdk/library/sceRead.c, sceRead. */
s32 func_0011E4E0(s32 fd, void *buffer, s32 length)
{
    struct FsReadRequest *request;
    struct SifFileSlot *file_slot;
    struct SemaphoreParameters2 semaphore_parameters;
    s32 result;
    s32 flags;
    s32 completion_semaphore;
    s32 async_index;
    volatile s32 *async_slot;
    s32 rpc_result;

    request = &r1_D_00156980_1C5C0;
    file_slot = r1_func_0011B7F8_1C5C0(fd);
    r1_func_0011BC40_1C5C0(2);
    if (r1_D_0012FD94_1C5C0[0] == 0) {
        r1_func_0011BC70_1C5C0();
        return -1;
    }
    if (file_slot == 0 || (flags = file_slot->flags) == 0) {
        r1_func_0011BC70_1C5C0();
        return -9;
    }
    request->fd = file_slot->fd;
    semaphore_parameters.max_count = 1;
    request->slot = file_slot - r1_D_00157E80_1C5C0;
    request->buffer = buffer;
    request->length = length;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = r1_func_00118C70_1C5C0(&semaphore_parameters);
    request->result = &result;
    request->result_size = 4;
    r1_D_00156980_1C5C0.completion_semaphore = completion_semaphore;
    if ((s16)flags & 0x8000) {
        r1_func_00118CB0(r1_D_0012FDA4_1C5C0[0]);
        for (async_index = 0; async_index < 32; async_index++) {
            volatile s32 *async_semaphores = r1_D_0012FD10;

            async_slot = async_semaphores + async_index;
            if (*async_slot == -1) {
                *async_slot = request->completion_semaphore;
                request->completion_semaphore = -request->completion_semaphore;
                break;
            }
        }
        r1_func_00118C90(r1_D_0012FDA4_1C5C0[0]);
    }
    if (!(flags & 0x20000000)) {
        r1_func_0011AD70(buffer, length);
    }
    r1_func_0011AD70(request, 0x20);
    if (r1_func_0011B4C8(&r1_D_00158080_1C5C0, 2, 0, &r1_D_00156980_1C5C0, 0x20, r1_D_001575C0_1C5C0, 4, 0, 0) < 0) {
        r1_func_00118C80_1C5C0(completion_semaphore);
        r1_func_0011BC70_1C5C0();
        return -11;
    }
    rpc_result = *(s32 *)((u32)r1_D_001575C0_1C5C0 | 0x20000000);
    r1_func_0011BC70_1C5C0();
    if (rpc_result == 0) {
        r1_func_00118C80_1C5C0(completion_semaphore);
        return -11;
    }
    if (flags & 0x8000) {
        r1_func_00118C80_1C5C0(completion_semaphore);
        return 0;
    }
    r1_func_00118CB0(completion_semaphore);
    r1_func_00118C80_1C5C0(completion_semaphore);
    return result;
}

INCLUDE_ASM("asm/core", func_0011E740);

extern int D_00134744;
extern char D_00140140[];
extern int D_00140180[];
extern int D_001401C0[];
extern int func_0011D620(void *, int, int, void *, int, void *, int, int, int);

int func_0011E828(int first, int second, int third) {
    if (D_00134744 < 0)
        return 0;
    D_001401C0[0] = second;
    D_001401C0[1] = first;
    D_001401C0[2] = third;
    if (func_0011D620(D_00140140, 4, 0, D_001401C0, 12, D_00140180, 4, 0, 0) < 0)
        return 0;
    return D_00140180[0];
}

extern int D_00134744;
extern char D_00140140[];
extern int D_00140180[];
extern int D_001401C0[];
extern int func_0011D620(void *, int, int, void *, int, void *, int, int, int);

int func_0011E8A8(int value) {
    if (D_00134744 < 0)
        return 0;
    D_001401C0[0] = value;
    if (func_0011D620(D_00140140, 2, 0, D_001401C0, 4, D_00140180, 4, 0, 0) < 0)
        return -1;
    return D_00140180[0];
}

INCLUDE_ASM("asm/core", func_0011E920);
