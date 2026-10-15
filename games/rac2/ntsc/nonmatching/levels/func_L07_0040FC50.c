/* func_L07_0040FC50 -- best attempt (wave 3, worker w05, file src/levels/07_siberius/L07_003F4578.c)
 * Residual: size OK (52 bytes). Only the FIRST TWO instructions are transposed:
 *   retail : addiu $v1,$zero,0xF ; lui $v0,0x1B ; addiu $v0,$v0,0x31AF
 *   candidate: lui $v0,0x1B ; addiu $v1,$zero,0xF ; addiu $v0,$v0,0x31AF
 * The rest (including the padding nop at +0xC and the bgez/sb) is identical.
 * Tried: declaration initialiser, comma expression in the for, explicit do/while --
 * the scheduler always puts the lui at the front.
 */
extern u8 D_L07_001B31AF;

void func_L07_0040FC50(void)
{
    u8 *p;
    s32 i;

    i = 15;
    p = &D_L07_001B31AF;
    do {
        *p-- = 0;
        i--;
    } while (i >= 0);
}
