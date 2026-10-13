/* best attempt — last check.py line:
   01_oozla: 176 bytes, expected 172 / instruction at +0x4 differs
   remaining: retail keeps the func_312B68 result in $v0 (no daddu $v1,$v0,$zero),
   and its second test is beql with the 0xFF materialisation in the delay slot. */
#include "common.h"

/* Arms the 0xB24/0xA54 object for a first use once its 0xC8 state is ready. */
u8 func_L01_003D9830(u8 *a0)
{
    u8 *s0 = a0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    u8 *v;
    u8 *q;
    s16 k;
    u8 t;

    v = func_312B68(s0, 1, 0);
    if (v != 0) {
        if (*(u8 **)(v + 0x20) != 0) {
            k = *(s16 *)(v + 0xAA);
            if (k == 0xB24 || k == 0xA54) {
                q = *(u8 **)(s1 + 0xC8);
                if (q != 0) {
                    t = *(u8 *)(q + 0x20);
                    if (t != 1 && t != 4 && t != 5) {
                        func_310418(s0, 2, -1);
                        *(u32 *)(s1 + 0xC4) = D_L01_001B07FC;
                    }
                }
            }
        }
    }
    *(u8 *)(s0 + 0xA8) = 0xFF;
    return 0xFF;
}
