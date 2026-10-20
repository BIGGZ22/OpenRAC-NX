extern char *D_L08_001B29DC __attribute__((sda));

float func_L08_003CCD58(int index) {
    if (index != -1)
        return *(float *)(D_L08_001B29DC + ((unsigned int)index << 8) + 0x18);
    return 0.0f;
}

int func_L08_003CCD28(int index) {
    if (index != -1) {
        if (*(unsigned char *)(*(unsigned char **)(D_L08_001B29DC
                                                  + ((unsigned int)index << 8) + 0x68)
                              + 0x14) != 0)
            return 1;
    }
    return 0;
}

int func_L08_003D0098(int index) {
    if (index != -1) {
        if (*(unsigned char *)(*(unsigned char **)(D_L08_001B29DC
                                                  + ((unsigned int)index << 8) + 0x68)
                              + 0x14) != 0)
            return 1;
    }
    return 0;
}
