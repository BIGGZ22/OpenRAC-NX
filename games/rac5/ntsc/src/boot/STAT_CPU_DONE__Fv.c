extern unsigned int STAT_nFrames;
extern long long g_CPUTimeStart;
extern int STAT_CPUTimes[32];
extern long long sceKernelGetSystemTimeWide__Fv(void);

void STAT_CPU_DONE__Fv(void) {
    STAT_CPUTimes[STAT_nFrames & 0x1F] =
        (int)(sceKernelGetSystemTimeWide__Fv() - g_CPUTimeStart);
}
