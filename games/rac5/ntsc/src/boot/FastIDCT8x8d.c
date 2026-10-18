/* RAD FastIDCT8x8d, retail 0x01EB5A50 (40 bytes):
 *   a3 becomes the table address + (a3 << 8), then tail-calls func_1EB51B8.
 */
extern int func_1EB51B8(int a, int b, int c, void *table);
extern char rac5_idct_tables[];         /* 0x01F23F60 */

int FastIDCT8x8d(int a, int b, int c, int d) {
    return func_1EB51B8(a, b, c, rac5_idct_tables + (d << 8));
}
