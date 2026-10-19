extern unsigned int gAnimMemSize;    /* 0x01EF58F8 */
extern void *gAnimBuffers[2];        /* 0x01F7E2A0 */
extern const char gAnimMemName[];    /* 0x01F1F370 */
extern void *MEM_memalign__FUiUi12MEMAssetTypePcUi(unsigned int size, unsigned int align,
                                                   int type, const char *name,
                                                   unsigned int line);

void ANIM_PostInit__Fv(void) {
    unsigned int i;

    for (i = 0; i < 2; i++) {
        gAnimBuffers[i] = MEM_memalign__FUiUi12MEMAssetTypePcUi(0x40, gAnimMemSize, 0,
                                                                gAnimMemName, 0x6A7);
    }
}
