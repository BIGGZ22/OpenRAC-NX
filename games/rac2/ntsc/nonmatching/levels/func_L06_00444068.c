/* func_L06_00444068 (256 bytes) -- ABANDON (best 268 bytes, 2nd shape 344 bytes).
 * Structure recovered from the diff: three calls, float clamps into
 * 0x2C0 / 0x2C8, constants 0x3F49FDC0 (+0.7890167236328125) and 0x3F060A93
 * (pi/6), and the second func_32DC80's first argument is func_32D740's return
 * (its `mov.s $f12,$f0` sits in the jal delay slot, before the call).
 * Two walls:
 *  - the `float v` result is given a callee-saved FPR ($f20) and grows the frame
 *    to 0x40 with an `swc1 $f20` save, where retail keeps it in $f0 throughout;
 *  - a `0.7890167f` literal is mis-rounded by cc1 to 0x3F49FD00 (use the full
 *    decimal 0.7890167236328125 to get the retail word).
 */
extern char D_L06_00189EA0[];
extern void func_32D5F8(char *, char *, char *);
extern float func_32DC80(float, float);
extern float func_32D740(char *);
extern void func_365030(char *, char *, char *, int);

void func_L06_00444068(char *a0)
{
    float buf[4];
    char *s1 = *(char **)(a0 + 0x68);
    float v;
    float w;

    func_32D5F8((char *)buf, D_L06_00189EA0, a0 + 0x10);
    func_365030(a0, (char *)buf, (char *)buf, 0);
    v = func_32DC80(buf[0], buf[1]);
    *(float *)(s1 + 0x2C0) = v;
    if (v > 0.7890167236328125)
        *(float *)(s1 + 0x2C0) = 0.7890167236328125;
    if (v < -0.7890167236328125)
        *(float *)(s1 + 0x2C0) = -0.7890167236328125;
    w = -func_32DC80(func_32D740((char *)buf), buf[2]);
    *(float *)(s1 + 0x2C8) = w;
    if (w > 0.5235987901687622)
        *(float *)(s1 + 0x2C8) = 0.5235987901687622;
    if (w < -0.5235987901687622)
        *(float *)(s1 + 0x2C8) = -0.5235987901687622;
}
