#include <stdarg.h>
#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x119D88-0x11CCE0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl), several of its modules back to
 * back: the kernel printf (scePrintf, func_0011A6C8, over the print core
 * func_0011A0A0), sifcmd (the command handler table and sceSifSendCmd),
 * sifrpc (the 0x40-byte packet table, sceSifBindRpc, sceSifCallRpc =
 * func_0011B4C8), the IOP file stubs (sceOpen, sceClose, sceRead, ... over
 * a 32-entry fd table) and the IOP heap stubs. Built with Sony's 2.9-ee
 * like the rest of the SDK (Makefile.sn, EE29_CORE): the sd/ld spills and
 * the frames of func_0011A6C8 and func_0011B770 are that compiler's.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);
extern int func_00114060(int, void *);
extern void func_00113AE0(void *);
extern void func_00117118(void *, void *, int, int);
extern int func_00119008();
extern int D_0012FCF0 NOT_SDA;
extern void func_00118E90(int arg0, void *arg1);
extern void *D_00154A40 NOT_SDA;
extern int func_001160D8(void);

/* kputchar (libkernl.a:kprintf.o): wait until the SIO status port
   (0x1000F130) has its busy bit (0x8000) clear, write the character to
   the SIO data port (0x1000F180) and return it.

   Retail's store is not volatile (it fills the return's delay slot, which
   a volatile access never does) yet its address stays in a register
   (lui/ori). A plain constant address is folded into the store by the
   optimizer instead, so the address goes through an empty asm that hides
   its value. */
int func_00119D88(int c) {
    unsigned char *p;

    while (*(volatile unsigned int *)0x1000F130 & 0x8000)
        ;
    __asm__("" : "=r"(p) : "0"(0x1000F180));
    *p = c;
    return c;
}

extern void func_00119840(char *);
extern int D_0012FCFC;
extern char D_00154E00[];

/* deci2Putchar (libkernl.a:kprintf.o): the kprintf hook used for the
   deci2 debug console. Buffers characters into D_00154E00 (a 0x80-byte
   line buffer) counted by D_0012FCFC, flushing (func_00119840) and
   resetting the count once it reaches 0x7E; on '\n' it also flushes
   immediately (appending the '\n' first) instead of just buffering it. */
void func_00119DC0(int c) {
    int count = D_0012FCFC;

    if (count >= 0x7E) {
        D_0012FCFC = 0;
        D_00154E00[0x7F] = 0;
        func_00119840(D_00154E00);
        count = D_0012FCFC;
    }
    if (c == 0xA) {
        D_0012FCFC = 0;
        D_00154E00[count] = c;
        D_00154E00[count + 1] = 0;
        func_00119840(D_00154E00);
    } else {
        D_0012FCFC = count + 1;
        D_00154E00[count] = c;
    }
}

extern int func_00119D88(int c);

/* serialPutchar (libkernl.a:kprintf.o): the printf hook used when going to
   the serial port. '\n' is expanded to a CR/LF pair (write '\r' then the
   original character); anything else is written as-is via kputchar. */
void func_00119E70(int c) {
    if (c == 0xA) {
        func_00119D88(0xD);
        func_00119D88(0xA);
    } else {
        func_00119D88(c);
    }
}

/* Float printer: extract the exponent and hidden mantissa bit, retain two
   guard bits when shifting right, and round up when both are set.
   Uses the target compiler's 64-bit long and R5900 shift semantics. */
int func_00119EA8(unsigned long bits) {
    long exponent = (long)((bits << 1) >> 53);
    exponent -= 1075;
    if (exponent < -53) return 0;
    if (exponent >= 13) return 9999;
    bits = (bits << 12) >> 12;
    bits |= 1UL << 52;
    if (exponent < 0) {
        exponent = -exponent;
        bits >>= (int)(exponent - 2);
        if ((bits & 3) == 3) bits = (bits >> 2) + 1;
        else bits >>= 2;
    } else {
        bits <<= (int)exponent;
    }
    return (int)bits;
}

extern void *D_0012FD00;
extern long func_0011E6D8(double);
extern void func_0011A690(const char *, ...);

/* printfloat (libkernl.a:kprintf.o): the kprintf %f formatter. Prints a
   leading '-' through the putchar hook (D_0012FD00) if negative, then
   normalises the magnitude into [0.1, 1.0) by repeated *10 (if it
   started below 0.1) or /10 (if at or above 1.0), counting the power of
   ten removed in exp10, scales the mantissa to an integer (*1000000,
   func_0011E6D8, then ftoi = func_00119EA8) and prints it as "0.%d",
   followed by the exponent as "e+%d" or "e%d" (%d supplies the '-' for
   a negative exponent itself).

   The magnitude is checked, then rescaled, in nested `if (v < 0.1)
   { while (v < 0.1) ... }`: retail keeps a separate 0.1 for each. Its
   three format strings and its constants open this object's .rodata
   (D_00152880-D_001528A8), followed by _printf's jump table
   (jtbl_001528B0): the strings are literals so that the table lands
   16-aligned at its retail address (config/core_rodata.txt). */
void func_00119F38(double v) {
    int exp10 = 0;
    int digits;

    if (v < 0.0) {
        v = -v;
        ((void (*)(int))D_0012FD00)(0x2D);
    }
    if (v < 0.1) {
        while (v < 0.1) {
            v = v * 10.0;
            exp10--;
        }
    } else if (v >= 1.0) {
        while (v >= 1.0) {
            v = v / 10.0;
            exp10++;
        }
    }
    digits = func_00119EA8(func_0011E6D8(v * 1000000.0));
    func_0011A690("0.%d", digits);
    if (exp10 >= 0) {
        func_0011A690("e+%d", exp10);
    } else {
        func_0011A690("e%d", exp10);
    }
}

/* _printf: the SDK libc formatter behind printf/sprintf: walks fmt, takes
   each argument from the 8-byte slots at ap and writes characters through
   the output hook at D_0012FD00, with interrupts held off around it.
   Adapted from Lombyte (GPL-3.0): src/sdk/debug/printf_impl.c, _printf. */

extern s32 DIntr_A0A0(void) __asm__("func_0011D960");
extern s32 EnableInterrupts_A0A0(void) __asm__("func_0011D9A8");
extern void printfloat_A0A0(double) __asm__("func_00119F38");
extern void (*D_0012FD00_A0A0[])(s32) __asm__("D_0012FD00");


