/* w13 ABANDON — func_L01_003EE8E0, 112 o, 7 places.
 * Derniere ligne de check.py (3e compilation) :
 *   func_L01_003EE8E0: 0 of 7 places missing -> "instruction at +0x1c differs"
 *   (116 octets produits contre 112 attendus)
 * Toute la structure colle : les trois beql/bne avec `addiu $v0,$zero,0xFF`
 * dans les cases de delai, le call, le magasin final `sb $v0,0xA8($s0)` et le
 * retour 0xFF partagent bien $v0. Il reste UNE instruction de trop :
 *   retail : addiu $v0,$0,0xFF ; ld $ra ; sb $v0,0xA8($s0) ; ld $s0 ; jr
 *   ici    : addiu $v0,$0,0xFF ; sb $v0,0xA8($s0) ; ld $s0 ; addiu $v0,$0,0xFF(!!) ; ld $ra ; jr
 * gcc rematerialise la constante pour le retour au lieu de reutiliser $v0.
 * Essaye aussi : `return 0xFF;` litteral (donne $v1 pour le store, 4
 * materialisations), et un `r = 0xFF; return r;` unique.
 */
extern char *func_002A9640(char *, int, int);

extern void func_002A8598(int, int);

int func_L01_003EE8E0(char *a0)
{
    char *s0 = a0;
    char *p = func_002A9640(a0, 0x10000, 0);
    int r;

    if (p == 0) {
        r = 0xFF;
    } else {
        p = *(char **)(p + 0x20);
        if (p == 0) {
            r = 0xFF;
        } else if (*(short *)(p + 0xAA) != 0x825) {
            r = 0xFF;
        } else {
            if (*(unsigned char *)(s0 + 0x21) != 0xFF)
                func_002A8598(*(unsigned char *)(s0 + 0x21), 2);
            r = 0xFF;
        }
    }
    *(unsigned char *)(s0 + 0xA8) = r;
    return r;
}
