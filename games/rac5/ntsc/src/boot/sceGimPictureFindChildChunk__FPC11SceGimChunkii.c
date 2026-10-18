typedef struct {
    unsigned short f00;
    unsigned short f02;
    unsigned int f04;
    unsigned int f08;
} SceGimChunk;

const SceGimChunk *sceGimPictureFindChildChunk__FPC11SceGimChunkii(const SceGimChunk *chunk,
                                                                   int label, int index) {
    const SceGimChunk *child;
    const char *end;

    if (chunk == 0) return 0;
    end = (const char *)chunk + chunk->f04;
    child = (const SceGimChunk *)((const char *)chunk + chunk->f08);
    if ((const char *)child < end) {
        do {
            if (label == 0 || child->f00 == label) {
                if (--index == -1) return child;
            }
            child = (const SceGimChunk *)((const char *)child + child->f04);
        } while ((const char *)child < end);
    }
    return 0;
}
