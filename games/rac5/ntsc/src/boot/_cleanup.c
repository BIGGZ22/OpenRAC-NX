/* newlib _cleanup, retail 0x01EC6FA0 (40 bytes + padding):
 *   lui $v0,0x1F0 ; addiu $sp,$sp,-0x10 ; lw $a0,-0x18D4($v0) ; sd $ra,0($sp) ;
 *   jal _cleanup_r ; epilogue
 */
struct _reent;

extern struct _reent *_impure_ptr;
extern void _cleanup_r(struct _reent *ptr);

void _cleanup(void) {
    _cleanup_r(_impure_ptr);
}
