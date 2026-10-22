/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The EE kernel and the services of the I/O processor that the game reaches
 * through it (libraries.tsv). The port has no second processor and no
 * kernel: the game is single-threaded and polls, so these keep its handlers
 * (called where the console would raise the interrupt), answer at once, and
 * leave the IOP's services to the replacements of the libraries above them
 * (disc, memory card, pads, sound). */
#include <stdio.h>
#include <stdlib.h>

#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/guest.h"

#include <stdint.h>
#include <string.h>

#define MAX_HANDLERS 8

typedef struct {
    int cause;
    gaddr handler;
    int arg;
    gaddr data;    /* AddDmacHandler2: the handler's second argument */
    int with_data; /* called as handler(cause, data, 0) */
} handler;

static handler intc[MAX_HANDLERS], dmac[MAX_HANDLERS];
static gaddr vsync_callback;

static int add(handler* table, int cause, gaddr fn, int arg) {
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (table[i].handler == 0) {
            table[i] = (handler){cause, fn, arg, 0, 0};
            return i + 1;
        }
    }
    return -1;
}

static int remove_handler(handler* table, int cause, int id) {
    if (id >= 1 && id <= MAX_HANDLERS && table[id - 1].cause == cause) {
        table[id - 1].handler = 0;
        return 0;
    }
    return -1;
}

/* The interrupt causes of the console's INTC that the game uses. */
#define INTC_VBLANK_START 2

void openrac_game_run_vsync_handlers(void) {
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (intc[i].handler != 0 && intc[i].cause == INTC_VBLANK_START) {
            GFN(int (*)(int), intc[i].handler)(intc[i].cause);
        }
    }
    if (vsync_callback != 0) {
        GFN(void (*)(void), vsync_callback)();
    }
}

void openrac_game_run_dmac_handlers(int channel) {
    for (int i = 0; i < MAX_HANDLERS; i++) {
        if (dmac[i].handler != 0 && dmac[i].cause == channel) {
            if (dmac[i].with_data) {
                GFN(int (*)(int, gaddr, gaddr), dmac[i].handler)(channel, dmac[i].data, 0);
            } else {
                GFN(int (*)(int), dmac[i].handler)(channel);
            }
        }
    }
}

/* AddIntcHandler */
int openrac_lib_AddIntcHandler(int cause, gaddr fn, int next) {
    return add(intc, cause, fn, next);
}

/* RemoveIntcHandler */
int openrac_lib_RemoveIntcHandler(int cause, int id) {
    return remove_handler(intc, cause, id);
}

/* AddDmacHandler */
int openrac_lib_AddDmacHandler(int channel, gaddr fn, int next) {
    return add(dmac, channel, fn, next);
}

/* AddDmacHandler2 (system call 0x12, as AddDmacHandler): the handler also
 * gets an argument of the caller's, handler(channel, arg, 0). */
int openrac_lib_AddDmacHandler2(int channel, gaddr fn, int next, gaddr arg) {
    int id = add(dmac, channel, fn, next);
    if (id > 0) {
        dmac[id - 1].data = arg;
        dmac[id - 1].with_data = 1;
    }
    return id;
}

/* RemoveDmacHandler */
int openrac_lib_RemoveDmacHandler(int channel, int id) {
    return remove_handler(dmac, channel, id);
}

/* sceGsSyncVCallback: the previous callback is returned. */
gaddr openrac_lib_sceGsSyncVCallback(gaddr fn) {
    gaddr old = vsync_callback;
    vsync_callback = fn;
    return old;
}

/* Interrupt and DMA controller masks: nothing to mask. */
int openrac_lib_EnableIntc(int cause) {
    (void)cause;
    return 0;
} /* EnableIntc */

int openrac_lib_DisableIntc(int cause) {
    (void)cause;
    return 0;
} /* DisableIntc */

int openrac_lib_DisableDmac(int channel) {
    (void)channel;
    return 0;
} /* DisableDmac */

int openrac_lib_EnableDmac(int channel) {
    (void)channel;
    return 0;
} /* EnableDmac */

/* DIntr and EIntr return whether interrupts were on before (the EIE bit of
 * the console's Status register). The game brackets its critical sections
 * with them: DIntr finds interrupts on, the EIntr closing the bracket finds
 * them off. */
