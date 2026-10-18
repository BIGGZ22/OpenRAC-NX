extern void *MEM_memalign__FUiUi12MEMAssetTypePcUi(unsigned int alignment,
                                                   unsigned int size, int type,
                                                   char *name, unsigned int flags);

void *MEM_malloc_uncached__FUi12MEMAssetTypePcUi(unsigned int size, int type,
                                                 char *name, unsigned int flags) {
    return MEM_memalign__FUiUi12MEMAssetTypePcUi(0x40, (size + 0x40) & 0xFFFFFFC0,
                                                 type, name, flags);
}
