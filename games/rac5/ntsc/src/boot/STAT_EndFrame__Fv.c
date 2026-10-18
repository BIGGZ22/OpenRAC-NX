extern unsigned int STAT_nFrames;
extern int STAT_nLastFrame;
extern int g_LastDListSize;
extern long long g_PrevFrameTime;
extern int STAT_FrameTimes[32];
extern void STAT_GPU_DONE__FUi(unsigned int usage);
extern void Signal__Q34scee3Psp2GuUs(unsigned short signal);
extern long long sceKernelGetSystemTimeWide__Fv(void);

void STAT_EndFrame__Fv(void) {
    unsigned int frame = STAT_nFrames;
    long long now;
    STAT_nLastFrame = frame;
    STAT_nFrames = (frame + 1) & 0x1F;
    STAT_GPU_DONE__FUi(g_LastDListSize);
    Signal__Q34scee3Psp2GuUs(0xDEAD);
    now = sceKernelGetSystemTimeWide__Fv();
    {
        long long delta = now - g_PrevFrameTime;
        g_PrevFrameTime = now;
        STAT_FrameTimes[STAT_nFrames] = (int)delta;
    }
}