int func_0011A0A0(const char *fmt, va_list ap) {
    char buf[32];
    char *pad;
    char *p;
    s32 mod;
    s32 n;
    unsigned long d;
    u8 ch;
    char second;
    s64 v;
    s32 ie;

    ie = DIntr_A0A0();
    for (; *fmt != 0; fmt++) {
        p = (char *)fmt;
        pad = 0;
        mod = 0;
        if (*fmt == '%') {
        again:
            fmt++;
            switch ((char)(*fmt - '0')) {
            case 0:
                n = fmt[1] - '0';
                second = fmt[2];
                if ((u8)n < 10) {
                    if ((u32)(second - '0') < 10) {
                        n = n * 10 + (second - '0');
                        fmt += 2;
                        if (n >= 32) {
                            n = 31;
                        }
                    } else {
                        fmt += 1;
                    }
                    pad = &buf[31 - n];
                    while (n > 0) {
                        buf[31 - n] = '0';
                        n--;
                    }
                    goto again;
                }
                goto again;
            case 'l' - '0':
                mod = 'l';
                goto again;
            case 'h' - '0':
                mod = 'h';
                goto again;
            case 'o' - '0':
                if (mod == 'l') {
                    v = (ap += 8, *(s64 *)(ap - 8));
                } else if (mod == 'h') {
                    v = (ap += 8, *(u16 *)(ap - 8));
                } else {
                    v = (ap += 8, *(u32 *)(ap - 8));
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        *--p = (v & 7) + '0';
                        v = (u64)v >> 3;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_0012FD00_A0A0[0](*p++);
                }
                break;
            case 'x' - '0':
                if (mod == 'l') {
                    v = (ap += 8, *(s64 *)(ap - 8));
                } else if (mod == 'h') {
                    v = (ap += 8, *(u16 *)(ap - 8));
                } else {
                    v = (ap += 8, *(u32 *)(ap - 8));
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        d = v & 0xF;
                        if (d < 10) {
                            *--p = d + '0';
                        } else {
                            *--p = d + 'a' - 10;
                        }
                        v = (u64)v >> 4;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_0012FD00_A0A0[0](*p++);
                }
                break;
            case 'd' - '0':
                if (mod == 'l') {
                    v = (ap += 8, *(s64 *)(ap - 8));
                } else if (mod == 'h') {
                    v = (ap += 8, *(s16 *)(ap - 8));
                } else {
                    v = (ap += 8, *(s32 *)(ap - 8));
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    if (v < 0) {
                        v = -v;
                        D_0012FD00_A0A0[0]('-');
                    }
                    while (v != 0) {
                        *--p = v % 10 + '0';
                        v = v / 10;
                    }
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_0012FD00_A0A0[0](*p++);
                }
                break;
            case 'u' - '0':
                if (mod == 'l') {
                    v = (ap += 8, *(s64 *)(ap - 8));
                } else if (mod == 'h') {
                    v = (ap += 8, *(u16 *)(ap - 8));
                } else {
                    v = (ap += 8, *(u32 *)(ap - 8));
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        *--p = (u64)v % 10 + '0';
                        v = (u64)v / 10;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_0012FD00_A0A0[0](*p++);
                }
                break;
            case 'e' - '0':
            case 'f' - '0': {
                f32 f = (ap += 8, *(f32 *)(ap - 8));
                if (f == 0.0f) {
                    D_0012FD00_A0A0[0]('0');
                } else {
                    printfloat_A0A0(f);
                }
            } break;
            case 's' - '0': {
                char *string = (ap += 8, *(char * *)(ap - 8));
                if (*string == 0) {
                    D_0012FD00_A0A0[0]('(');
                    D_0012FD00_A0A0[0]('n');
                    D_0012FD00_A0A0[0]('u');
                    D_0012FD00_A0A0[0]('l');
                    D_0012FD00_A0A0[0]('l');
                    D_0012FD00_A0A0[0](')');
                } else {
                    p = string;
                    if (*p != 0) {
                        do {
                            D_0012FD00_A0A0[0](*p++);
                        } while (*p != 0);
                    }
                }
                break;
            }
            case 'c' - '0':
                v = (ap += 8, *(char *)(ap - 8));
                D_0012FD00_A0A0[0](v);
                break;
            default:
                break;
            }
        } else {
            D_0012FD00_A0A0[0](*fmt);
        }
    }
    if (ie != 0) {
        EnableInterrupts_A0A0();
    }
}



/* The print core: walks fmt with the va_list, writing through the hook
   D_0012FD00, and returns the count. */
extern int func_0011A0A0(const char *, va_list);

/*
 * A real varargs forwarder: va_start, hand fmt and the va_list to
 * func_0011A0A0, va_end. Its frame is printf's (func_00116078, also
 * 2.9-ee): $ra at 0 and the $5-$11 save area at 0x58, which is where ap
 * points. Exact under 2.9-ee; under the game's 2.95.3 a varargs
 * definition also spills $f12-$f18 (SIZE 72/56), which is why this was
 * once a hand-built `long args[16]` forwarder (13/56).
 */
void func_0011A690(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    func_0011A0A0(fmt, ap);
    va_end(ap);
}

/*
 * The varargs twin of the function above (likely scePrintf): swap the
 * print hook D_0012FD00 for func_00119DC0 (the deci2 console writer),
 * pass fmt and the va_list to func_0011A0A0, restore the hook. Exact
 * under 2.9-ee; under 2.95.3 the frame comes out 0x90 against retail's
 * 0xB0 (the $f12-$f18 save again, as with sprintf, 00116248.c).
 */
extern void func_00119DC0();

int func_0011A6C8(const char *fmt, ...) {
    va_list ap;
    void *saved = D_0012FD00;
    int ret;

    D_0012FD00 = (void *)func_00119DC0;
    va_start(ap, fmt);
    ret = func_0011A0A0(fmt, ap);
    va_end(ap);
    D_0012FD00 = saved;
    return ret;
}

void func_0011A728(void *arg0, void *arg1) {
    int idx = *(int *)((char *)arg0 + 0x10);
    int val = *(int *)((char *)arg0 + 0x14);
    int *arr = *(int **)((char *)arg1 + 0x1C);
    arr[idx] = val;
}

int func_0011A748(void *arg0, void *arg1) {
    int v = *(int *)((char *)arg0 + 0x10);
    *(int *)((char *)arg1 + 0x8) = v;
    return v;
}

extern int D_00155080[];

int func_0011A758(int arg0) {
    return D_00155080[arg0];
}

LINKER_REMNANT("asm/remnants/core_text", func_0011A770);

extern int D_0012FD04;
extern int D_00154F54;
typedef struct { void (*f)(void *, void *); void *arg; } Hnd;
typedef struct {
    void *a0;
    void *a1;
    int a2;
    void *a3;
    int a4;
    int a5;
    int a6;
    int *a7;
} CmdData;
typedef struct { char pad[0xC]; int pad2; void *u10; } CmdPkt;
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern void func_00118D80(int);
extern void func_00118E40(void);
extern int func_00118AB0(int, void *, int);
extern void func_00119460(int);
extern void func_0011AC48(void);
extern unsigned char D_00154E80[];
extern unsigned char D_00154F00[];
extern CmdData D_00154F58;
extern Hnd D_00154F80[];
extern CmdPkt D_00154F40;

void func_0011A780(void) {
    int i;
    unsigned t5, t6;
    func_0011D960();
    if (D_0012FD04 != 0) {
        func_0011D9A8();
        return;
    }
    D_0012FD04 = 1;
    t6 = (unsigned)D_00154E80 | 0x20000000;
    t5 = (unsigned)D_00154F00 | 0x20000000;
    sif_cmd_data.a0 = (void *)t6;
    sif_cmd_data.a1 = (void *)t5;
    sif_cmd_data.a2 = 0;
    sif_cmd_data.a3 = sif_system_handlers;
    sif_cmd_data.a4 = 0x20;
    sif_cmd_data.a5 = 0;
    sif_cmd_data.a6 = 0;
    sif_cmd_data.a7 = D_00155080;
    for (i = 0; i < 32; i++) {
        sif_system_handlers[i].f = 0;
        sif_system_handlers[i].arg = 0;
    }
    for (i = 0; i < 32; i++) {
        D_00155080[i] = 0;
    }
    sif_system_handlers[0].f = (void (*)(void *, void *))func_0011A748;
    sif_system_handlers[1].f = (void (*)(void *, void *))func_0011A728;
    sif_system_handlers[0].arg = &sif_cmd_data;
    sif_system_handlers[1].arg = &sif_cmd_data;
    func_0011D9A8();
    func_00118D80(0);
    if (*(volatile unsigned *)0x1000E010 & 0x20) {
        *(volatile unsigned *)0x1000E010 = 0x20;
    }
    if (!(*(volatile unsigned *)0x1000C000 & 0x100)) {
        func_00118E40();
    }
    D_00154F54 = func_00118AB0(5, func_0011AC48, 0);
    func_00119460(5);
    sif_cmd_data.a2 = ((int (*)(unsigned))func_00118E70)(0x80000000);
    if (sif_cmd_data.a2 != 0) {
        sif_init_packet.u10 = D_00154E80;
        func_0011ABC8(0x80000000, (int)&sif_init_packet, 0x14, 0, 0, 0);
        return;
    }
    do {
    } while (!(func_00118E70(4) & 0x20000));
    sif_cmd_data.a2 = func_00118E70(2);
    func_00118E60(0x80000000, sif_cmd_data.a2);
    func_00118E60(0x80000001, (int)&sif_cmd_data);
    sif_init_packet.u10 = D_00154E80;
    sif_init_packet.pad2 = 0;
    func_0011ABC8(0x80000002, (int)&sif_init_packet, 0x14, 0, 0, 0);
}

extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;

extern int func_001193F8_i(int) __asm__("func_001193F8");
extern int func_00118AD0_i(int, int) __asm__("func_00118AD0");

/* func_001193F8 and func_00118AD0 both return int; their results decide
   which registers the globals' addresses get. */
