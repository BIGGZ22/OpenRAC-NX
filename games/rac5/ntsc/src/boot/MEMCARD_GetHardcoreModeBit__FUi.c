extern unsigned char memcard_region[];

int MEMCARD_GetHardcoreModeBit__FUi(unsigned int slot) {
    return memcard_region[slot * 20 + 0x7B] & 1;
}
