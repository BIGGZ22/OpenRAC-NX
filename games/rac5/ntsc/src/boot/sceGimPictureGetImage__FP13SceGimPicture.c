typedef struct SceGimPicture { int a; int b; int c; void *image; void *palette; } SceGimPicture;

void *sceGimPictureGetImage__FP13SceGimPicture(SceGimPicture *p) {
    return p ? p->image : 0;
}
