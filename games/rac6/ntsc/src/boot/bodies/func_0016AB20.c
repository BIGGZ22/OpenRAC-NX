/* func_0016AB20 -- SCUS_976.23 0x0016AB20, 32 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 @ 0x0016AB20 - 32 bytes
 *   lw    $v1, 0x4($a0)
 *   daddu $v0, $zero, $zero
 *   sw    $a0, 0x0($a1)
 *   sw    $v1, 0x4($a1)
 *   sw    $a1, 0x4($a0)
 *   lw    $v1, 0x4($a1)
 *   jr    $ra
 *   sw    $a1, 0x0($v1)
 *   Authenticator: splices the second node after the first one in a doubly
 *   linked list (prev at +0x00, next at +0x04).  The next pointer of the
 *   first node is read before any store, and the "next->prev = node" write
 *   reloads it from the second node (the stores may alias).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct BootListNode {
    struct BootListNode *prev;
    struct BootListNode *next;
};

int func_0016AB20(struct BootListNode *pos, struct BootListNode *node)
{
    struct BootListNode *next = pos->next;

    node->prev = pos;
    node->next = next;
    pos->next = node;
    node->next->prev = node;
    return 0;
}
