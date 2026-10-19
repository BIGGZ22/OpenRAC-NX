typedef struct SceGimPicture { int a; int b; int c; void *image; void *palette; } SceGimPicture;

void *sceGimPictureGetRootChunk__FPCvi(const SceGimPicture *p, int i) {
    if (p == 0) {
        return 0;
    }
    return (void *)((char *)p + 0x10);
}
