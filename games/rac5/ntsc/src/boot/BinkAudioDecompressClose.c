/* Bink audio close, retail 0x01EB3850 (28 bytes + nop padding):
 *   addiu $sp,$sp,-0x10 ; sd $ra,0($sp) ; jal radfree ; nop ; epilogue
 */
extern void radfree(void *ptr);

void BinkAudioDecompressClose(void *ptr) {
    radfree(ptr);
}