void func_0011AA00(void) {
    func_001193F8_i(0x5);
    func_00118AD0_i(0x5, D_00154F54);
    D_0012FD04 = 0;
}

extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;

/* Sets slot arg0's two words: D_00154F6C's for arg0 >= 0, D_00154F64's
   below. In func_0011AA68's shape: reassigning arg0 to the table is what
   puts the base in $a0, as in retail. */
void func_0011AA38(int arg0, int arg1, int arg2) {
    int off = arg0 << 3;
    if (arg0 < 0) {
        arg0 = D_00154F64;
    } else {
        arg0 = D_00154F6C;
    }
    off += arg0;
    *(int *)(off + 0) = arg1;
    *(int *)(off + 4) = arg2;
}

void func_0011AA68(int arg0) {
    int offset = arg0 << 3;
    if (arg0 < 0) {
        arg0 = D_00154F64;
    } else {
        arg0 = D_00154F6C;
    }
    offset += arg0;
    *(int *)offset = 0;
}

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
extern u32 D_00154F60[];
extern void func_0011AD70(void *addr, s32 size);
extern s32 func_00118E20();
extern s32 func_00118E30();

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/rpc/sce_sif_send_cmd.c, _sceSifSendCmd. */
s32 func_0011AA90(u32 packet, s32 mode, SifCmd *cmd, s32 size, u32 src, u32 dst, s32 dst_size) {
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
            func_0011AD70(src, dst_size);
        }
    } else {
        cmd->f4 = 0;
        cmd->f0_8 = 0;
    }
    addr0 = D_00154F60[0];
    tags[num].f0 = (u32)cmd;
    tags[num].f4 = addr0;
    tags[num].f8 = size;
    cmd->f0 = size;
    cmd->f8 = packet;
    tags[num].fC = 0x44;
    func_0011AD70(cmd, size);
    num++;
    if (mode & 1) {
        return func_00118E30(tags, num);
    }
    return func_00118E20(tags, num);
}

/* sceSifSendCmd and isceSifSendCmd: forward to the common sender
   func_0011AA90 with the mode (0, or 1 from interrupt context) injected
   as its second argument. EABI passes the first eight integer args in
   $4-$11, so this forwards seven of them.

   Both return the sender's result (it sets $v0 on every path; the packet
   id). That is what keeps 2.9-ee from turning the call into a bare tail
   jump (SIZE 40/60 when void), and its schedule is then retail's: the
   `addiu $sp,$sp,-0x10` fourth, after three argument moves, which
   2.95.3 could not produce (11/60). */

int func_0011ABC8(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    return func_0011AA90(arg0, 0, arg1, arg2, arg3, arg4, arg5);
}

int func_0011AC08(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    return func_0011AA90(arg0, 0x1, arg1, arg2, arg3, arg4, arg5);
}

ASM_FUNC("asm/handwritten/core_text", func_0011AC48);

ASM_FUNC("asm/handwritten/core_text", func_0011AD70);

/*
 * The last word of sceSifWriteBackDCache (func_0011AD70, hand-written): a bare
 * `jr $31` whose delay slot is the next object's first instruction. The SDK's
 * libkernl.a gives the function 176 bytes in sifcmd.o (0x11AD70..0x11AE20),
 * this word included, after its own `last:` label's `jr $31; nop`
 * (docs/ASM_CLASSIFICATION.md).
 */
ASM_FUNC("asm/handwritten/core_text", func_0011AE1C);

typedef struct {
    int pid;
    void *pkt_table;
    int pkt_table_len;
    int unk0C;
    int unk10;
    void *rdata_table;
    int rdata_table_len;
    void *client_table;
    int client_table_len;
    int rdata_table_idx;
} SifRpcData;

extern int D_0012FD08 NOT_SDA;
extern char D_00155100[];
extern char D_00155900[];
extern char D_00156100[];
extern SifRpcData D_00156900_d __asm__("D_00156900");
extern int func_0011D960(void);           /* DI */
extern void func_0011D9A8(void);          /* EI */
extern void func_0011A780(void);          /* sceSifInitCmd */
extern int func_00118E70(int);            /* sceSifGetReg */
extern int func_00118E60(int, int);       /* sceSifSetReg */
extern void func_0011B0E0(char *);
extern void func_0011B248(void *, void *);
extern void func_0011B438(void *, void *);
extern void func_0011B198(void *, void *);

/* sceSifInitRpc: once, set up the RPC tables and command handlers, then
   tell the IOP (SIF register 0x80000002) and wait for it. Adapted from
   Lombyte (GPL-3.0) for PAL. */
void func_0011AE20(int mode) {
    int *packet;

    func_0011D960();
    if (RpcCommandState) {
        func_0011D9A8();
        return;
    }
    RpcCommandState = 1;
    func_0011D9A8();
    func_0011A780();
    func_0011D960();
    D_00156900_d.pkt_table = (void *)((unsigned int)D_00155100 | 0x20000000);
    D_00156900_d.pkt_table_len = 32;
    D_00156900_d.unk0C = 0;
    D_00156900_d.unk10 = 0;
    D_00156900_d.rdata_table = (void *)((unsigned int)D_00155900 | 0x20000000);
    D_00156900_d.rdata_table_len = 32;
    D_00156900_d.client_table = (void *)((unsigned int)D_00156100 | 0x20000000);
    D_00156900_d.client_table_len = 32;
    D_00156900_d.rdata_table_idx = 0;
    D_00156900_d.pid = 1;
    func_0011AA38(0x80000008, (int)func_0011B0E0, (int)&D_00156900_d);
    func_0011AA38(0x80000009, (int)func_0011B248, (int)&D_00156900_d);
    func_0011AA38(0x8000000A, (int)func_0011B438, (int)&D_00156900_d);
    func_0011AA38(0x8000000C, (int)func_0011B198, (int)&D_00156900_d);
    func_0011D9A8();
    if (func_00118E70(0x80000002)) {
        return;
    }
    packet = (int *)(D_00155100 + 64);
    packet[3] = 1;
    func_0011ABC8(0x80000002, (int)packet, 16, 0, 0, 0);
    while (func_0011A758(0) == 0) {
    }
    func_00118E60(0x80000002, 1);
}

extern void func_0011AA00(void);

void func_0011AFC0(void) {
    func_0011AA00();
    RpcCommandState = 0;
}

extern int func_0011D960(void);
extern void func_0011D9A8(void);

/* Claim the first free 0x40-byte slot in the table {next_id, base,
   count} at arg0, with interrupts held off across the search. A claimed
   slot records its index and state in +0x10, points +0x14 at itself and
   takes the next id; id 0 is skipped, so after handing out 1 the
   counter jumps straight to 2. Returns the slot, or 0 if the table is
   full.

   Two shapes mattered. `i` must be assigned AFTER the func_0011D960
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
void *func_0011AFE8(void *arg0) {
    char *a = (char *)arg0;
    int n;
    char *p;
    int i;
    int id;
    int v;

    func_0011D960();
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
            func_0011D9A8();
            return p;
        }
        i++;
        p += 0x40;
    }
    func_0011D9A8();
    return 0;
}

void func_0011B090(void *arg0) {
    char *self = (char *)arg0;
    unsigned int flags = *(unsigned int *)(self + 0x10);
    *(int *)(self + 0x18) = 0;
    *(unsigned int *)(self + 0x10) = flags & 0xFFFFFFFEu;
}

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
 * allocation. Same open scratch-register question as func_001160D8.
 */
/* Hand out the next 0x40-byte slot of the ring {.., +0x14 base, +0x18
   count, .., +0x24 next}: take next modulo count, advance next past it,
   return the slot. The slot address has to be formed BEFORE the store of
   the new index: that is what gives retail's allocation (remainder in $v0,
   divisor in $v1) and its tail, `addiu` before `sll` and the `addu` in the
   jr delay slot. Storing first and indexing in the return is 20/48. */
/* The slot address is computed into a local before next = i + 1 is
   stored. */
void *func_0011B0B0(void *arg0) {
    char *p = (char *)arg0;
    int i = *(int *)(p + 0x24) % *(int *)(p + 0x18);
    char *s = *(char **)(p + 0x14) + i * 0x40;
    *(int *)(p + 0x24) = i + 1;
    return s;
}

