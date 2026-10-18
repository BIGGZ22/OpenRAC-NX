/* newlib abort, retail 0x01EC82D8 (16 bytes): _exit(1) with no epilogue.
 *   addiu $sp,$sp,-0x10 ; sd $ra,0($sp) ; jal _exit ; addiu $a0,$zero,1
 */
extern void _exit(int code) __attribute__((noreturn));

void abort(void) {
    _exit(1);
}
