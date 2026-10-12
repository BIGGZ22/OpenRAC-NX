/* func_L07_0040FC50 -- meilleur essai (vague 3, worker w05, fichier src/levels/07_siberius/L07_003F4578.c)
 * Residuel: taille OK (52 o). Seules les DEUX premieres instructions sont transposees:
 *   retail : addiu $v1,$zero,0xF ; lui $v0,0x1B ; addiu $v0,$v0,0x31AF
 *   candidat: lui $v0,0x1B ; addiu $v1,$zero,0xF ; addiu $v0,$v0,0x31AF
 * Le reste (dont le nop de rembourrage a +0xC et le bgez/sb) est identique.
 * Essayes: initialiseur de declaration, expression virgule dans le for, do/while explicite --
 * le planificateur place toujours le lui en tete.
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
