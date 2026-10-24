/* LEVEL18_01E618 -- LEVEL18.REL 0x01E618, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   lui $a1,0x20 ; lui $a3,0x35 ; lw $a0,-0x40D0($a1) ; addiu $sp,$sp,-0x10
 *   lw $v1,-0x2E80($a3) ; ori $a2,$zero,0x8000 ; sd $ra,0x0($sp)
 *   sw $v1,0xECC($a0) ; lw $v0,-0x40D0($a1) ; addu $v0,$v0,$a2
 *   sb $zero,0x7755($v0) ; jal func_0376B0 ; lw $a0,-0x2E80($a3)
 *   ld $ra,0x0($sp) ; jr $ra ; addiu $sp,$sp,0x10
 *   L'ecriture copie un mot du global 0x34D180 vers le bloc global (pointeur
 *   0x1FBF30, offset 0xECC), efface un octet a l'offset 0x7755 du sous-bloc
 *   0x8000, puis appelle func_0376B0 sur la meme valeur globale.
 *   Les deux acces au pointeur global sont separes par l'ecriture : le
 *   pointeur est donc recharge apres le store (le pointeur peut avoir ete
 *   modifie par le store).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct TargetPart {
    unsigned char pad000[0x7755];
    unsigned char flag;
} TargetPart;

typedef struct GlobalVars {
    unsigned char pad000[0xECC];
    unsigned int value;
    unsigned char pad004[0x8000 - 0xECC - 4];
    TargetPart part;
} GlobalVars;

extern GlobalVars *D_001FBF30;
extern unsigned int D_0034D180;

extern void func_0376B0(unsigned int target);

void func_01E618(void)
{
    *(unsigned int *)((unsigned char *)D_001FBF30 + 0xECC) = D_0034D180;
    ((TargetPart *)((unsigned char *)D_001FBF30 + 0x8000))->flag = 0;
    func_0376B0(D_0034D180);
}
