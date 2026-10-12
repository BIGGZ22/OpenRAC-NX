#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_func_0011D960(void);

           /* DI */
extern void r1_func_0011D9A8(void);

       /* sceSifSetReg */
extern void func_0011D238(char *);

/*
 * sifrpc's _request_end: the completion handler for a request block. Two
 * command ids do extra work before the common teardown: 0x8000000A (RPC
 * call) runs the client's end callback, 0x80000009 (RPC bind) copies the
 * server, buff and cbuff back into it. Then, whatever the id was,
 * iSignalSema the client's semaphore if it has one, free its packet
 * (func_0011D1E8) and clear the pointer. Exact under both compilers.
 *
 * Two levers (build-sn/try/func_0011D238/RESULT.md):
 *  - Retail's dispatch is gcc's case-node decision tree rooted at
 *    0x8000000A, which a two-case switch never builds (at two nodes it
 *    stays a chain). The switch has a third case above 0x8000000A that
 *    shares the default's body: with one shared label the node stays in
 *    the case list, the three-node tree roots at the median, and the
 *    now-redundant test is deleted. Any value above 0x8000000A works;
 *    0x8000000C is ps2sdk's SIF_CMD_RPC_RDATA, a guess.
 *  - `o` is loaded inside each arm and reloaded only after the callback.
 *    (Re-read once after the switch it is also exact under 2.9-ee, whose
 *    -fstrict-aliasing drops the reload after the 0x80000009 stores, but
 *    2.95.3 keeps that reload.)
 */
extern int r1_func_00118CA0(int);

extern void func_0011D2F0(void *, void *);

extern void func_0011D3A0(void *, void *);

typedef struct {
    int unk0;
    int unk4;
    int unk8;
    char pad_C[0x4];
    int unk10;
    char pad_14[0x10];
    int unk24;
} BindClient;

typedef struct {
    char pad_0[0x14];
    int unk14;
    int unk18;
    int unk1C;
    int unk20;
} BindPacket;

extern char r1_D_00156900[];

extern int r1_func_00118C70(void *);

   /* CreateSema */
extern void r1_func_00118C80(int);

     /* DeleteSema */
extern int r1_func_00118CB0(int);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118C70();

extern s32_1B4C8 r1_func_00118CB0();

extern s32_1B4C8 func_0011CD20();

extern s32 r1_func_00118CB0(s32);

extern int func_0011D450(void *, int, int);

extern void func_0011D590(void *, void *);

extern void r1_func_00119678(int thread_id);

/* rac1-decomp src/core/00119D88.c: func_0011AFE8 */
/* Claim the first free 0x40-byte slot in the table {next_id, base,
   count} at arg0, with interrupts held off across the search. A claimed
   slot records its index and state in +0x10, points +0x14 at itself and
   takes the next id; id 0 is skipped, so after handing out 1 the
   counter jumps straight to 2. Returns the slot, or 0 if the table is
   full.

   Two shapes mattered. `i` must be assigned AFTER the r1_func_0011D960
   call: initialised in its declaration it is live across the call, so
   it lands in a third callee-saved register and drags in a save/restore
   pair, where retail keeps it in $v1.

   And the id fan-in has to be written as an explicit if/ELSE with a
   separate assignment in each arm. Retail rematerialises `addiu $v1,$0,1`
   in the taken arm even though $v1 already holds that value, and no
   form that computes the value once -- including writing the redundant
   `id = 1;` after the store -- survives: the compiler folds it away and
   the function comes out 4 bytes short. Giving the two arms their own
   assignments to `id` keeps both definitions alive. */
void *func_0011D140(void *arg0) {
    char *a = (char *)arg0;
    int n;
    char *p;
    int i;
    int id;
    int v;

    r1_func_0011D960();
    n = *(int *)(a + 0x8);
    i = 0;
    p = *(char **)(a + 0x4);
    while (i < n) {
        if ((*(int *)(p + 0x10) & 1) == 0) {
            *(int *)(p + 0x10) = (i << 16) | 5;
            v = *(int *)a + 1;
            *(int *)a = v;
            if (v == 1) {
                *(int *)a = v + 1;
                id = 1;
            } else {
                id = v;
            }
            *(char **)(p + 0x14) = p;
            *(int *)(p + 0x18) = id;
            r1_func_0011D9A8();
            return p;
        }
        i++;
        p += 0x40;
    }
    r1_func_0011D9A8();
    return 0;
}