int openrac_lib_DIntr(void) {
    return 1;
} /* DIntr */

int openrac_lib_EIntr(void) {
    return 0;
} /* EIntr */

/* Threads: only the movie player makes them, and it is to be replaced as a
 * whole (libraries.tsv, libmpeg). They are given ids and never run. As in
 * the kernel, a call on a thread returns its id; terminating one fails (-1),
 * as for a thread that has already ended. */
static int next_thread = 2;

/* A thread id the port has: 0 (the calling thread), 1 (the one thread that
 * runs) or one CreateThread handed out. */
static int known_thread(int id) {
    return id >= 0 && id < next_thread;
}

int openrac_lib_CreateThread(gaddr param) {
    (void)param;
    return next_thread++;
} /* CreateThread */

int openrac_lib_DeleteThread(int id) {
    return id;
} /* DeleteThread */

int openrac_lib_StartThread(int id, gaddr arg) { /* StartThread */
    (void)arg;
    openrac_guest_missing("StartThread (threads do not run in the port)");
    return id;
}

int openrac_lib_TerminateThread(int id) {
    (void)id;
    return -1;
} /* TerminateThread */

int openrac_lib_ChangeThreadPriority(int id, int priority) {
    return known_thread(id) ? priority : -1; /* the console: -1 for a thread it does not have */
} /* ChangeThreadPriority */

int openrac_lib_RotateThreadReadyQueue(int priority) {
    return priority;
} /* RotateThreadReadyQueue: the priority rotated */

int openrac_lib_GetThreadId(void) {
    return 1;
} /* GetThreadId */

/* Semaphores: counted, never waited on (one thread). */
#define MAX_SEMAS 32
static int semas[MAX_SEMAS];
static int sema_used[MAX_SEMAS];

int openrac_lib_CreateSema(gaddr params) { /* CreateSema: {count, max_count, init_count, ...} */
    for (int i = 0; i < MAX_SEMAS; i++) {
        if (!sema_used[i]) {
            sema_used[i] = 1;
            semas[i] = GREF(int, params + 8);
            return i + 1;
        }
    }
    return -1;
}

int openrac_lib_DeleteSema(int id) { /* DeleteSema */
    if (id >= 1 && id <= MAX_SEMAS) {
        sema_used[id - 1] = 0;
    }
    return id;
}

int openrac_lib_SignalSema(int id) { /* SignalSema */
    if (id >= 1 && id <= MAX_SEMAS) {
        semas[id - 1]++;
    }
    return id;
}

int openrac_lib_WaitSema(int id) { /* WaitSema */
    if (id >= 1 && id <= MAX_SEMAS && semas[id - 1] > 0) {
        semas[id - 1]--;
    }
    return id;
}

/* Caches: nothing to flush. */
int openrac_lib_EnableCache(int which) {
    (void)which;
    return 0;
} /* EnableCache */

void openrac_lib_FlushCache(int which) {
    (void)which;
} /* FlushCache */

/* The IOP: no second processor. Transfers to it complete at once, its RPC
 * servers are bound and answer, its heap hands out addresses that are never
 * used, modules load. */
int openrac_lib_sceSifDmaStat(unsigned int id) {
    (void)id;
    return -1;
} /* sceSifDmaStat: done */

/* The second processor's memory, for what the game parks there: the "stash"
 * (IOP_stash_daemon, RPC server 0x11) keeps data the game sends with
 * sceSifSetDma and fetches back by RPC. Nothing else of the IOP is modelled. */
#define IOP_BYTES (2u * 1024 * 1024)
static uint8_t iop_memory[IOP_BYTES];

/* sceSifSetDma(transfers, count): each transfer is {source, destination,
 * size, attributes}, the destination an address in the IOP's memory. */
unsigned int openrac_lib_sceSifSetDma(gaddr transfers, int count) { /* sceSifSetDma */
    int i;
    for (i = 0; i < count; i++) {
        uint32_t t[4];
        memcpy(t, G(transfers + (gaddr)i * 16u), 16);
        if (t[1] < IOP_BYTES && t[2] <= IOP_BYTES - t[1]) {
            memcpy(iop_memory + t[1], G(t[0]), t[2]);
        }
    }
    return 1;
}