/*
 * sifrpc's _request_end: the completion handler for a request block. Two
 * command ids do extra work before the common teardown: 0x8000000A (RPC
 * call) runs the client's end callback, 0x80000009 (RPC bind) copies the
 * server, buff and cbuff back into it. Then, whatever the id was,
 * iSignalSema the client's semaphore if it has one, free its packet
 * (func_0011B090) and clear the pointer. Exact under both compilers.
 *
 * Two levers (build-sn/try/func_0011B0E0/RESULT.md):
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
extern int func_00118CA0(int);

void func_0011B0E0(char *p) {
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
        func_00118CA0(h);
    }
    func_0011B090(*(void **)o);
    *(int *)o = 0;
}

/* _request_rdata (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_RDATA (0x8000000c). Grabs a free response packet
   (func_0011B0B0 = _rpc_get_fpacket), copies the requester's pkt_addr
   and recvbuf pointer (rdata's words 5 and 7) into the response, tags
   the response SIF_CMD_RPC_RDATA (word 8), then forwards the
   requester's src/dest/size (words 8/9/10) to the IOP as
   isceSifSendCmd(SIF_CMD_RPC_END, ...) (func_0011AC08). Void: the
   trailing call is a tail jump. Both copied fields have to be read into
   their own locals before either store: that is what puts pkt_addr in
   $a1 and recvbuf in $a0, matching retail (any other shape -- inline
   reads, one field hoisted, or reversed declaration order -- swaps that
   allocation or reloads recvbuf late). */
void func_0011B198(void *rdata, void *data) {
    int *r = (int *)rdata;
    int *rend = (int *)func_0011B0B0(data);
    int pkt_addr = r[5];
    int recvbuf = r[7];

    rend[5] = pkt_addr;
    rend[7] = recvbuf;
    rend[8] = 0x8000000C;
    func_0011AC08(0x80000008, (int)rend, 0x40, r[8], r[9], r[10]);
}