/* rac1-decomp src/core/00119D88.c: func_0011B090 */
void func_0011D1E8(void *arg0) {
    char *self = (char *)arg0;
    unsigned int flags = *(unsigned int *)(self + 0x10);
    *(int *)(self + 0x18) = 0;
    *(unsigned int *)(self + 0x10) = flags & 0xFFFFFFFEu;
}

/* rac1-decomp src/core/00119D88.c: func_0011B0B0 */
/*
 * REVERTED at 20/48 (same size). Semantics are certain:
 *
 *   int rem = *(int *)(arg0 + 0x24) % *(int *)(arg0 + 0x18);
 *   *(int *)(arg0 + 0x24) = rem + 1;
 *   return *(int *)(arg0 + 0x14) + (rem << 6);
 *
 * A wrapping counter: takes the modulo of field 0x24 by field 0x18,
 * writes back rem+1, and returns field 0x14 + rem*64 (a 64-byte-stride
 * table index). Every instruction matches retail including the div trap
 * guard; the residual is the allocator picking the opposite registers
 * for the divisor and the mfhi result ($2/$3 swapped versus retail),
 * which then reorders the tail so the store lands in the jr delay slot
 * where retail puts the addu. Tried: hoisting the base load into a
 * local, and naming rem+1 as a separate local -- neither changed the
 * allocation. Same open scratch-register question as r1_func_001160D8.
 */
/* Hand out the next 0x40-byte slot of the ring {.., +0x14 base, +0x18
   count, .., +0x24 next}: take next modulo count, advance next past it,
   return the slot. The slot address has to be formed BEFORE the store of
   the new index: that is what gives retail's allocation (remainder in $v0,
   divisor in $v1) and its tail, `addiu` before `sll` and the `addu` in the
   jr delay slot. Storing first and indexing in the return is 20/48. */
/* The slot address is computed into a local before next = i + 1 is
   stored. */
void *func_0011D208(void *arg0) {
    char *p = (char *)arg0;
    int i = *(int *)(p + 0x24) % *(int *)(p + 0x18);
    char *s = *(char **)(p + 0x14) + i * 0x40;
    *(int *)(p + 0x24) = i + 1;
    return s;
}

/* rac1-decomp src/core/00119D88.c: func_0011B0E0 */
void func_0011D238(char *p) {
    char *o;
    int h;

    switch (*(unsigned int *)(p + 0x20)) {
    case 0x8000000A:
        o = *(char **)(p + 0x1C);
        if (*(int *)(o + 0x1C) != 0) {
            (*(void (**)(int))(o + 0x1C))(*(int *)(o + 0x20));
            o = *(char **)(p + 0x1C);
        }
        break;
    case 0x80000009:
        o = *(char **)(p + 0x1C);
        *(int *)(o + 0x24) = *(int *)(p + 0x24);
        *(int *)(o + 0x14) = *(int *)(p + 0x28);
        *(int *)(o + 0x18) = *(int *)(p + 0x2C);
        break;
    case 0x8000000C:
    default:
        o = *(char **)(p + 0x1C);
        break;
    }
    h = *(int *)(o + 0x8);
    if (h >= 0) {
        r1_func_00118CA0(h);
    }
    func_0011D1E8(*(void **)o);
    *(int *)o = 0;
}

/* rac1-decomp src/core/00119D88.c: func_0011B198 */
/* _request_rdata (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_RDATA (0x8000000c). Grabs a free response packet
   (func_0011D208 = _rpc_get_fpacket), copies the requester's pkt_addr
   and recvbuf pointer (rdata's words 5 and 7) into the response, tags
   the response SIF_CMD_RPC_RDATA (word 8), then forwards the
   requester's src/dest/size (words 8/9/10) to the IOP as
   isceSifSendCmd(SIF_CMD_RPC_END, ...) (func_0011CD60). Void: the
   trailing call is a tail jump. Both copied fields have to be read into
   their own locals before either store: that is what puts pkt_addr in
   $a1 and recvbuf in $a0, matching retail (any other shape -- inline
   reads, one field hoisted, or reversed declaration order -- swaps that
   allocation or reloads recvbuf late). */
