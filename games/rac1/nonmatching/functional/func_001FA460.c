/* functional: high. fcheck: SAME on 3 calls (OpenRAC tools/fcheck_gate.py, 2026-10-18). Copies 48 bytes (three quadwords) from src to dst, then stores the VU0 register vf0 (0,0,0,1) as the quadword at dst+0x30, the sqc2 in the delay slot. */
void func_001FA460(void *dst, void *src)
{
    float *d = (float *)dst;
    float *s = (float *)src;
    int i;

    for (i = 0; i < 12; i++)
        d[i] = s[i];
    d[12] = 0.0f;
    d[13] = 0.0f;
    d[14] = 0.0f;
    d[15] = 1.0f;
}
