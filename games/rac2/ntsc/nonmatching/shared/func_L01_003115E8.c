extern unsigned short *D_L01_001FB020[];

extern char * volatile D_L01_001B255C;

void func_L01_003115E8(int index, int value) {
    unsigned short *p;
    unsigned short entry;
    char *item;
    if (D_L01_001FB020[index] == 0)
        return;
    p = D_L01_001FB020[index];
    do {
        entry = *p;
        item = D_L01_001B255C + ((entry & 0x7FFF) << 8);
        if (*(signed char *)(item + 0x20) >= 0)
            *(item + 0x20) = value;
    } while (*(short *)p++ >= 0);
}

