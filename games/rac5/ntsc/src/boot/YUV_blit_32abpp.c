/* RAD YUV blit wrapper, retail 0x01EADE58 (56 bytes).
 * Eight register arguments plus two stack arguments are forwarded unchanged;
 * a third stack slot carries the pixel-format descriptor.
 */
extern int func_1EAB7E0(int, int, int, int, int, int, int, int, int, int, char *);
extern char rac5_yuv_desc_32abpp[];      /* 0x01EFC0E0 */

int YUV_blit_32abpp(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return func_1EAB7E0(a, b, c, d, e, f, g, h, i, j, rac5_yuv_desc_32abpp);
}
