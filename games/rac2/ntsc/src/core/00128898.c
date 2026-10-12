#include "common.h"
#include "structs.h"

 /* exit */
extern void func_00128898(void *, ...);

/*
 * Reverted: size mismatch (ours=72, retail=60 -- 12 bytes over).
 *
 *   extern int r1_D_00132ED0;
 *   extern char r1_D_0015B640_b[] __asm__("r1_D_0015B640");
 *
 *   int func_001288C0(void) {
 *       int p = (int)r1_D_0015B640_b;
 *       int end = p + 0x3300;
 *       r1_D_00132ED0 = 1;
 *       do {
 *           *(int *)(p + 0) = 0;
 *           *(int *)(p + 4) = 0;
 *           *(int *)(p + 8) = 0;
 *           p += 0x330;
 *       } while (p < end);
 *       return 1;
 *   }
 *
 * Zeroes the +0x00/+0x04/+0x08 fields of all 16 entries in the
 * 0x330-stride table r1_D_0015B640, then marks it initialized. Semantics
 * certain; retail's loop is a plain ascending `bnez` with the
 * comparison as `slt` (signed) and no duplicated body. Because
 * end-start (0x3300) is an exact multiple of the stride (0x330) --
 * always 16 iterations -- this compiler can prove the trip count at
 * compile time and reverses the loop into a `bnezl`-based down-count
 * with the first store duplicated as the branch's delay-slot
 * instruction, regardless of whether the cursor is typed `int`,
 * `unsigned int`, or `char *` (tried all three; `int` at least
 * recovers retail's `slt`, but the reversal itself doesn't go away).
 * A different case from the established "unsigned loop counter blocks
 * reversal" lever ([[rac1-64bit-field-type]]'s sibling precedent,
 * e.g. r1_func_0011DCB8) -- that lever stops reversal driven by a small
 * loop-counter comparison, not one driven by a statically-divisible
 * address range.
 */
extern int r1_D_00132ED0;

typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;

extern Ent330 r1_D_0015B640[];

extern void r1_func_001153FC(void *, int, int);

extern void *memcpy(void *, const void *, unsigned int);

extern int func_001284B0(void *, int, int);

extern char r1_D_0015B640_b[] __asm__("r1_D_0015B640");

typedef struct {
    int w0;
    int mode;
    int w2;
    int w3;
    int w4;
    char name[16];
} Pad2SockParam;

extern int func_00128D58(int);

extern int func_00128E98(unsigned char *, int *);

/* The command word, built as bitfields of an `unsigned long` union: the
   union stays in a register, each insert is a 64-bit and/or, and the
   int view is the dsll32/dsra32 truncation retail passes in $a1. */
typedef union {
    struct {
        unsigned long f0 : 14;
        unsigned long f1 : 2;
        unsigned long f2 : 8;
        unsigned long f3 : 8;
    } b;
    int i;
} McCmd;

extern void *func_00128DB0(int);

extern int func_00128E18(int);

/* A single stack-resident local shared by both call paths below: its
   byte 0 is the state code every path eventually returns (retail's two
   "side" sub-paths converge on one `sb`/`lbu` round trip through it,
   and the RPC path stages its return through the same byte), and the
   word at +4 is the uninitialised in/out count func_001287A8 wants. */
typedef struct {
    unsigned char state;
    char pad[3];
    int result;
} Pad2Buf;

/* rac1-decomp src/core/001236F0.c: func_00124B60 */
/* The library's debug print, compiled out to an empty varargs function.
   Only Sony's 2.9-ee gives retail's shape, spilling $5-$11 and no FP
   argument registers; the game's 2.95.3 also saves $f12-$f18. That is
   how this file was found to be built with 2.9-ee (Makefile.sn). */
void func_00128898(void *fmt, ...) {
}

/* rac1-decomp src/core/001236F0.c: func_00124B88 */
int func_001288C0(void) {
    int p;
    int end;

    r1_D_00132ED0 = 1;
    p = (int)r1_D_0015B640;
    end = p + 0x3300;
    do {
        *(int *)(p + 0) = 0;
        *(int *)(p + 4) = 0;
        *(int *)(p + 8) = 0;
        p += 0x330;
    } while (p < end);
    return 1;
}