/* Which RPC server each bound client is for, by client address. */
#define RPC_CLIENTS 16
static gaddr rpc_client[RPC_CLIENTS];
static unsigned int rpc_server[RPC_CLIENTS];
static unsigned int sif_bind_calls;
static unsigned int sif_rpc_calls;

static int trace_sif(void) {
    const char* value = getenv("OPENRAC_TRACE_SIF");
    return value != NULL && value[0] != '\0';
}

static unsigned int server_of(gaddr client) {
    int i;
    for (i = 0; i < RPC_CLIENTS; i++) {
        if (rpc_client[i] == client) {
            return rpc_server[i];
        }
    }
    return 0;
}

void openrac_lib_sceSifInitRpc(int mode) {
    (void)mode;
} /* sceSifInitRpc */

/* Binding succeeds at once: the word at 0x24 of the client (its server) is
 * what the game waits on to become non-zero. */
int openrac_lib_sceSifBindRpc(gaddr client, unsigned int number, int mode) { /* sceSifBindRpc */
    int i;
    uint32_t one = 1;
    if (trace_sif() && sif_bind_calls++ < 32) {
        fprintf(stderr, "SIF bind client=%08x server=%08x mode=%d\n", client, number, mode);
        fflush(stderr);
    }
    (void)mode;
    for (i = 0; i < RPC_CLIENTS; i++) {
        if (rpc_client[i] == 0 || rpc_client[i] == client) {
            rpc_client[i] = client;
            rpc_server[i] = number;
            break;
        }
    }
    memcpy(G(client + 0x24u), &one, 4);
    return 0;
}

int openrac_lib_sceSifCallRpc(
    gaddr client,
    int number,
    int mode,
    gaddr send,
    int ssize,
    gaddr receive,
    int rsize,
    gaddr end,
    gaddr end_param
) { /* sceSifCallRpc */
    const unsigned int server = server_of(client);
    uint32_t bound;
    (void)mode;
    (void)ssize;
    if (trace_sif() && sif_rpc_calls++ < 32) {
        uint32_t client_word_14, client_word_18, client_word_1c, client_server;
        char caller[160];
        memcpy(&client_word_14, G(client + 0x14u), 4);
        memcpy(&client_word_18, G(client + 0x18u), 4);
        memcpy(&client_word_1c, G(client + 0x1cu), 4);
        memcpy(&client_server, G(client + 0x24u), 4);
        openrac_host_caller(1, caller, sizeof caller);
        fprintf(stderr,
                "SIF call caller=%s client=%08x server=%08x fields=%08x/%08x/%08x server_ptr=%08x fn=%d mode=%d send=%08x/%d recv=%08x/%d end=%08x param=%08x\n",
                caller, client, server, client_word_14, client_word_18, client_word_1c,
                client_server, number, mode, send, ssize, receive, rsize, end, end_param);
        fflush(stderr);
    }
    /* An unbound client has no IOP server to receive this request. Reporting
     * success without completing an async call leaves the game's wait loop
     * spinning forever on its untouched receive buffer. Unbound is what the
     * game's own memory says: the client's server word (+0x24), which binding
     * sets and the console's sceSifCallRpc sends with the request, still 0.
     * The table above only says which server a bound client is for (it holds
     * RPC_CLIENTS of them); a call replayed on its own, without the binds
     * before it, still sees the binding. */
    memcpy(&bound, G(client + 0x24u), 4);
    if (bound == 0) {
        return -1;
    }
    /* The stash: function 2 says where it is in the IOP's memory and how long;
     * function 1 copies from the address sent into the receive buffer. */
    if (server_of(client) == 0x11) {
        const uint32_t base = 0x00080000u, bytes = 0x00170000u;
        if (number == 2 && rsize >= 8) {
            memcpy(G(receive), &base, 4);
            memcpy(G(receive + 4u), &bytes, 4);
        } else if (number == 1 && rsize > 0) {
            uint32_t from;
            memcpy(&from, G(send), 4);
            if (from < IOP_BYTES && (uint32_t)rsize <= IOP_BYTES - from) {
                memcpy(G(receive), iop_memory + from, (size_t)rsize);
            }
        }
        if (end != 0) {
            GFN(void (*)(gaddr), end)(end_param);
        }
    }
    return 0;
}

int openrac_lib_sceSifCheckStatRpc(gaddr client) {
    (void)client;
    return 0;
} /* sceSifCheckStatRpc: idle */