void *func_0011B1F8(int key, void *arg1) {
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

/* _request_bind (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_BIND (0x80000009). Grabs a free response packet
   (func_0011B0B0 = _rpc_get_fpacket), copies the requester's pkt_addr
   and cd (bind's words 5 and 7) into it, tags it SIF_CMD_RPC_BIND,
   looks up the requested service id (bind's word 8, search_svdata =
   func_0011B1F8) and fills in the server/buf/cbuf fields (words 9-11)
   from it if found, zeroing them otherwise, then forwards to the IOP as
   isceSifSendCmd(SIF_CMD_RPC_END, ...) (func_0011AC08). Matches
   ps2sdk's _request_bind field-for-field (sifrpc.c).

   Two shapes mattered: pkt_addr and cd need their own locals declared
   in that order (pkt_addr first) even though cd is stored first --
   reversed, the two loads swap registers (func_0011B198's lever again);
   and the not-found case has to be the `if` arm, sd-found the `else`
   (retail branches on `sd == 0` being false, i.e. `bnel`; writing the
   found case first inverts the branch, as in func_0011B438). */
void func_0011B248(void *bind, void *data) {
    int *b = (int *)bind;
    int *rend = (int *)func_0011B0B0(data);
    int pkt_addr = b[5];
    int cd = b[7];
    int *sd;

    rend[7] = cd;
    rend[5] = pkt_addr;
    rend[8] = 0x80000009;
    sd = (int *)func_0011B1F8(b[8], data);
    if (sd == 0) {
        rend[9] = 0;
        rend[10] = 0;
        rend[11] = 0;
    } else {
        rend[9] = (int)sd;
        rend[10] = sd[2];
        rend[11] = sd[5];
    }
    func_0011AC08(0x80000008, (int)rend, 0x40, 0, 0, 0);
}

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

extern char D_00156900[];
extern int func_00118C70(void *);   /* CreateSema */
extern void func_00118C80(int);     /* DeleteSema */
extern int func_00118CB0(int);      /* WaitSema */

/* sceSifBindRpc. Adapted from Lombyte (GPL-3.0) for PAL. */
int func_0011B2F8(void *client, int arg1, int arg2) {
    BindClient *arg0 = client;
    int sema_param[6];
    int temp_2_31;
    BindPacket *temp_2_15;

    arg0->unk10 = 0;
    arg0->unk24 = 0;
    temp_2_15 = (BindPacket *)func_0011AFE8(D_00156900);
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
    temp_2_31 = func_00118C70(sema_param);
    arg0->unk8 = temp_2_31;
    if (temp_2_31 >= 0) {
        goto block_4;
    }
    func_0011B090(temp_2_15);
    return -3;
block_4:
    if (func_0011ABC8(0x80000009, (int)temp_2_15, 0x40, 0, 0, 0) != 0) {
        goto block_6;
    }
    func_0011B090(temp_2_15);
    func_00118C80(arg0->unk8);
    return -2;
block_6:
    func_00118CB0(arg0->unk8);
    func_00118C80(arg0->unk8);
    return 0;
block_7:
    arg0->unk8 = -1;
    if (func_0011ABC8(0x80000009, (int)temp_2_15, 0x40, 0, 0, 0) != 0) {
        return 0;
    }
    func_0011B090(temp_2_15);
    return -2;
}

extern void func_00119678(int thread_id);

/* _request_call (libkernl.a:sifrpc.o): the sifcmd system handler for
   SIF_CMD_RPC_CALL (0x8000000a). Queues the request's server object
   (request->sd, word 13) onto its data queue's (sd->base, word 16)
   linked list of pending calls -- append to the current end if the
   queue is non-empty (base->start, word 3), else start a new list --
   then copies the request's fields into the server object (pkt_addr,
   client/cd, rpc_number, size/send_size, recvbuf, rsize/recv_size,
   rmode, rid/rec_id), and finally wakes the queue's worker thread
   (iWakeupThread, func_00119678) unless it is already running or has
   none. Matches ps2sdk's _request_call field-for-field (sifrpc.c).

   Three shapes mattered: the empty-queue arm has to come first (retail
   tests and branches on it, not on the append case); and the first two
   copied fields (pkt_addr, client) need their own locals, both declared
   right there (not hoisted to the top, which pulls their loads into the
   queue-splice code) and both declared before either store -- the same
   allocator lever as func_0011B198 -- or the pair's load/store order
   swaps or its two registers trade places. */
void func_0011B438(void *request, void *data) {
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
    func_00119678(base[0]);
}

typedef unsigned char u8_1B4C8;
typedef signed int s32_1B4C8;
struct SifRpcClient_1B4C8
{
  volatile s32_1B4C8 packet;
  volatile s32_1B4C8 request_id;
    s32_1B4C8 semaphore_id;
    u8_1B4C8 pad_C[0x8];
    s32_1B4C8 server_buffer;
    u8_1B4C8 pad_18[0x4];
    s32_1B4C8 end_callback;
  volatile s32_1B4C8 end_argument;
    s32_1B4C8 server;
};
struct SifRpcPacket_1B4C8 {
    u8_1B4C8 pad_0[0x14];
    s32_1B4C8 packet_address;
    s32_1B4C8 request_id;
    s32_1B4C8 client;
    s32_1B4C8 rpc_number;
    s32_1B4C8 send_size;
    s32_1B4C8 receive_address;
    s32_1B4C8 receive_size;
    s32_1B4C8 completion_mode;
    s32_1B4C8 server;
};
extern u8_1B4C8 D_00156900_1B4C8[] __asm__("D_00156900");
extern s32_1B4C8 func_00118C70();
extern s32_1B4C8 func_00118C80_1B4C8() __asm__("func_00118C80");
extern s32_1B4C8 func_00118CB0();
extern s32_1B4C8 func_0011AFE8_1B4C8() __asm__("func_0011AFE8");
extern s32_1B4C8 func_0011B090_1B4C8() __asm__("func_0011B090");
extern s32_1B4C8 func_0011ABC8();
extern s32_1B4C8 func_0011AD70_1B4C8() __asm__("func_0011AD70");

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/rpc/sce_sif_call_rpc.c, sceSifCallRpc. */
s32_1B4C8 func_0011B4C8_r(struct SifRpcClient_1B4C8 *client, s32_1B4C8 rpc_number, s32_1B4C8 mode, s32_1B4C8 send_buf, s32_1B4C8 send_size,
                          s32_1B4C8 recv_buf, s32_1B4C8 recv_size, s32_1B4C8 end_func, s32_1B4C8 end_data) __asm__("func_0011B4C8");
s32_1B4C8 func_0011B4C8_r(struct SifRpcClient_1B4C8 *client, s32_1B4C8 rpc_number, s32_1B4C8 mode, s32_1B4C8 send_buf, s32_1B4C8 send_size,
                          s32_1B4C8 recv_buf, s32_1B4C8 recv_size, s32_1B4C8 end_func, s32_1B4C8 end_data) {
    s32_1B4C8 semaphore_parameters[6];
    struct SifRpcPacket_1B4C8 *packet;
    s32_1B4C8 semaphore_id;
    s32_1B4C8 skip_cache_writeback;
    s32_1B4C8 request_id;
    s32_1B4C8 allocation_failure_result;
    packet = (struct SifRpcPacket_1B4C8 *)func_0011AFE8_1B4C8(D_00156900_1B4C8);
    allocation_failure_result = -1;
    if (packet == 0) {
        return allocation_failure_result;
    }
    request_id = packet->request_id;
    skip_cache_writeback = mode & 2;
    client->end_argument = end_data;
    client->packet = (s32_1B4C8)packet;
    client->request_id = request_id;
    client->end_callback = end_func;
    packet->rpc_number = rpc_number;
    packet->send_size = send_size;
    packet->receive_address = recv_buf;
    packet->receive_size = recv_size;
    packet->packet_address = (s32_1B4C8)packet;
    packet->server = client->server;
    packet->client = (s32_1B4C8)client;
    if (!skip_cache_writeback) {
        if (send_buf == recv_buf) {
            func_0011AD70_1B4C8(send_buf, (send_size >= recv_size) ? (send_size) : (recv_size));
        } else {
            if (send_size > 0) {
                func_0011AD70_1B4C8(send_buf, send_size);
            }
            if (recv_size > 0) {
                func_0011AD70_1B4C8(recv_buf, recv_size);
            }
        }
    }
    if (mode & 1) {
        if (end_func == 0) {
            packet->completion_mode = 0;
        } else {
            packet->completion_mode = 1;
        }
        client->semaphore_id = -1;
        if (func_0011ABC8(0x8000000A, packet, 0x40, send_buf, client->server_buffer, send_size) != 0) {
            return 0;
        } else {
            func_0011B090_1B4C8(packet);
            return -2;
        }
    } else {
        semaphore_parameters[1] = 1;
        semaphore_parameters[2] = 0;
        semaphore_id = func_00118C70(semaphore_parameters);
        client->semaphore_id = semaphore_id;
        if (semaphore_id < 0) {
            func_0011B090_1B4C8(packet);
            return -3;
        }
        packet->completion_mode = 1;
        if (func_0011ABC8(0x8000000A, packet, 0x40, send_buf, client->server_buffer, send_size) == 0) {
            func_00118C80_1B4C8(client->semaphore_id);
            func_0011B090_1B4C8(packet);
            return -2;
        }
        func_00118CB0(client->semaphore_id);
        func_00118C80_1B4C8(client->semaphore_id);
        return 0;
    }
}

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
int func_0011B6B8(void *arg0) {
    char *p = (char *)arg0;
    char *q = *(char **)p;

    if (q == 0 || *(int *)(p + 4) != *(int *)(q + 0x18) ||
        !(*(int *)(q + 0x10) & 1)) {
        return 0;
    }
    return 1;
}

LINKER_REMNANT("asm/remnants/core_text", func_0011B6F8);

extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;

/*
 * One-shot init: if D_0012FDA0 is still -1, fill a semaphore descriptor
 * (the kernel's SemaParam: initCount [2] = 1, maxCount [1] = 1, option
 * [5] = 0) and create two semaphores from it via func_00118C70
 * (CreateSema), storing them into D_0012FDA0 and D_0012FDA4.
 *
 * Under 2.9-ee the store order decides which store lands in the first
 * call's delay slot: written initCount, maxCount, option it is exact
 * (retail puts buf[2] in the slot). Under the game's 2.95.3 no order was
 * exact (8/92 at best), which is what kept this one open.
 */
void func_0011B710(void) {
    int buf[8];
    if (D_0012FDA0 == -1) {
        buf[2] = 1;
        buf[1] = 1;
        buf[5] = 0;
        D_0012FDA0 = func_00118C70(buf);
        D_0012FDA4 = func_00118C70(buf);
    }
}

extern char D_00157E80[];

/*
 * Claim the first free slot of the 32-entry, 0x10-byte table D_00157E80
 * -- the table func_0011B7F8 below indexes -- under the
 * func_00118CB0/func_00118C90 lock (WaitSema/SignalSema). A slot is free
 * when its +4 word is zero; claiming it writes 0x10000000 there and
 * returns the slot, and a full table returns 0. Both exits drop the lock
 * first.
 *
 * Exact under 2.9-ee with the C that 2.95.3 compiled one instruction
 * short: retail spends a third callee-saved register ($17 holds
 * %hi(D_0012FDA0) for the whole function), which is 2.9-ee's allocation;
 * 2.95.3 parked the %hi in $4 instead.
 */
void *func_0011B770(void) {
    char *p;

    func_0011B710();
    func_00118CB0(D_0012FDA0);
    for (p = D_00157E80; p < D_00157E80 + 0x200; p += 0x10) {
        if (*(int *)(p + 0x4) == 0) {
            *(int *)(p + 0x4) = 0x10000000;
            func_00118C90(D_0012FDA0);
            return p;
        }
    }
    func_00118C90(D_0012FDA0);
    return 0;
}

/* The out-of-range guard comes first and returns 0: 2.9-ee then branches
   to the in-range block on the true side of the sltiu, as retail does
   (bnez). The in-range-first if/else that matched under 2.95.3 is laid
   out the other way round by 2.9-ee. */
void *func_0011B7F8(unsigned int arg0) {
    void *ret;
    func_0011B710();
    func_00118CB0(D_0012FDA0);
    if (arg0 >= 0x20) {
        func_00118C90(D_0012FDA0);
        return 0;
    }
    ret = &D_00157E80[arg0 * 0x10];
    func_00118C90(D_0012FDA0);
    return ret;
}

typedef struct {
    s32 v;
} __attribute__((packed)) Word;

typedef struct {
    Word ret;
    Word func;
    Word dest;
    Word size;
} FsRcvHead;

typedef struct {
    void *dest;
} __attribute__((packed)) FsRcvPtr;

typedef struct {
    Word dest;
    Word size;
} FsRcvBuf;

typedef struct {
    s32 n1;
    s32 n2;
    u8 *d1;
    u8 *d2;
    u8 buf1[0x40];
    u8 buf2[0x40];
} FsRcvRead;

typedef struct {
    u8 b[0x144];
} __attribute__((packed)) FsDirent;

typedef struct {
    u8 b[0x40];
} __attribute__((packed)) FsStat;

extern volatile s32 D_0012FD10[];
extern u32 D_0012FD90[];
extern s32 D_0012FD98[];
extern u8 D_00157600[];
extern void *func_00115248(void *, const void *, u32);

void func_0011B868(s32 *arg) {
    FsRcvHead h;
    FsRcvHead *src;
    FsRcvBuf *bs;
    union {
        FsRcvPtr p;
        FsRcvBuf b;
    } u;
    FsRcvRead *r;
    u8 *pkt;
    u8 *base;
    u8 *d;
    u32 *bank = D_0012FD90;
    s32 idx;
    s32 i;
    s64 ret;
    u32 n;
    u32 addr;

    idx = D_0012FD98[0] != 0 ? arg[3] : 0;
    *bank = idx;
    base = D_00157600;
    addr = (u32)base;
    addr += (u32)idx * 0x440;
    pkt = (u8 *)(addr | 0x20000000);
    src = (FsRcvHead *)pkt;
    h.ret = src->ret;
    h.func = src->func;
    h.dest = src->dest;
    h.size = src->size;
    if (h.ret.v >= 0) {
        func_00115248((void *)h.dest.v, pkt + 0x10, h.size.v);
    }
    switch (h.func.v) {
    case 2:
        r = (FsRcvRead *)(pkt + 0x14);
        if (r->n1 > 0) {
            d = r->d1;
            for (i = 0; i < r->n1; i++) {
                d[i] = r->buf1[i];
            }
        }
        if (r->n2 > 0) {
            d = r->d2;
            for (i = 0; i < r->n2; i++) {
                d[i] = r->buf2[i];
            }
        }
        break;
    case 11:
        u.p = *(FsRcvPtr *)(pkt + 0x14);
        *(FsDirent *)u.p.dest = *(FsDirent *)(pkt + 0x18);
        break;
    case 12:
        u.p = *(FsRcvPtr *)(pkt + 0x14);
        *(FsStat *)u.p.dest = *(FsStat *)(pkt + 0x18);
        break;
    case 23:
    case 25:
    case 26:
        bs = (FsRcvBuf *)(pkt + 0x14);
        u.b.dest = bs->dest;
        u.b.size = bs->size;
        n = u.b.size.v;
        if (n > 0x400) {
            u.b.size.v = 0x400;
            n = 0x400;
        }
        func_00115248((void *)u.b.dest.v, pkt + 0x1C, n);
        break;
    }
    if (h.ret.v < 0) {
        u32 semaphore = 0u - (u32)h.ret.v;
        ret = (s32)semaphore;
        h.ret.v = (s32)semaphore;
        for (i = 0; i < 32; i++) {
            if (((s32 *)D_0012FD10)[i] == ret) {
                D_0012FD10[i] = -1;
                break;
            }
        }
        return;
    }
    func_00118CA0(h.ret.v);
}

extern int D_0012FD9C;
extern int func_00118C70(void *);

void func_0011BBF0(void) {
    int buf[8];
    if (D_0012FD9C == -1) {
        buf[5] = 0;
        buf[2] = 1;
        buf[1] = 1;
        D_0012FD9C = func_00118C70(buf);
    }
}

extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;

int func_0011BC40(void) {
    func_0011BBF0();
    func_00118CB0(D_0012FD9C);
    return 0;
}

/* A tail jump with the argument load in its delay slot (`lui / j /
   lw`), as 2.9-ee, which builds this SDK object, emits it. */
void func_0011BC70(void) {
    func_00118C90(D_0012FD9C);
}

ASM_FUNC("asm/handwritten/core_text", func_0011BC80);

/* CD/DVD init step: install the two RPC-side handlers, bind the RPC server
 * at 0x80000001 (retrying until it has an address), clear the 32-slot table
 * D_00157E80, then send command 0xFF and copy back the reply word and the
 * flag that says whether the reply was 2. Returns 0 on success, -1 when the
 * bind failed, 0xFFFEFFFF when the RPC failed. */
struct RpcCb { void (*cb)(void *); void *data; };
extern struct RpcCb D_00158100;
extern char D_001580C0[];
extern char D_00158080[];
extern char D_001575C0[];
extern char *D_00156940[];
extern char D_001580A8[];
extern int D_0012FD94;
extern void func_0011B868();
extern void func_0011BC80();

struct W4 { char b[4]; };
s32 func_0011BCB0(void) {
    s32 r, i;
    char *p;
    struct W4 w;
    s32 *q;

    struct RpcCb *rq = &D_00158100;
    func_0011AE20(0);
    D_00158100.cb = 0;
    rq->data = 0;
    r = func_0011D960();
    func_0011AA38(0x80000011, (int)func_0011B868, (int)D_001580C0);
    func_0011AA38(0x80000013, (int)func_0011BC80, (int)rq);
    if (r != 0) {
        func_0011D9A8();
    }
    for (;;) {
        r = func_0011B2F8(D_00158080, 0x80000001, 0);
        if (r < 0) {
            return -1;
        }
        if (*(s32 *)(D_00158080 + 0x24) != 0) {
            break;
        }
        for (i = 0x100000; i != -1; i--) {
        }
    }
    func_0011B710();
    func_00118CB0(D_0012FDA0);
    for (p = D_00157E80; p < D_00157E80 + 0x200; p += 0x10) {
        *(s32 *)(p + 4) = 0;
    }
    func_00118C90(D_0012FDA0);
    D_00156940[0] = D_00157600;
    D_00156940[1] = D_00157600 + 0x440;
    r = func_0011B4C8(D_00158080, 0xFF, 0, D_00156940, 8, D_001575C0, 8, 0, 0);
    if (r < 0) {
        return (s32)0xFFFEFFFF;
    }
    *(struct W4 *)FsResetBuffer = *(struct W4 *)((u32)D_001575C0 | 0x20000000);
    w = *(struct W4 *)((u32)(D_001575C0 + 4) | 0x20000000);
    D_0012FD98[0] = (*(s32 *)&w == 2);
    FsResetState = 1;
    return 0;
}

extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;

/* Three-way compare-and-confirm: the slot D_001580A8 has to agree with
   the scratch buffer D_0012FCEC, then with the handle D_0012FDA8, and
   finally the two of those with each other. Only the last comparison's
   result is reported, as a 0/1.

   The two globals are bound to locals, buffer first, purely to fix the
   order the compiler materialises their %hi halves in: used directly,
   the slot's lui comes first and the function is 4/140. Declaration
   order of the locals decides it. */
int func_0011BEB8(void) {
    char *buf = D_0012FCEC;
    char *slot = FsResetBuffer;
    int r = 0;

    if (func_001151B4(slot, buf, 4) != 0 &&
        func_001151B4(slot, D_0012FDA8, 4) != 0) {
        r = func_001151B4(buf, D_0012FDA8, 4) != 0;
    }
    return r;
}

extern void func_001153FC(void *, int, int);
extern int D_0012FD94;

int func_0011BF48(void) {
    FsResetState = 0;
    func_001153FC(FsResetBuffer, 0, 0x4);
    return 0;
}

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
extern s32 D_0012FD94_1BF80[] __asm__("D_0012FD94");
extern s32 D_0012FDA0_1BF80[] __asm__("D_0012FDA0");
extern struct FsOpenRequest D_00156980_1BF80 __asm__("D_00156980");
extern u8 D_001575C0_1BF80[] __asm__("D_001575C0");
extern struct SifFileSlot D_00157E80_1BF80[] __asm__("D_00157E80");
extern u8 D_00158080_1BF80[] __asm__("D_00158080");
extern s32 func_00118C70_1BF80(struct SemaphoreParameters *) __asm__("func_00118C70");
extern s32 func_00118C80_1BF80(s32) __asm__("func_00118C80");
extern s32 func_0011BC70_1BF80(void) __asm__("func_0011BC70");
extern s32 func_00118C90(s32);
extern s32 func_00118CB0(s32);
extern s32 func_0011BC40_1BF80(s32) __asm__("func_0011BC40");
extern s32 func_0011BEB8(void);
extern struct SifFileSlot *func_0011B770_1BF80(void) __asm__("func_0011B770");
extern s32 func_0011BCB0(void);
extern s32 func_0011B4C8(void *, s32, s32, void *, s32, void *, s32, void *, void *);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/library/sceopen.c, sceOpen. */
s32 func_0011BF80(const u8 *path, s32 flags, ...)
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

    request = &D_00156980_1BF80;
    /* The six remaining EE argument registers occupy eight bytes each. */
    arguments = __builtin_next_arg(flags) - 0x30;
    func_0011BC40_1BF80(0);
    if (D_0012FD94_1BF80[0] == 0) {
        func_0011BCB0();
    }
    if (func_0011BEB8() != 0) {
        func_0011BC70_1BF80();
        return -0x10004;
    }
    file_slot = func_0011B770_1BF80();
    if (file_slot == 0) {
        func_0011BC70_1BF80();
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
    slot_index = file_slot - D_00157E80_1BF80;
    request->flags = (s32) (flags & 0x6FFFFFFF);
    request->mode = mode;
    semaphore_parameters.max_count = 1;
    request->slot = slot_index;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = func_00118C70_1BF80(&semaphore_parameters);
    request->result = &result;
    request->completion_semaphore = completion_semaphore;
    request->result_size = 4;
    if (func_0011B4C8(D_00158080_1BF80, 0, 0, &D_00156980_1BF80, 0x418, D_001575C0_1BF80, 4, 0, 0) < 0) {
        func_00118C80_1BF80(completion_semaphore);
        func_0011BC70_1BF80();
        return -0xB;
    }
    return_value = *(u32 *)((u32) D_001575C0_1BF80 | 0x20000000);
    func_0011BC70_1BF80();
    if (return_value == 0) {
        func_00118C80_1BF80(completion_semaphore);
        return -0xB;
    }
    func_00118CB0(completion_semaphore);
    func_00118C80_1BF80(completion_semaphore);
    if (result < 0) {
        func_00118CB0(D_0012FDA0_1BF80[0]);
        file_slot->flags = 0;
        func_00118C90(D_0012FDA0_1BF80[0]);
        return result;
    }
    return_value = slot_index;
    func_00118CB0(D_0012FDA0_1BF80[0]);
    slot_mutex = D_0012FDA0_1BF80[0];
    file_slot->fd = result;
    file_slot->flags = (s32) (file_slot->flags | flags);
    func_00118C90(slot_mutex);
    return return_value;
}

typedef struct {
    int handle;
    int active;
} CloseIob;

typedef struct {
    int sema;
    int result;
    int size;
    int handle;
    int slot;
} CloseState;

typedef struct {
    int count;
    int maxCount;
    int initialCount;
    int waitThreads;
    int attr;
    int option;
} CloseSema;

extern CloseState D_00156980;
extern char D_001575C0[];
extern char D_00158080[];
extern int func_00118CB0(int);
extern int func_0011B4C8();
extern int func_0011BC40_arg(int) __asm__("func_0011BC40");

/* sceClose: close a SIF file handle, adapted from Lombyte (GPL-3.0) for PAL. */
int func_0011C208(unsigned int fd) {
    CloseSema sema;
    int result;
    int sid;
    int rpcResult;
    CloseIob *iob;
    CloseState *state = &D_00156980;

    iob = func_0011B7F8(fd);
    func_0011BC40_arg(1);
    if (FsResetState == 0) {
        func_0011BC70();
        return -1;
    }
    if (iob == 0 || iob->active == 0) {
        func_0011BC70();
        return -9;
    }
    state->handle = iob->handle;
    sema.maxCount = 1;
    state->slot = ((char *)iob - D_00157E80) >> 4;
    sema.initialCount = 0;
    sema.option = 0;
    sid = func_00118C70(&sema);
    state->sema = sid;
    state->result = (int)&result;
    state->size = 4;
    rpcResult = func_0011B4C8(D_00158080, 1, 0, state, 0x14,
                                D_001575C0, 4, 0, 0);
    if (rpcResult < 0) {
        func_00118C80(sid);
        func_0011BC70();
        return -11;
    }
    iob->active = 0;
    rpcResult = *(int *)((unsigned int)D_001575C0 | 0x20000000);
    func_0011BC70();
    if (rpcResult == 0) {
        func_00118C80(sid);
        return -11;
    }
    func_00118CB0(sid);
    func_00118C80(sid);
    if (result < 0) {
        return result;
    }
    return 0;
}

struct FsSeekRequest {
    s32 completion_semaphore;
    void *result;
    s32 result_size;
    s32 fd;
    s32 offset;
    s32 origin;
    s32 slot;
};
struct SemaphoreParameters2 {
    s32 count;
    s32 max_count;
    s32 init_count;
    s32 wait_threads;
    u32 attr;
    u32 option;
};
extern struct FsSeekRequest D_00156980_1C388 __asm__("D_00156980");
extern volatile s32 D_0012FD10[];
extern s32 D_0012FDA4_1C388[] __asm__("D_0012FDA4");
struct SifClient {
    u8 pad[0x28];
};
extern struct SifClient D_00158080_1C388 __asm__("D_00158080");
extern struct SifFileSlot *func_0011B7F8_1C388(s32 fd) __asm__("func_0011B7F8");
extern s32 func_0011BC40_1C388(s32) __asm__("func_0011BC40");
extern s32 func_0011BC70_1C388(void) __asm__("func_0011BC70");
extern s32 func_00118C70_1C388(struct SemaphoreParameters2 *) __asm__("func_00118C70");
extern s32 func_00118C80_1C388(s32) __asm__("func_00118C80");

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/library/sce_lseek.c, sceLseek. */
s32 func_0011C388(s32 fd, s32 offset, s32 origin)
{
    struct FsSeekRequest *request;
    struct SifFileSlot *file_slot;
    struct SemaphoreParameters2 semaphore_parameters;
    s32 result;
    s32 flags;
    s32 completion_semaphore;
    s32 async_index;
    volatile s32 *async_slot;
    s32 rpc_result;

    request = &D_00156980_1C388;
    file_slot = func_0011B7F8_1C388(fd);
    func_0011BC40_1C388(4);
    if (D_0012FD94_1BF80[0] == 0) {
        func_0011BC70_1C388();
        return -1;
    }
    if (file_slot == 0 || (flags = file_slot->flags) == 0) {
        func_0011BC70_1C388();
        return -9;
    }
    request->fd = file_slot->fd;
    request->offset = offset;
    request->origin = origin;
    request->slot = file_slot - D_00157E80_1BF80;
    semaphore_parameters.max_count = 1;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = func_00118C70_1C388(&semaphore_parameters);
    request->result = &result;
    request->result_size = 4;
    D_00156980_1C388.completion_semaphore = completion_semaphore;
    if ((s16)flags & 0x8000) {
        func_00118CB0(D_0012FDA4_1C388[0]);
        for (async_index = 0; async_index < 32; async_index++) {
            volatile s32 *async_semaphores = D_0012FD10;

            async_slot = async_semaphores + async_index;
            if (*async_slot == -1) {
                *async_slot = request->completion_semaphore;
                request->completion_semaphore = -request->completion_semaphore;
                break;
            }
        }
        func_00118C90(D_0012FDA4_1C388[0]);
    }
    if (func_0011B4C8(&D_00158080_1C388, 4, 0, &D_00156980_1C388, 0x1C, D_001575C0_1BF80, 4, 0, 0) < 0) {
        func_00118C80_1C388(completion_semaphore);
        func_0011BC70_1C388();
        return -11;
    }
    rpc_result = *(s32 *)((u32)D_001575C0_1BF80 | 0x20000000);
    func_0011BC70_1C388();
    if (rpc_result == 0) {
        func_00118C80_1C388(completion_semaphore);
        return -11;
    }
    if (flags & 0x8000) {
        func_00118C80_1C388(completion_semaphore);
        return 0;
    }
    func_00118CB0(completion_semaphore);
    func_00118C80_1C388(completion_semaphore);
    return result;
}

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
extern struct FsReadRequest D_00156980_1C5C0 __asm__("D_00156980");
extern struct SifFileSlot *func_0011B7F8_1C5C0(s32 fd) __asm__("func_0011B7F8");
extern s32 func_0011BC40_1C5C0(s32) __asm__("func_0011BC40");
extern s32 func_0011BC70_1C5C0(void) __asm__("func_0011BC70");
extern s32 func_00118C70_1C5C0(struct SemaphoreParameters2 *) __asm__("func_00118C70");
extern s32 func_00118C80_1C5C0(s32) __asm__("func_00118C80");
extern void func_0011AD70(void *, s32);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/library/sceRead.c, sceRead. */
s32 func_0011C5C0(s32 fd, void *buffer, s32 length)
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

    request = &D_00156980_1C5C0;
    file_slot = func_0011B7F8_1C5C0(fd);
    func_0011BC40_1C5C0(2);
    if (D_0012FD94_1BF80[0] == 0) {
        func_0011BC70_1C5C0();
        return -1;
    }
    if (file_slot == 0 || (flags = file_slot->flags) == 0) {
        func_0011BC70_1C5C0();
        return -9;
    }
    request->fd = file_slot->fd;
    semaphore_parameters.max_count = 1;
    request->slot = file_slot - D_00157E80_1BF80;
    request->buffer = buffer;
    request->length = length;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = func_00118C70_1C5C0(&semaphore_parameters);
    request->result = &result;
    request->result_size = 4;
    D_00156980_1C5C0.completion_semaphore = completion_semaphore;
    if ((s16)flags & 0x8000) {
        func_00118CB0(D_0012FDA4_1C388[0]);
        for (async_index = 0; async_index < 32; async_index++) {
            volatile s32 *async_semaphores = D_0012FD10;

            async_slot = async_semaphores + async_index;
            if (*async_slot == -1) {
                *async_slot = request->completion_semaphore;
                request->completion_semaphore = -request->completion_semaphore;
                break;
            }
        }
        func_00118C90(D_0012FDA4_1C388[0]);
    }
    if (!(flags & 0x20000000)) {
        func_0011AD70(buffer, length);
    }
    func_0011AD70(request, 0x20);
    if (func_0011B4C8(&D_00158080_1C388, 2, 0, &D_00156980_1C5C0, 0x20, D_001575C0_1BF80, 4, 0, 0) < 0) {
        func_00118C80_1C5C0(completion_semaphore);
        func_0011BC70_1C5C0();
        return -11;
    }
    rpc_result = *(s32 *)((u32)D_001575C0_1BF80 | 0x20000000);
    func_0011BC70_1C5C0();
    if (rpc_result == 0) {
        func_00118C80_1C5C0(completion_semaphore);
        return -11;
    }
    if (flags & 0x8000) {
        func_00118C80_1C5C0(completion_semaphore);
        return 0;
    }
    func_00118CB0(completion_semaphore);
    func_00118C80_1C5C0(completion_semaphore);
    return result;
}

