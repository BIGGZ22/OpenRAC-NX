extern unsigned int g_JointBufferSpaceUsedMax;
extern unsigned int g_JointBufferSpaceUsed;
extern int g_JointMatrixActiveBuffer;

void ANIM_SwapJointMatrixBuffers__Fi(int spaceUsed) {
    if (g_JointBufferSpaceUsed > g_JointBufferSpaceUsedMax) {
        g_JointBufferSpaceUsedMax = g_JointBufferSpaceUsed;
    }
    if (spaceUsed < 0) {
        g_JointBufferSpaceUsed = 0;
    } else {
        g_JointBufferSpaceUsed = spaceUsed;
    }
    g_JointMatrixActiveBuffer = !g_JointMatrixActiveBuffer;
}