int openrac_lib_sceSifInitIopHeap(void) {
    return 0;
} /* sceSifInitIopHeap */

int openrac_lib_sceSifAllocIopHeap(int size) {
    (void)size;
    return 0x00100000;
} /* sceSifAllocIopHeap */

int openrac_lib_sceSifFreeIopHeap(int at) {
    (void)at;
    return 0;
} /* sceSifFreeIopHeap */

int openrac_lib_sceSifLoadModuleBuffer(
    gaddr module, int size, gaddr args
) { /* sceSifLoadModuleBuffer */
    (void)module;
    (void)size;
    (void)args;
    return 1;
}

int openrac_lib_sceSifSyncIop(void) {
    return 1;
} /* sceSifSyncIop */

int openrac_lib_sceSifRebootIop(gaddr image) {
    (void)image;
    return 1;
} /* sceSifRebootIop */

/* fileio: the game reads no asset this way (debug output and rom0: settings
 * only). Writes to the console's terminal go to the log. */
int openrac_lib_sceFsReset(void) {
    return 0;
} /* sceFsReset */

int openrac_lib_sceOpen(gaddr name, int flags, ...) { /* sceOpen */
    (void)flags;
    fprintf(stderr, "[game] sceOpen(\"%s\"): no files in the port\n", (const char*)G(name));
    return -1;
}

int openrac_lib_sceClose(int fd) {
    (void)fd;
    return 0;
} /* sceClose */

int openrac_lib_sceLseek(int fd, int offset, int whence) { /* sceLseek */
    (void)fd;
    (void)offset;
    (void)whence;
    return -1;
}

int openrac_lib_sceRead(int fd, gaddr buf, int size) { /* sceRead */
    (void)fd;
    (void)buf;
    (void)size;
    return -1;
}

int openrac_lib_sceWrite(int fd, gaddr buf, int size) { /* sceWrite */
    if (fd == 1 || fd == 2) {
        fprintf(stderr, "[game] %.*s", size, (const char*)G(buf));
        return size;
    }
    return -1;
}

/* The scf library: the console's system settings. */
int openrac_lib_sceScfGetLanguage(void) { /* sceScfGetLanguage */
    return openrac_game_language;
}

void openrac_lib_sceScfGetLocalTimefromRTC(gaddr clock
) { /* sceScfGetLocalTimefromRTC: the clock is local already */
    (void)clock;
}

/* SetGsCrt(interlace, mode, field): the display mode; the port shows the
 * game's frames in its window whatever the mode. */
void openrac_lib_SetGsCrt(int interlace, int mode, int field) {
    openrac_game_set_video_mode(interlace, mode, field);
} /* SetGsCrt */

/* GsPutIMR(mask): which GS events interrupt; the port has no GS interrupts. */
void openrac_lib_GsPutIMR(unsigned int mask) {
    (void)mask;
} /* GsPutIMR */

/* ---- The other EE kernel services the game's libraries call ----
 *
 * Each is a system call stub in the program (addiu $v1, N; syscall; jr $ra).
 * Which service a number is comes from the open-source ps2sdk's table of EE
 * kernel calls (ee/kernel/include/syscallnr.h, AFL 2.0); the argument
 * layouts from its kernel.h and osd_config.h. Facts only: nothing of the
 * console's own kernel is reproduced. The port has one thread and no
 * interrupts, so what would wait for an event finds it has happened. */

/* Exit (KExit, 0x04): the program ends. Only the start-up code calls it,
 * when the memory map it sets up does not fit. */
void openrac_lib_Exit(int status) {
    fprintf(stderr, "[game] Exit(%d)\n", status);
    exit(status);
} /* Exit */

/* SetAlarm (0xFC): the callback runs once `time` horizontal blanks have
 * passed, from the timer interrupt. The port has no interrupts and treats
 * every wait as already over: it runs the callback at once. */
static int next_alarm = 1;

int openrac_lib_SetAlarm(unsigned short time, gaddr callback, gaddr common) { /* SetAlarm */
    int id = next_alarm++;
    if (callback != 0) {
        GFN(void (*)(int, unsigned short, gaddr), callback)(id, time, common);
    }
    return id;
}