struct FsWriteRequest {
    s32 completion_semaphore;
    void *result;
    s32 result_size;
    s32 fd;
    void *buffer;
    s32 length;
    s32 prefix_length;
    u8 prefix[0x10];
    s32 slot;
};
extern struct FsWriteRequest D_00156980_1C820 __asm__("D_00156980");
extern struct SifFileSlot *func_0011B7F8_1C820(s32 fd) __asm__("func_0011B7F8");
extern s32 func_0011BC40_1C820(s32) __asm__("func_0011BC40");
extern s32 func_0011BC70_1C820(void) __asm__("func_0011BC70");
extern s32 func_00118C70_1C820(struct SemaphoreParameters2 *) __asm__("func_00118C70");
extern s32 func_00118C80_1C820(s32) __asm__("func_00118C80");
extern s32 func_00118CB0(s32);
extern void func_0011AD70(void *, s32);
extern s32 func_0011B4C8(void *, s32, s32, void *, s32, void *, s32, void *, void *);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/sdk/library/scewrite.c, sceWrite. */
s32 func_0011C820(s32 fd, u8 *buffer, s32 length)
{
    struct FsWriteRequest *request;
    struct SifFileSlot *file_slot;
    struct SemaphoreParameters2 semaphore_parameters;
    s32 result;
    s32 flags;
    s32 completion_semaphore;
    s32 async_index;
    volatile s32 *async_slot;
    s32 rpc_result;
    s32 prefix_length;
    s32 prefix_index;
    u32 previous_block_address;

    request = &D_00156980_1C820;
    file_slot = func_0011B7F8_1C820(fd);
    func_0011BC40_1C820(3);
    if (D_0012FD94_1BF80[0] == 0) {
        func_0011BC70_1C820();
        return -1;
    }
    if (file_slot == 0 || (flags = file_slot->flags) == 0) {
        func_0011BC70_1C820();
        return -9;
    }
    request->fd = file_slot->fd;
    semaphore_parameters.max_count = 1;
    request->slot = file_slot - D_00157E80_1BF80;
    request->length = length;
    request->buffer = buffer;
    semaphore_parameters.init_count = 0;
    semaphore_parameters.option = 0;
    completion_semaphore = func_00118C70_1C820(&semaphore_parameters);
    request->result = &result;
    request->result_size = 4;
    D_00156980_1C820.completion_semaphore = completion_semaphore;
    if ((s16)flags & 0x8000) {
        func_00118CB0(D_0012FDA4_1C388[0]);
        for (async_index = 0; async_index < 32; async_index++) {
            volatile s32 *async_semaphores = D_0012FD10;

            async_slot = async_semaphores + async_index;
            if (*async_slot == -1) {
                *async_slot = request->completion_semaphore;
                request->completion_semaphore = -request->completion_semaphore;
                break;
            }
        }
        func_00118C90(D_0012FDA4_1C388[0]);
    }
    if (((u32)buffer & 0xF) == 0) {
        prefix_length = 0;
    } else {
        previous_block_address = (u32)buffer - 0x10;
        prefix_length = (((u32)buffer >> 4) << 4) - previous_block_address;
    }
    if (length < prefix_length) {
        prefix_length = length;
    }
    if (!(flags & 0x20000000)) {
        func_0011AD70(buffer, length);
    }
    buffer = (u8 *)((u32)buffer | 0x20000000);
    request->prefix_length = prefix_length;
    for (prefix_index = 0; prefix_index < prefix_length; prefix_index++) {
        request->prefix[prefix_index] = buffer[prefix_index];
    }
    if (func_0011B4C8(&D_00158080_1C388, 3, 0, &D_00156980_1C820, 0x30, D_001575C0_1BF80, 4, 0, 0) < 0) {
        func_00118C80_1C820(completion_semaphore);
        func_0011BC70_1C820();
        return -11;
    }
    rpc_result = *(s32 *)((u32)D_001575C0_1BF80 | 0x20000000);
    func_0011BC70_1C820();
    if (rpc_result == 0) {
        func_00118C80_1C820(completion_semaphore);
        return -11;
    }
    if (flags & 0x8000) {
        func_00118C80_1C820(completion_semaphore);
        return 0;
    }
    func_00118CB0(completion_semaphore);
    func_00118C80_1C820(completion_semaphore);
    return result;
}

