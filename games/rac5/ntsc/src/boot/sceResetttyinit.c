/* sceResetttyinit 0x01ED10E0..0x01ED10F0 (16 bytes).
 *   lui $v0, 0x1F0
 *   jr  $ra
 *   sw  $zero, -0x10C0($v0)     (delay slot)
 *   nop
 * Clears the kernel tty-init word at 0x01F0EF40.  The store must not be
 * volatile: a volatile store is not moved into the return delay slot. */
extern unsigned int tty_init_word;

void sceResetttyinit(void) {
    tty_init_word = 0;
}