/* rac1-decomp src/core/001236F0.c: func_00124BC8 */
/* scePad2CreateSocket(param, buf). c5: `desc.w0 = 0` moved inside the
   else (null) block instead of shared before the if, probing the last
   1-byte beqz/beql (branch-likely) difference from c4. */
int func_00128900(void *param, char *buf) {
    Pad2SockParam desc;
    int r;
    int i;
    char *p;

    if (((int)buf & 0x3F) != 0) {
        return -1;
    }
    if (param != 0) {
        int *src = (int *)param;
        desc.w0 = src[0];
        desc.w2 = src[1];
        desc.w3 = src[2];
        desc.w4 = src[3];
        memcpy(desc.name, (char *)param + 0x10, 16);
    } else {
        desc.w0 = 0;
        desc.w2 = 0;
        desc.w3 = 0;
        desc.w4 = 0;
        desc.name[0] = 0;
    }
    desc.w0 |= 1;
    desc.mode = 1;
    r = func_001284B0(&desc, (int)buf, (int)buf + 0x80);
    if (r < 0) {
        return r;
    }
    *(int *)(r1_D_0015B640_b + r * 0x330) = 1;
    *(char **)(r1_D_0015B640_b + r * 0x330 + 0xC) = buf;
    p = buf;
    for (i = 1; i >= 0; i--) {
        p[0] = 0;
        *(int *)(p + 0x7C) = 0;
        p[1] = 0;
        p[3] = 0;
        p[2] = 0;
        *(int *)(p + 4) = 0;
        r1_func_001153FC(p + 0x1C, 0xFF, 0x20);
        p += 0x80;
    }
    return r;
}

INCLUDE_ASM("asm/core", func_00128A48);

/* rac1-decomp src/core/001236F0.c: func_00124DF0 */
/* Opens entry `port` of the table if it is not (func_00128D58), sends
   command {2, 3, 2, 1} through func_001287A8, expands the 40-bit mask it
   returns into the entry's slots (func_00128E98) and returns the result
   word func_001287A8 filled in. */
int func_00128B28(int port, void *arg1) {
    McCmd cmd;
    int result;
    int r;

    if (r1_D_0015B640[port].unk_04 == 0) {
        if (func_00128D58(port) < 0) {
            return -1;
        }
    }
    cmd.b.f0 = 2;
    cmd.b.f1 = 3;
    cmd.b.f2 = 2;
    cmd.b.f3 = 1;
    r = func_001287A8(port, cmd.i, &result, arg1);
    if (r < 0) {
        return r;
    }
    func_00128E98(arg1, (int *)r1_D_0015B640[port].unk_10);
    return result;
}

/* rac1-decomp src/core/001236F0.c: func_00124EE0 */
/* scePad2GetState(port): link the pad if needed (func_00128D58), then
   ask func_00128E18 (scePad2CheckDma) whether new data has arrived.
   If so, fetch the active "side" struct (func_00128DB0) and take its
   state byte directly if it looks initialised (+0x4 != 0); otherwise
   try one more relink and use it anyway on success, else forget this
   port's link and return 0. If CheckDma says nothing changed, build
   the RPC command word (the same McCmd bitfield union as
   func_00128B28's {2,3,2,1}, here {0xC,2,1,1}) and round-trip it
   through func_001287A8; return its state byte on success, 0 on
   failure. */
int func_00128C18(int port) {
    Pad2Buf buf;
    char *side;
    McCmd cmd;

    if (r1_D_0015B640[port].unk_04 == 0) {
        if (func_00128D58(port) < 0) {
            return 0;
        }
    }
    if (func_00128E18(port) != 0) {
        side = func_00128DB0(port);
        if (*(int *)(side + 0x4) != 0) {
            buf.state = side[0];
        } else if (func_00128D58(port) >= 0) {
            buf.state = side[0];
        } else {
            r1_D_0015B640[port].unk_04 = 0;
            return 0;
        }
        return buf.state;
    }
    cmd.b.f0 = 0xC;
    cmd.b.f1 = 2;
    cmd.b.f2 = 1;
    cmd.b.f3 = 1;
    if (func_001287A8(port, cmd.i, &buf.result, (char *)&buf) < 0) {
        return 0;
    }
    return buf.state;
}
