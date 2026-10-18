extern unsigned int STAT_nFrames;
extern unsigned int STAT_LastDListUsage;
extern unsigned int STAT_MaxDListUsage;
extern long long g_GPUTimeStart;
extern int STAT_GPUTimes[32];
extern long long sceKernelGetSystemTimeWide__Fv(void);

void STAT_GPU_DONE__FUi(unsigned int usage) {
    STAT_LastDListUsage = usage;
    if (STAT_MaxDListUsage < usage) {
        STAT_MaxDListUsage = usage;
    }
    STAT_GPUTimes[STAT_nFrames & 0x1F] =
        (int)(sceKernelGetSystemTimeWide__Fv() - g_GPUTimeStart);
}