void func_0011D2F0(void *rdata, void *data) {
    int *r = (int *)rdata;
    int *rend = (int *)func_0011D208(data);
    int pkt_addr = r[5];
    int recvbuf = r[7];

    rend[5] = pkt_addr;
    rend[7] = recvbuf;
    rend[8] = 0x8000000C;
    func_0011CD60(0x80000008, (int)rend, 0x40, r[8], r[9], r[10]);
}

/* rac1-decomp src/core/00119D88.c: func_0011B1F8 */
void *func_0011D350(int key, void *arg1) {
    void *outer = *(void **)((char *)arg1 + 0x28);
    while (outer != 0) {
        void *inner = *(void **)((char *)outer + 8);
        while (inner != 0) {
            if (*(int *)inner == key) {
                return inner;
            }
            inner = *(void **)((char *)inner + 0x38);
        }
        outer = *(void **)((char *)outer + 0x14);
    }
    return 0;
}

/* rac1-decomp src/core/00119D88.c: func_0011B248 */
/* _request_bind (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_BIND (0x80000009). Grabs a free response packet
   (func_0011D208 = _rpc_get_fpacket), copies the requester's pkt_addr
   and cd (bind's words 5 and 7) into it, tags it SIF_CMD_RPC_BIND,
   looks up the requested service id (bind's word 8, search_svdata =
   func_0011D350) and fills in the server/buf/cbuf fields (words 9-11)
   from it if found, zeroing them otherwise, then forwards to the IOP as
   isceSifSendCmd(SIF_CMD_RPC_END, ...) (func_0011CD60). Matches
   ps2sdk's _request_bind field-for-field (sifrpc.c).

   Two shapes mattered: pkt_addr and cd need their own locals declared
   in that order (pkt_addr first) even though cd is stored first --
   reversed, the two loads swap registers (func_0011D2F0's lever again);
   and the not-found case has to be the `if` arm, sd-found the `else`
   (retail branches on `sd == 0` being false, i.e. `bnel`; writing the
   found case first inverts the branch, as in func_0011D590). */
void func_0011D3A0(void *bind, void *data) {
    int *b = (int *)bind;
    int *rend = (int *)func_0011D208(data);
    int pkt_addr = b[5];
    int cd = b[7];
    int *sd;

    rend[7] = cd;
    rend[5] = pkt_addr;
    rend[8] = 0x80000009;
    sd = (int *)func_0011D350(b[8], data);
    if (sd == 0) {
        rend[9] = 0;
        rend[10] = 0;
        rend[11] = 0;
    } else {
        rend[9] = (int)sd;
        rend[10] = sd[2];
        rend[11] = sd[5];
    }
    func_0011CD60(0x80000008, (int)rend, 0x40, 0, 0, 0);
}

/* rac1-decomp src/core/00119D88.c: func_0011B2F8 */
      /* WaitSema */

/* sceSifBindRpc. Adapted from Lombyte (MIT) for PAL. */
int func_0011D450(void *client, int arg1, int arg2) {
    BindClient *arg0 = client;
    int sema_param[6];
    int temp_2_31;
    BindPacket *temp_2_15;

    arg0->unk10 = 0;
    arg0->unk24 = 0;
    temp_2_15 = (BindPacket *)func_0011D140(r1_D_00156900);
    if (temp_2_15 == 0) {
        return -1;
    }
    arg0->unk4 = temp_2_15->unk18;
    arg0->unk0 = (int)temp_2_15;
    temp_2_15->unk20 = arg1;
    temp_2_15->unk14 = (int)temp_2_15;
    temp_2_15->unk1C = (int)arg0;
    if (arg2 & 1) {
        goto block_7;
    }
    sema_param[1] = 1;
    sema_param[2] = 0;
    temp_2_31 = r1_func_00118C70(sema_param);
    arg0->unk8 = temp_2_31;
    if (temp_2_31 >= 0) {
        goto block_4;
    }
    func_0011D1E8(temp_2_15);
    return -3;
block_4:
    if (func_0011CD20(0x80000009, (int)temp_2_15, 0x40, 0, 0, 0) != 0) {
        goto block_6;
    }
    func_0011D1E8(temp_2_15);
    r1_func_00118C80(arg0->unk8);
    return -2;
block_6:
    r1_func_00118CB0(arg0->unk8);
    r1_func_00118C80(arg0->unk8);
    return 0;
block_7:
    arg0->unk8 = -1;
    if (func_0011CD20(0x80000009, (int)temp_2_15, 0x40, 0, 0, 0) != 0) {
        return 0;
    }
    func_0011D1E8(temp_2_15);
    return -2;
}

