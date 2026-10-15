/* Scale helper: K * x with K the float at D_0015EE60 (gp-relative). Retail also moves $sp
 * by +0x1F0 (five addiu on $29 in the body): not reproducible in C and not called. */
extern float D_0015EE60;

float func_001F9810(float x) {
    return *(float *)&D_0015EE60 * x;
}
