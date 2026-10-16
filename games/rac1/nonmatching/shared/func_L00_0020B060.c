/* NON_MATCHING func_L00_0020B060 -- src/overlays/shared/help_00203E98.c
 * Best so far: SIZE ours 680 / retail 684, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_0020B060: re-rolls four random motion records (shared help): for i in 0..3, if func_001F9908(&q[i]) t
 *   Best p5: 680 vs 684 bytes (q as a named int * with q[1], q[2], q[i]); p7 (same size) adds j=i+13 but registers
 */

extern char D_L00_0017A780[];
extern int func_001F9908(int *);
extern int func_001F9850(int);
extern float func_L00_00258C80(float, float);
extern int func_L00_00258BC8(int, int);
extern float func_002140F8(float, float);

/* Re-rolls four random motion parameters and copies each pair into its 0xB0-byte record. */
void func_L00_0020B060(void) {
    char *base = (char *)D_0013E633 + 0xE1D;
    int *q = (int *)(base + 0x1060);
    int i;

    if (*(int *)(base + 0x2084) != 0) return;
    if (*(unsigned char *)(*(int *)(base + 0x2080) + 0x53) != 0) return;
    for (i = 0; i < 4; i++) {
        char *f = base + i * 0x10;
        if (func_001F9908(&q[i])) {
            if (i == 0) {
                *(float *)(base + 0x1028) = func_L00_00258C80(0.17453292f, 0.5235988f);
                *(int *)(D_0013E633 + 0x1E7D) = func_L00_00258BC8(func_001F9850(0x46), func_001F9850(0x96));
            } else if (i == 1) {
                *(float *)(base + 0x1038) = func_L00_00258C80(0.17453292f, 0.5235988f);
                q[1] = func_L00_00258BC8(func_001F9850(0x28), func_001F9850(0x5A));
            } else if (i == 2) {
                *(float *)(base + 0x1048) = func_L00_00258C80(0.2617994f, 0.8726646f);
                q[2] = func_L00_00258BC8(func_001F9850(0x28), func_001F9850(0x5A));
            } else {
                float old, nw;
                *(float *)(f + 0x1024) = func_002140F8(-0.08726646f, 0.5235988f);
                old = *(float *)(f + 0x1028);
                nw = func_L00_00258C80(0.34906584f, 0.95993108f);
                *(float *)(f + 0x1028) = nw;
                if ((old <= 0 && nw <= 0) || (old >= 0 && nw >= 0))
                    *(float *)(f + 0x1028) = -*(float *)(f + 0x1028);
                q[i] = func_L00_00258BC8(func_001F9850(0x23), func_001F9850(0x46));
            }
        }
        *(float *)(D_L00_0017A780 + (i + 13) * 0xB0 + 0x64) = *(float *)(f + 0x1024);
        *(float *)(D_L00_0017A780 + (i + 13) * 0xB0 + 0x68) = *(float *)(f + 0x1028);
    }
}
