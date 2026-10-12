extern char *D_L08_001B29DC __attribute__((sda));

int func_L08_003D0098(int index) {
    char *base = D_L08_001B29DC;
    void *object;
    if (index == -1) return 0;
    object = *(void **)(base + index * 0x100 + 0x68);
    if (*(unsigned char *)((char *)object + 0x14)) return 1;
    return 0;
}
