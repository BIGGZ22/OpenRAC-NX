/* newlib _localeconv_r, retail 0x01ECE128 (12 bytes + padding):
 *   lui $v0,%hi(s) ; jr $ra ; addiu $v0,$v0,%lo(s) ; nop
 */
extern char rac5_lconv_storage[];   /* 0x01F27258 */

void *_localeconv_r(void *ptr) {
    return rac5_lconv_storage;
}