/* ReferThreadStatus (0x30): the 0x30-byte status record of a thread
 * (status, entry, stack, stack size, gp, initial and current priority,
 * attributes, option, wait type, wait id, wake-up count). The port's one
 * thread is always running (status 1). */
int openrac_lib_ReferThreadStatus(int id, gaddr info) { /* ReferThreadStatus */
    uint32_t status[12] = {0};
    status[0] = 1; /* THS_RUN */
    status[5] = 1; /* initial priority */
    status[6] = 1; /* current priority */
    if (info != 0) {
        memcpy(G(info), status, sizeof status);
    }
    return id;
}

/* SleepThread (0x32), WakeupThread (0x33, and _iWakeupThread, -0x34),
 * SuspendThread (0x37): one thread, which the wake-up it waits for (an
 * alarm, above) has already reached. */
int openrac_lib_SleepThread(void) {
    return openrac_lib_GetThreadId();
} /* SleepThread */

int openrac_lib_WakeupThread(int id) {
    return id;
} /* WakeupThread */

int openrac_lib_SuspendThread(int id) {
    return id;
} /* SuspendThread */

/* EndOfHeap (0x3E): the top of the area sbrk may grow into. The game's stack
 * is outside its RAM in the port (GUEST_STACK_TOP), so the heap may reach
 * the end of the 32 MB. */
gaddr openrac_lib_EndOfHeap(void) {
    return GUEST_RAM + GUEST_RAM_SIZE;
} /* EndOfHeap */

/* PollSema (0x45, and iPollSema): WaitSema without the wait: the semaphore's
 * id when its count was above zero (and is now one less), else negative. */
int openrac_lib_PollSema(int id) { /* PollSema */
    if (id >= 1 && id <= MAX_SEMAS && sema_used[id - 1] && semas[id - 1] > 0) {
        semas[id - 1]--;
        return id;
    }
    return -1;
}

/* The system settings word (GetOsdConfigParam 0x4B, SetOsdConfigParam 0x4A):
 * bit 0 S/PDIF off, bits 1-2 screen type, 3 video output, 4 non-Japanese,
 * 5-12 PS1 driver, 13-15 version, 16-20 language, 21-31 time zone. Until the
 * game sets one, it says: version 1, the language of the port's settings
 * (the same numbering: 1 English, 2 French, ...), 4:3, no time-zone offset. */
static uint32_t osd_config;
static int osd_config_set;

static uint32_t osd_config_word(void) {
    if (osd_config_set) {
        return osd_config;
    }
    return (1u << 4) | (1u << 13) | (((uint32_t)openrac_game_language & 31u) << 16);
}

void openrac_lib_SetOsdConfigParam(gaddr config) { /* SetOsdConfigParam */
    if (config != 0) {
        memcpy(&osd_config, G(config), 4);
        osd_config_set = 1;
    }
}

void openrac_lib_GetOsdConfigParam(gaddr config) { /* GetOsdConfigParam */
    if (config != 0) {
        uint32_t word = osd_config_word();
        memcpy(G(config), &word, 4);
    }
}

/* GetOsdConfigParam2 (0x6F): `size` bytes of the extended settings from
 * `offset` (daylight saving, time and date formats): all zero, the
 * console's defaults (standard time, 24-hour clock, YYYYMMDD). */
void openrac_lib_GetOsdConfigParam2(gaddr config, int size, int offset) { /* GetOsdConfigParam2 */
    (void)offset;
    if (config != 0 && size > 0) {
        memset(G(config), 0, (size_t)size);
    }
}

/* The memory map (GetMemorySize 0x7F, _InitTLB 0x82, _SetTLBEntry 0x56):
 * the start-up code sizes RAM and maps the TLB from it. The port's guest
 * memory is 32 MB and has no TLB. */
int openrac_lib_GetMemorySize(void) {
    return (int)GUEST_RAM_SIZE;
} /* GetMemorySize */

int openrac_lib__InitTLB(void) {
    return 0;
} /* _InitTLB */

int openrac_lib__SetTLBEntry(
    unsigned int index, unsigned int mask, unsigned int hi, unsigned int lo0, unsigned int lo1
) { /* _SetTLBEntry */
    (void)mask;
    (void)hi;
    (void)lo0;
    (void)lo1;
    return (int)index;
}

