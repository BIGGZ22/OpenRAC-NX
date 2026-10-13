extern char *D_L08_001B29DC __attribute__((sda));

float func_L08_003CCD58(int index) {
    if (index != -1)
        return *(float *)(D_L08_001B29DC + ((unsigned int)index << 8) + 0x18);
    return 0.0f;
}
