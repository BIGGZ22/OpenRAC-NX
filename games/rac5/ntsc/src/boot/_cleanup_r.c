/* newlib _cleanup_r, retail 0x01EC6F80 (32 bytes):
 *   addiu $sp,$sp,-0x10 ; lui $a1,0x1ED ; sd $ra,0($sp) ; jal _fwalk ;
 *   addiu $a1,$a1,-0xD78  (0x01ECF288 = fflush)
 */
struct _reent;
struct _FILE;

extern int fflush(struct _FILE *fp);
extern int _fwalk(struct _reent *ptr, int (*fn)(struct _FILE *));

void _cleanup_r(struct _reent *ptr) {
    _fwalk(ptr, fflush);
}
