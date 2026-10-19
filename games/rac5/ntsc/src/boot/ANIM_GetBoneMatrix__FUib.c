typedef struct {
    char pad00[0x14];
    unsigned char *matrixNames;
} tGDE_SkeletonInfo;

extern tGDE_SkeletonInfo *gAnimSkeletonList; /* 0x01F7E294 */
extern char *gAnimBoneMatrices;              /* 0x01EF58E4 */

char *ANIM_GetBoneMatrix__FUib(unsigned int index, int useNames) {
    if (useNames != 0) {
        tGDE_SkeletonInfo *skeleton = gAnimSkeletonList;
        unsigned char *names = skeleton->matrixNames;

        if (names != 0) {
            index = names[index];
        }
    }
    return gAnimBoneMatrices + index * 0x40;
}
