/* RAD FastIDCT8x8, retail 0x01EB5A28 (40 bytes):
 *   a3 becomes the table address + (a3 << 8), then tail-calls func_1EB4E08.
 */
extern int func_1EB4E08(int a, int b, int c, void *table);
extern char rac5_idct_tables[];         /* 0x01F23F60 */

int FastIDCT8x8(int a, int b, int c, int d) {
    return func_1EB4E08(a, b, c, rac5_idct_tables + (d << 8));
}
