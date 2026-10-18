typedef struct SceGimPicture { int a; int b; int c; void *image; void *palette; } SceGimPicture;

void *sceGimPictureGetPalette__FP13SceGimPicture(SceGimPicture *p) {
    return p ? p->palette : 0;
}
