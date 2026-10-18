extern unsigned char memcard_flags;

int MEMCARD_GetDanSkinUnlocked__Fv(void) {
    return (memcard_flags >> 1) & 1;
}
