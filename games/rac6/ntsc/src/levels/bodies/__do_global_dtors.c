/* __do_global_dtors -- FE.REL 0x00069048, 84 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail body (84 bytes) : a cursor word at 0x108AE8 is walked, each entry
 *   is a destructor pointer, the list ends on a null entry.
 *   addiu $sp, $sp, -0x20
 *   lui   $a0, 0x11
 *   sd    $ra, 0x10($sp)
 *   sd    $s0, 0x0($sp)
 *   lw    $v0, -0x7518($a0)      ; le curseur
 *   lw    $v1, 0x0($v0)
 *   beqz  $v1, ...
 *   daddu $s0, $a0, $zero
 *   lw    $v0, -0x7518($s0)
 *   addiu $v0, $v0, 0x4
 *   lw    $v1, -0x4($v0)
 *   jalr  $v1
 *   sw    $v0, -0x7518($s0)
 *   lw    $v0, -0x7518($s0)
 *   lw    $v1, 0x0($v0)
 *   bnez  $v1, ...
 *   addiu $v0, $v0, 0x4
 *   ld    $ra, 0x10($sp)
 *   ld    $s0, 0x0($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20
 *   Le curseur est un mot local du module a 0x108AE8 (relocation locale du
 *   corps retail) ; il est de type char * : c'est ce type qui fait que le
 *   retail charge l'entree par la nouvelle adresse du curseur (-4($v0)).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef void (*dtor_entry)(void);

/* mot local du module (relocation locale du corps retail) */
extern char *D_00108AE8;

void __do_global_dtors(void)
{
    while (*(dtor_entry *)D_00108AE8) {
        D_00108AE8 = D_00108AE8 + 4;
        ((dtor_entry)*(dtor_entry *)(D_00108AE8 - 4))();
    }
}
