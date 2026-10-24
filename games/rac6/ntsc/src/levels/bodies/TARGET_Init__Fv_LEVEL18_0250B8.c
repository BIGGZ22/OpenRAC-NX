/* TARGET_Init__Fv_LEVEL18_0250B8 -- LEVEL18.REL 0x0250B8, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   lui $v0,0x2C ; lui $v1,0x1F ; addiu $v0,$v0,-0x6DC0 ; lui $a0,0x2C
 *   sw $v0,0x5F94($v1) ; addiu $a1,$v0,0x5C ; sw $zero,-0x6DCC($a0)
 *   daddu $v1,$zero,$zero ; sw $zero,0x0($a1) ; addiu $v1,$v1,0x1
 *   sltiu $v0,$v1,0x10 ; nop ; bnez $v0, boucle ; addiu $a1,$a1,0x64
 *   jr $ra ; nop
 *   L'initialisation memorise le pointeur de l'objet global (0x2B9240) dans
 *   le pointeur global (0x1F5F94), efface un mot (0x2B9234), puis met a zero
 *   16 mots du tableau de l'objet, de 100 octets en 100 octets, a partir de
 *   l'offset 0x5C.  Le compteur de boucle est non signe (sltiu).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int D_002B9240[];      /* objet global a 0x2B9240 */
extern int *D_001F5F94;       /* pointeur global a 0x1F5F94 */
extern int D_002B9234;        /* mot global a 0x2B9234 */

void TARGET_Init__Fv(void)
{
    unsigned int i;
    int *obj = D_002B9240;
    int *p;

    D_001F5F94 = obj;
    D_002B9234 = 0;
    p = obj + 23;
    for (i = 0; i < 16; i++) {
        *p = 0;
        p += 25;
    }
}
