/* sceOpen — RAC2 src/core/0011E0D8.c func_0011E0D8 (Lombyte sceopen.c),
   adapted to the RAC5 retail 0x01ED5718. Retail facts folded in:
   - the file-system RPC goes through the 10-argument wrapper at 0x01ED4DE0
     (the 10th word, 0, sits at 0(sp) like sceClose);
   - CreateSema's last field is the semaphore name pointer 0x01F27860;
   - lock 0x01ED5350, unlock 0x01ED5380, slot alloc 0x01ED4D58, check
     0x01ED5680, sceFsInit 0x01ED5478.
   Data: request 0x01FE5940, reply 0x01FE6580, slot table 0x01FE6E40,
   client 0x01FE7040, fs-init flag 0x01EFEFDC, mutex 0x01EFEFE8. */

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned char u8;

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

extern s32 D_01EFEFDC[];
extern s32 D_01EFEFE8[];
extern struct FsOpenRequest D_01FE5940;
extern u8 D_01FE6580[];
extern struct SifFileSlot D_01FE6E40[];
extern u8 D_01FE7040[];
extern char S_01FE7860[];

extern int H_lock(s32);
extern int sceFsInit(void);
extern int H_check(void);
extern struct SifFileSlot *H_slotalloc(void);
extern int H_unlock(void);
extern int CreateSema(struct SemaphoreParameters *);
extern int DeleteSema(s32);
extern int WaitSema(s32);
extern int SignalSema(s32);
extern int H_call_rpc(void *, s32, s32, void *, s32, void *, s32, void *, void *, s32);

s32 sceOpen(const u8 *path, s32 flags, ...) {
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

    request = &D_01FE5940;
    arguments = __builtin_next_arg(flags) - 0x30;
    H_lock(0);
    if (D_01EFEFDC[0] == 0) {
        sceFsInit();
    }
    if (H_check() != 0) {
        H_unlock();
        return -0x10004;
    }
    file_slot = H_slotalloc();
    if (file_slot == 0) {
        H_unlock();
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
    slot_index = file_slot - D_01FE6E40;
    request->flags = (s32)(flags & 0x7FFFFFFF);
    request->mode = mode;
    semaphore_parameters.max_count = 1;
    request->slot = slot_index;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = (s32)S_01FE7860;
    completion_semaphore = CreateSema(&semaphore_parameters);
    request->result = &result;
    request->completion_semaphore = completion_semaphore;
    request->result_size = 4;
    if (H_call_rpc(D_01FE7040, 0, 0, &D_01FE5940, 0x418, D_01FE6580, 4, 0, 0, 0) < 0) {
        DeleteSema(completion_semaphore);
        H_unlock();
        return -0xB;
    }
    return_value = *(u32 *)((u32)D_01FE6580 | 0x20000000);
    H_unlock();
    if (return_value == 0) {
        DeleteSema(completion_semaphore);
        return -0xB;
    }
    WaitSema(completion_semaphore);
    DeleteSema(completion_semaphore);
    if (result < 0) {
        WaitSema(D_01EFEFE8[0]);
        file_slot->flags = 0;
        SignalSema(D_01EFEFE8[0]);
        return result;
    }
    return_value = slot_index;
    WaitSema(D_01EFEFE8[0]);
    slot_mutex = D_01EFEFE8[0];
    file_slot->fd = result;
    file_slot->flags = (s32)(file_slot->flags | flags);
    SignalSema(slot_mutex);
    return return_value;
}
