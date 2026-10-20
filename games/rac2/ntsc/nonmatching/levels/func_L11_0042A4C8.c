/* Retained 228-byte candidate for src/levels/11_joba/L11_00425F10.c.
 * The original -O2 -G0 -ffunction-sections unit reports 0 of 1 places match:
 * entry scheduling and register allocation differ, first at +0x0c.
 * The loop after +0x34 matches the retail instruction sequence. The source
 * marker remains; this draft is never linked and behavior is not verified.
 */
#include "common.h"

extern void *D_L11_001FC420[] __attribute__((sda));
extern u8 *D_L11_001B359C;

void func_L11_0042A4C8(u8 *arg0) {
    void *list;
    u16 *p;
    u8 *rec;
    u8 *owner;
    u8 *base;
    u8 *self;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L11_001FC420[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    base = D_L11_001B359C;
    self = *(u8 **)(arg0 + 0x68);
    do {
        rec = base + ((*p & 0x7FFF) << 8);
        if (*(s8 *)(rec + 0x20) >= 0 && *(s16 *)(rec + 0xAA) == 0xCF6) {
            owner = *(u8 **)(rec + 0x68);
            *(u16 *)(owner + 0x266) = 1;
            (*(u32 *)(owner + 0x560))++;
        }
        if (*(s8 *)(rec + 0x20) >= 0 && *(s16 *)(rec + 0xAA) == 0xCF5) {
            owner = *(u8 **)(rec + 0x68);
            *(u16 *)(owner + 0x266) = 1;
            (*(u32 *)(owner + 0x560))++;
            if (*(f32 *)(self + 0x20) <= 0.0f)
                (*(u32 *)(owner + 0x584))++;
        }
    } while (*(s16 *)p++ >= 0);
}