/* Kernel patching (SetSyscall 0x74, GetEntryAddress 0x5B, Copy 0x5A,
 * FindAddress 0x83): the start-up code replaces kernel routines with its
 * own. The port has no kernel: the table of system calls is kept (so what is
 * read back is what was set), copies into kernel memory go nowhere, and a
 * search of kernel memory finds nothing (0). A copy whose both ends are in
 * the game's RAM is done. */
#define SYSCALLS 256
static gaddr syscall_table[SYSCALLS];

void openrac_lib_SetSyscall(int number, gaddr handler) { /* SetSyscall */
    syscall_table[(unsigned int)number % SYSCALLS] = handler;
}

gaddr openrac_lib_GetEntryAddress(int number) { /* GetEntryAddress */
    return syscall_table[(unsigned int)number % SYSCALLS];
}

int openrac_lib_Copy(gaddr dest, gaddr src, int size) { /* Copy */
    if (size > 0 && dest < GUEST_RAM_SIZE && (uint32_t)size <= GUEST_RAM_SIZE - dest &&
        src < GUEST_RAM_SIZE && (uint32_t)size <= GUEST_RAM_SIZE - src) {
        memmove(G(dest), G(src), (size_t)size);
    }
    return 0;
}

gaddr openrac_lib_FindAddress(gaddr start, gaddr end, gaddr value) { /* FindAddress */
    (void)start;
    (void)end;
    (void)value;
    return 0;
}

/* SetVSyncFlag (0x73): at the next vertical blank the kernel stores 1 in
 * *flag and the GS's status register in *csr (64 bits). The game spins on
 * the flag; in the port the blank has come: both are stored now. */
void openrac_lib_SetVSyncFlag(gaddr flag, gaddr csr) { /* SetVSyncFlag */
    uint32_t one = 1;
    uint64_t status = 0;
    if (csr != 0) {
        memcpy(G(csr), &status, 8);
    }
    if (flag != 0) {
        memcpy(G(flag), &one, 4);
    }
}

/* The SIF (sceSifStopDma 0x6B, sceSifSetDChain 0x78, sceSifSetReg 0x79,
 * sceSifGetReg 0x7A): the link to the IOP, which the port does not have.
 * Its registers keep what is written; the IOP's status flags (register 4)
 * always read as up and its command system ready (0x10000, 0x20000), so a
 * wait for the IOP ends at once. */
#define SIF_REGS 8
static int sif_reg[SIF_REGS], sif_sysreg[SIF_REGS];

void openrac_lib_sceSifStopDma(void) {
} /* sceSifStopDma */

void openrac_lib_sceSifSetDChain(void) {
} /* sceSifSetDChain */

int openrac_lib_sceSifSetReg(unsigned int reg, int value) { /* sceSifSetReg */
    int* file = (reg & 0x80000000u) ? sif_sysreg : sif_reg;
    unsigned int i = reg & 0x7FFFFFFFu;
    if (i < SIF_REGS) {
        int old = file[i];
        file[i] = value;
        return old;
    }
    return 0;
}

int openrac_lib_sceSifGetReg(unsigned int reg) { /* sceSifGetReg */
    const int* file = (reg & 0x80000000u) ? sif_sysreg : sif_reg;
    unsigned int i = reg & 0x7FFFFFFFu;
    if (i >= SIF_REGS) {
        return 0;
    }
    if (!(reg & 0x80000000u) && i == 4) {
        return file[i] | 0x30000;
    }
    return file[i];
}

/* Deci2Call (0x7C): the development kit's debug link. Function 16 (kputs)
 * writes a string to the console's terminal, which goes to the log; the
 * others (open, close, send, poll, ...) fail, as on a console without the
 * link. */
int openrac_lib_Deci2Call(int function, gaddr args) { /* Deci2Call */
    if (function == 16 && args != 0) {
        uint32_t text;
        memcpy(&text, G(args), 4);
        if (text != 0) {
            fprintf(stderr, "[game] %s", (const char*)G(text));
        }
        return 0;
    }
    return -1;
}


/* Guest RAM and its aliases share native storage; no separate PS2 data cache. */
void openrac_lib_SyncDCache(gaddr start, gaddr end) {
    (void)start;
    (void)end;
}

void openrac_lib_InvalidDCache(gaddr start, gaddr end) {
    openrac_lib_SyncDCache(start, end);
}
