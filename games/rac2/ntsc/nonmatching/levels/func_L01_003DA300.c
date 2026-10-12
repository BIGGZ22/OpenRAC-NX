/* best attempt — last check.py line:
   01_oazla: 236 bytes OK / instruction at +0x4 differs
   remaining: only the $s3/$s4 assignment of the a0/a1 copies is mirrored
   (pure register allocation: retail a0->$s3, a1->$s4). */
#include "common.h"

/* Creates the 0xB24 object, seeds its state block and copies the two vectors in. */
u8 *func_L01_003DA300(u8 *a0, u8 *a1, u8 *a2, f32 f12)
{
    u8 *s2 = a2;
    u8 *s3 = a0;
    u8 *s4 = a1;
    f32 f20 = f12;
    u8 *s1;
    u8 *s0;

    s1 = func_305EE8(0xB24);
    if (s1 != 0) {
        *(u8 *)(s1 + 0x30) = 0xFF;
        *(u16 *)(s1 + 0x32) = 0xFF;
        *(u8 *)(s1 + 0x31) = 1;
        *(u8 *)(s1 + 0x20) = 0;
        s0 = *(u8 **)(s1 + 0x68);
        *(f32 *)(s1 + 0xF8) = func_310648();
        *(f32 *)(s1 + 0xF4) = func_310648();
        *(f32 *)(s1 + 0xF0) = func_310648();
        *(f32 *)(s1 + 0x2C) = *(f32 *)(*(u8 **)(s1 + 0x24) + 0x24) * D_L01_001B0828;
        *(u32 *)(s0 + 0x10) = s2;
        *(f32 *)(s0 + 0x18) = f20;
        qcopy(s1 + 0x10, s3);
        qcopy(s0, s4);
        if (*(u8 *)(s1 + 0x43) != 0)
            func_310138(s1, 0, 0, 0xA);
    }
    return s1;
}
