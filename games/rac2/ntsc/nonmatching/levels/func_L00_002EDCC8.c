extern float D_001A790C __attribute__((sda));

/* Scales the integer input and rounds to an integer. */
int func_L00_002EDCC8(int value) {
    return (int)((float)value * D_001A790C + 0.25f * 0.25f);
}
