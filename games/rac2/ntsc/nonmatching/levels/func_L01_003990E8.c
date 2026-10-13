/* w13 ABANDON — func_L01_003990E8, 132 o, 5 places.
 * Derniere ligne de check.py (5e compilation) :
 *   func_L01_003990E8: 0 of 5 places match
 *   (retail) "instruction at +0xc differs"
 * UNE seule instruction differe : l'ordre de materialisation des deux
 * constantes du preheader. Retail : [addiu $t1,$0,1 ; lhu $v0,0x34 ; addiu
 * $t0,$0,2]. Ce candidat : [addiu $t0,$0,2 ; lhu ; addiu $t1,$0,1]. Les
 * registres, l'ordre des 3 stores (0x98, 0x34, 0x31) et toute la boucle sont
 * identiques au retail.
 * Essais : ordre des stores inverse, `p = ...` avant/apres le store 0x31,
 * `i = 2;` explicite avant le RMW -- aucun ne deplace la constante hissee.
 */
void func_L01_003990E8(char *a3)
{
    int i;
    char **p;
    char *q;

    q = *(char **)(a3 + 0x24);
    *(unsigned int *)(a3 + 0x98) = *(unsigned int *)(q + 0x10);
    *(unsigned short *)(a3 + 0x34) &= 0xFFFE;
    p = (char **)(*(char **)(a3 + 0x68) + 0x14);
    *(unsigned char *)(a3 + 0x31) = 1;
    for (i = 2; i >= 0; i--) {
        if (*p != 0) {
            *(unsigned short *)(*p + 0x34) &= 0xFFFE;
            *(unsigned char *)(*p + 0x31) = 1;
            *(unsigned short *)(*p + 0x34) &= 0xFF7F;
            *(long long *)(a3 + 0x38) = *(long long *)(*p + 0x38);
        }
        p++;
    }
}
