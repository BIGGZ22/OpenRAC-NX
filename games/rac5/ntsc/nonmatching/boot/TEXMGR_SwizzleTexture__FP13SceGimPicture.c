typedef struct SceGimPicture SceGimPicture;

typedef struct {
    unsigned short unk00;       /* +0x0 */
    unsigned short unk02;       /* +0x2 */
    unsigned short format;      /* +0x4 */
    unsigned short swizzled;    /* +0x6 */
    unsigned short width;       /* +0x8 */
    unsigned short height;      /* +0xA */
} GIM_IMAGE;

extern GIM_IMAGE *sceGimPictureGetImage__FP13SceGimPicture(SceGimPicture *pic);
extern void *sceGimPictureGetImagePixels__FP13SceGimPictureii(SceGimPicture *pic, int a, int b);
extern void writeTexPSMT8__FiiiiiiPv(int a0, int a1, int a2, int a3, int w, int h, void *p);
extern void readTexPSMCT32__FiiiiiiPv(int a0, int a1, int a2, int a3, int w, int h, void *p);
extern void writeTexPSMT4__FiiiiiiPv(int a0, int a1, int a2, int a3, int w, int h, void *p);
extern void readTexPSMCT16__FiiiiiiPv(int a0, int a1, int a2, int a3, int w, int h, void *p);

void TEXMGR_SwizzleTexture__FP13SceGimPicture(SceGimPicture *pic) {
    GIM_IMAGE *image = sceGimPictureGetImage__FP13SceGimPicture(pic);
    void *pixels;

    if (image->swizzled != 0)
        return;
    pixels = sceGimPictureGetImagePixels__FP13SceGimPictureii(pic, 0, 0);
    if (image->format == 5) {
        if (image->width < 0x20)
            return;
        if (image->height < 0x20)
            return;
        writeTexPSMT8__FiiiiiiPv(0, (image->width + 0x3F) >> 6, 0, 0,
                                 image->width, image->height, pixels);
        readTexPSMCT32__FiiiiiiPv(0, ((image->width >> 1) + 0x3F) >> 6, 0, 0,
                                  image->width >> 1, image->height >> 1, pixels);
        image->swizzled = 1;
    } else if (image->format == 4) {
        if (image->width < 0x20)
            return;
        if (image->height < 0x20)
            return;
        writeTexPSMT4__FiiiiiiPv(0, (image->width + 0x3F) >> 6, 0, 0,
                                 image->width, image->height, pixels);
        readTexPSMCT16__FiiiiiiPv(0, ((image->height >> 1) + 0x3F) >> 6, 0, 0,
                                  image->height >> 1, image->width >> 1, pixels);
        image->swizzled = 1;
    }
}
