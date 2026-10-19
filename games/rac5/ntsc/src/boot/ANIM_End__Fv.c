typedef struct {
    char pad00[0x14];
    char *boneComponents;
} tGDE_SkeletonInfo;

extern tGDE_SkeletonInfo *gAnimSkeletonList; /* 0x01F7E294 */
extern char gAnimBoneComponents[];           /* 0x01F7CA90 */
extern void ANIM_CreateOutputMatrices__FP16GDE_SkeletonInfoP18GDE_BoneComponents(
    tGDE_SkeletonInfo *skeleton, char *components);

void ANIM_End__Fv(void) {
    tGDE_SkeletonInfo *skeleton = gAnimSkeletonList;

    if (skeleton != 0) {
        ANIM_CreateOutputMatrices__FP16GDE_SkeletonInfoP18GDE_BoneComponents(skeleton,
                                                                             gAnimBoneComponents);
    }
}
