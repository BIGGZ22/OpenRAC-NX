/* func_L07_003F9028 -- best attempt (wave 3, worker w05, file src/levels/07_siberius/L07_003F4578.c)
 * Residual: 92 bytes, 3 instructions out of 23. The byte at 0x43($a0) is loaded into $a1 instead of $v1
 * (the retail reuses the flag register, dead after the bnez). Branch and layout OK.
 * Tried: u8 c, s32 c, declaration before the pointer -- always $a1.
 */
extern void func_002A7470(u8 *a0, s32 a1, s32 a2, s32 a3);

void func_L07_003F9028(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);
    s32 c = *(u8 *)(a0 + 0x43);

    if (*(s32 *)(p + 0x420) == 0) {
        if (c == 2)
            return;
        func_002A7470(a0, 2, 0, 10);
    } else {
        if (c == 8)
            return;
        func_002A7470(a0, 8, 0, 10);
    }
}
