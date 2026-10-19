/* newlib _tolower, retail 0x01ECE108 (32 bytes):
 * the ctype table at 0x01F27151 is indexed by c; bit 0 marks an upper-case
 * letter, which is mapped down by 0x20.
 */
extern unsigned char rac5_ctype_tbl[];   /* 0x01F27151 */

int _tolower(int c) {
    return (rac5_ctype_tbl[c] & 1) ? c + 0x20 : c;
}
