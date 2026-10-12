#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_D_00155080[];

extern void r1_func_001193F8(int);

extern void r1_func_00118AD0(int, int);

extern int r1_D_00154F54;

extern int r1_D_0012FD04;

extern int r1_func_001193F8_i(int) __asm__("r1_func_001193F8");

extern int r1_func_00118AD0_i(int, int) __asm__("r1_func_00118AD0");

extern void func_0011CB58(void);

extern int r1_D_00154F64 NOT_SDA;

extern int r1_D_00154F6C NOT_SDA;

typedef struct {
    u32 f0 : 8;
    u32 f0_8 : 24;
    u32 f4;
    u32 f8;
} SifCmd;

typedef struct {
    u32 f0;
    u32 f4;
    u32 f8;
    u32 fC;
} SifDma;

extern u32 r1_D_00154F60[];

extern void r1_func_0011AD70(void *addr, s32 size);

extern s32 r1_func_00118E20();

extern s32 r1_func_00118E30();

extern void r1_func_0011AD70(void *, s32);

typedef signed int s32_1B4C8;

extern s32_1B4C8 func_0011CD20();

/* rac1-decomp src/core/00119D88.c: func_0011A758 */
int func_0011C8B0(int arg0) {
    return r1_D_00155080[arg0];
}

INCLUDE_ASM("asm/core", func_0011C8C8);

INCLUDE_ASM("asm/core", func_0011C8D8);

/* rac1-decomp src/core/00119D88.c: func_0011AA00 */
/* r1_func_001193F8 and r1_func_00118AD0 both return int; their results decide
   which registers the globals' addresses get. */
void func_0011CB58(void) {
    r1_func_001193F8_i(0x5);
    r1_func_00118AD0_i(0x5, r1_D_00154F54);
    r1_D_0012FD04 = 0;
}

/* rac1-decomp src/core/00119D88.c: func_0011AA38 */
/* Sets slot arg0's two words: r1_D_00154F6C's for arg0 >= 0, r1_D_00154F64's
   below. In func_0011CBC0's shape: reassigning arg0 to the table is what
   puts the base in $a0, as in retail. */
void func_0011CB90(int arg0, int arg1, int arg2) {
    int off = arg0 << 3;
    if (arg0 < 0) {
        arg0 = r1_D_00154F64;
    } else {
        arg0 = r1_D_00154F6C;
    }
    off += arg0;
    *(int *)(off + 0) = arg1;
    *(int *)(off + 4) = arg2;
}

/* rac1-decomp src/core/00119D88.c: func_0011AA68 */
void func_0011CBC0(int arg0) {
    int offset = arg0 << 3;
    if (arg0 < 0) {
        arg0 = r1_D_00154F64;
    } else {
        arg0 = r1_D_00154F6C;
    }
    offset += arg0;
    *(int *)offset = 0;
}

/* rac1-decomp src/core/00119D88.c: func_0011AA90 */
/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/sdk/rpc/sce_sif_send_cmd.c, _sceSifSendCmd. */
s32 func_0011CBE8(u32 packet, s32 mode, SifCmd *cmd, s32 size, u32 src, u32 dst, s32 dst_size) {
    SifDma tags[2];
    s32 num;
    register u32 mode44;
    register u32 addr0;

    if ((u32)(size - 0x10) >= 0x61) {
        return 0;
    }
    num = 0;
    if (dst_size > 0) {
        cmd->f0_8 = dst_size;
        tags[0].f0 = src;
        tags[0].f4 = dst;
        cmd->f4 = dst;
        tags[0].f8 = dst_size;
        tags[0].fC = 0;
        num = 1;
        if (mode & 4) {
            r1_func_0011AD70(src, dst_size);
        }
    } else {
        cmd->f4 = 0;
        cmd->f0_8 = 0;
    }
    addr0 = r1_D_00154F60[0];
    tags[num].f0 = (u32)cmd;
    tags[num].f4 = addr0;
    tags[num].f8 = size;
    cmd->f0 = size;
    cmd->f8 = packet;
    tags[num].fC = 0x44;
    r1_func_0011AD70(cmd, size);
    num++;
    if (mode & 1) {
        return r1_func_00118E30(tags, num);
    }
    return r1_func_00118E20(tags, num);
}

/* rac1-decomp src/core/00119D88.c: func_0011ABC8 */
/* sceSifSendCmd and isceSifSendCmd: forward to the common sender
   func_0011CBE8 with the mode (0, or 1 from interrupt context) injected
   as its second argument. EABI passes the first eight integer args in
   $4-$11, so this forwards seven of them.

   Both return the sender's result (it sets $v0 on every path; the packet
   id). That is what keeps 2.9-ee from turning the call into a bare tail
   jump (SIZE 40/60 when void), and its schedule is then retail's: the
   `addiu $sp,$sp,-0x10` fourth, after three argument moves, which
   2.95.3 could not produce (11/60). */

int func_0011CD20(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    return func_0011CBE8(arg0, 0, arg1, arg2, arg3, arg4, arg5);
}

/* rac1-decomp src/core/00119D88.c: func_0011AC08 */
int func_0011CD60(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    return func_0011CBE8(arg0, 0x1, arg1, arg2, arg3, arg4, arg5);
}

INCLUDE_ASM("asm/core", func_0011CDA0);

INCLUDE_ASM("asm/core", func_0011CEC8);

INCLUDE_ASM("asm/core", func_0011CF74);