LINKER_REMNANT("asm/remnants/core_text", func_0011CAE0);

extern int func_0011B2F8(void *, int, int);
extern char D_00158140[];
extern int D_0012FDAC;

/* sceSifInitIopHeap: bind the RPC client D_00158140 to IOP server
   0x80000003, busy-waiting until the server answers; -1 if binding fails.
   Built with -fno-schedule-insns (config/func_cflags.txt), the flag
/* func_0011CB40: matched only with -fno-schedule-insns, which the rest of its object does not build with (docs/BUILD_FIDELITY.md, "Removed"). */
int func_0011CB40(void) {
    int k;
    int r;

    for (;;) {
        r = func_0011B2F8(D_00158140, 0x80000003, 0);
        if (r < 0) {
            return -1;
        }
        if (*(int *)(D_00158140 + 0x24) != 0) {
            D_0012FDAC = 0;
            break;
        }
        for (k = 0x100000; k != -1; k--) {
        }
    }
    return 0;
}

extern int D_00158180;
extern int D_001581C0;

/* Another func_0011B4C8 RPC (see func_001245F8), guarded on the handle
   D_0012FDAC being valid. Both failure exits share the single `return 0`
   that the early guard branches to. */
int func_0011CBC8(int arg0) {
    if (D_0012FDAC < 0) {
        return 0;
    }
    D_001581C0 = arg0;
    if (func_0011B4C8(D_00158140, 1, 0, &D_001581C0, 4,
                      &D_00158180, 4, 0, 0) >= 0) {
        return D_00158180;
    } else {
        return 0;
    }
}

/* Sibling of func_0011CBC8 with command id 2 and -1 as the RPC failure
   result, so the two failure exits cannot share a return.

   Defined old-style ON PURPOSE. func_0011CCB0 just below calls this with
   NO argument at all -- retail's call has a bare nop in its delay slot --
   which a prototype would reject. A K&R definition creates no prototype,
   so both functions compile as retail's source evidently did. */
int func_0011CC38(arg0)
int arg0;
{
    if (D_0012FDAC < 0) {
        return 0;
    }
    D_001581C0 = arg0;
    if (func_0011B4C8(D_00158140, 2, 0, &D_001581C0, 4,
                      &D_00158180, 4, 0, 0) >= 0) {
        return D_00158180;
    } else {
        return -1;
    }
}

/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_0011CCB0(void) {
    return func_0011CC38();
}

LINKER_REMNANT("asm/remnants/core_text", func_0011CCD0);
