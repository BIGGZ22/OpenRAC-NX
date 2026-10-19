typedef struct SceGimPicture { int a; int b; int c; int d; } SceGimPicture;

extern void *sceGimPictureGetRootChunk__FPCvi(const SceGimPicture *p, int i);
extern void *sceGimPictureFindChildChunk__FPC11SceGimChunkii(void *chunk, int id, int idx);

void *sceGimPictureGetChunk__FPCvii(const SceGimPicture *p, int a, int b) {
    return sceGimPictureFindChildChunk__FPC11SceGimChunkii(sceGimPictureGetRootChunk__FPCvi(p, a), 3, b);
}
