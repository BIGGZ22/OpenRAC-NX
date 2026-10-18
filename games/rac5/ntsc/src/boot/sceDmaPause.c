typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
typedef void VOID;
#define NULL ((void *)0)

extern s32 DIntr();
extern s32 EnableInterrupts();
s32 sceDmaPause(volatile u32 *chcr) {
    s32 state;
    u32 ctrl;
    u32 chan;
    u32 dstat;
    u32 masked;

    state = DIntr();
    ctrl = *(volatile u32 *)0x1000F520;
    if (!(ctrl & 0x10000)) {
        *(volatile u32 *)0x1000F590 = ctrl | 0x10000;
    }
    dstat = *(volatile u32 *)0x1000E000;
    chan = *chcr;
    masked = chan & 0xFFFFFEFF;
    *chcr = masked;
    *(volatile u32 *)0x1000F590 = ctrl;
    if (state != 0) {
        EnableInterrupts(masked, dstat, ctrl, state);
    }
    return chan;
}
