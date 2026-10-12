/* func_L07_003FF618 -- meilleur essai (vague 3, worker w05, fichier src/levels/07_siberius/L07_003F4578.c)
 * Residuel: taille OK (56 o). Meme transposition que 0040FC50:
 *   retail : addiu $sp,$sp,-0x10 ; lui $v0,0x1A ; lbu $v0,0x7BD0($v0)
 *   candidat: lui $v1,0x1A ; addiu $sp,$sp,-0x10 ; lbu $v0,0x7BD0($v1)
 * Tout le reste est identique (beqz, sd $ra en slot, ori 2, jal, epilogue).
 * Polarite corrigee: la condition est bien `!= 0` -> or2, sinon appel.
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
