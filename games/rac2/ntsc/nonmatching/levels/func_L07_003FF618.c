/* func_L07_003FF618 -- best attempt (wave 3, worker w05, file src/levels/07_siberius/L07_003F4578.c)
 * Residual: size OK (56 bytes). Same transposition as 0040FC50:
 *   retail : addiu $sp,$sp,-0x10 ; lui $v0,0x1A ; lbu $v0,0x7BD0($v0)
 *   candidate: lui $v1,0x1A ; addiu $sp,$sp,-0x10 ; lbu $v0,0x7BD0($v1)
 * Everything else is identical (beqz, sd $ra in slot, ori 2, jal, epilogue).
 * Polarity fixed: the condition is indeed `!= 0` -> or2, otherwise a call.
 */
extern u8 D_001A7BD0;
extern void func_L01_003C91C8(u8 *a0);

void func_L07_003FF618(u8 *a0)
{
    u8 flag = D_001A7BD0;

    if (flag != 0)
        *(u16 *)(a0 + 0x34) |= 2;
    else
        func_L01_003C91C8(a0);
}
