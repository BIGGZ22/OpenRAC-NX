/* functional: medium. Plain C for the VU0 sequence: q = a / b (vdiv), truncate q toward zero (vftoi0 then vitof0), take the fraction (vsub), multiply by b (vmul). VU rounding and the inf/NaN cases of vdiv are not reproduced. */
float func_L00_00200210(float a, float b) {
    float q = a / b;
    int t = (int)q;
    return (q - (float)t) * b;
}
