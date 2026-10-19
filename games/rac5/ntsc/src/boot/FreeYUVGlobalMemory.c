/* FreeYUVGlobalMemory - release both YUV scratch buffers. */

extern void radfree(void *);
extern void *yuv_global_a;      /* 0x01EFB040 */
extern int yuv_global_a_key;    /* 0x01EFC0D4 */
extern void *yuv_global_b;      /* 0x01EFB054 */
extern int yuv_global_b_key;    /* 0x01EFC0D8 */

void FreeYUVGlobalMemory(void)
{
    if (yuv_global_a != 0) {
        radfree(yuv_global_a);
        yuv_global_a_key = -1;
        yuv_global_a = 0;
    }
    if (yuv_global_b != 0) {
        radfree(yuv_global_b);
        yuv_global_b_key = -1;
        yuv_global_b = 0;
    }
}