/* rac1-decomp src/core/00119D88.c: func_0011B438 */
/* _request_call (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_CALL (0x8000000a). Queues the request's server object
   (request->sd, word 13) onto its data queue's (sd->base, word 16)
   linked list of pending calls -- append to the current end if the
   queue is non-empty (base->start, word 3), else start a new list --
   then copies the request's fields into the server object (pkt_addr,
   client/cd, rpc_number, size/send_size, recvbuf, rsize/recv_size,
   rmode, rid/rec_id), and finally wakes the queue's worker thread
   (iWakeupThread, r1_func_00119678) unless it is already running or has
   none. Matches ps2sdk's _request_call field-for-field (sifrpc.c).

   Three shapes mattered: the empty-queue arm has to come first (retail
   tests and branches on it, not on the append case); and the first two
   copied fields (pkt_addr, client) need their own locals, both declared
   right there (not hoisted to the top, which pulls their loads into the
   queue-splice code) and both declared before either store -- the same
   allocator lever as func_0011D2F0 -- or the pair's load/store order
   swaps or its two registers trade places. */
void func_0011D590(void *request, void *data) {
    int *req = (int *)request;
    int *sd = (int *)req[13];
    int *base = (int *)sd[16];

    if (base[3] == 0) {
        base[3] = (int)sd;
    } else {
        int *end = (int *)base[4];
        end[15] = (int)sd;
    }
    base[4] = (int)sd;
    {
        int pkt_addr = req[5];
        int client = req[7];
        sd[8] = pkt_addr;
        sd[7] = client;
    }
    sd[9] = req[8];
    sd[3] = req[9];
    sd[10] = req[10];
    sd[11] = req[11];
    sd[12] = req[12];
    sd[13] = req[4];

    if (base[0] < 0 || base[1] != 0) return;
    r1_func_00119678(base[0]);
}

INCLUDE_ASM("asm/core", func_0011D620);

/* rac1-decomp src/core/00119D88.c: func_0011B6B8 */
/*
 * REVERTED: size mismatch, 56 bytes against retail's 60. Semantics are
 * certain:
 *
 *   char *p = *(char **)arg0;
 *   if (p == 0) return 0;
 *   if (*(int *)(arg0 + 4) != *(int *)(p + 0x18)) return 0;
 *   if (*(int *)(p + 0x10) & 1) return 1;
 *   return 0;
 *
 * Retail keeps TWO exit blocks -- a shared `return 0` and a separate
 * `return 1`, each with its own `jr` -- which is 15 instructions. This
 * compiler merges them into a single exit and sets $v0 in the branch
 * delay slots, 14 instructions. Tried both the && chain and explicit
 * early returns; both merge. The exit structure is not expressible from
 * C here.
 */
/* Is the handle {slot, id} at arg0 still live: the slot is set, its id at
   +0x18 still matches the handle's, and its in-use bit (+0x10 bit 0) is
   set. Written as one inverted guard that returns 0, then `return 1`:
   that keeps retail's two exits (shared `return 0` block the tests fall
   into, a separate `return 1`), and gcc leaves all three branch slots to
   the assembler, which fills them with nops as retail has them. The &&
   form returning 1 merges the exits; early returns give bnel. */
/* One test for the three failure cases, then return 1. */
int func_0011D810(void *arg0) {
    char *p = (char *)arg0;
    char *q = *(char **)p;

    if (q == 0 || *(int *)(p + 4) != *(int *)(q + 0x18) ||
        !(*(int *)(q + 0x10) & 1)) {
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/core", func_0011D850);
