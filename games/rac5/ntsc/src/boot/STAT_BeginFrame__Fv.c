extern long long stat_frame_start; /* 0x01EF5CA8 */
extern long long stat_frame_prev;  /* 0x01EF5CB0 */
extern long long sceKernelGetSystemTimeWide__Fv(void);

void STAT_BeginFrame__Fv(void) {
    long long now = sceKernelGetSystemTimeWide__Fv();
    stat_frame_start = now;
    stat_frame_prev = now;
}
