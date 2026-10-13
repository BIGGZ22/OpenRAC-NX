extern char *D_L08_001B29DC __attribute__((sda));

int func_L08_003CCD28(int index) {
    char *base = D_L08_001B29DC;
    char *entry;
    unsigned char flag;
    if (index == -1) return -1;
    entry = *(char **)(base + (index << 8) + 0x68);
    flag = *(unsigned char *)(entry + 0x14);
    if (flag) return 1;
    return 0;
}
