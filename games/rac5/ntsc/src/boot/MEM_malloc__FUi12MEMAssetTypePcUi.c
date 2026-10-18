/* MEM_malloc is MEM_memalign with a fixed 16-byte alignment; the line number is
   forwarded unchanged (MEM_memalign takes five arguments). */
extern void *MEM_memalign__FUiUi12MEMAssetTypePcUi(unsigned int alignment,
                                                   unsigned int size, int type,
                                                   char *name, unsigned int line);

void *MEM_malloc__FUi12MEMAssetTypePcUi(unsigned int size, int type, char *name,
                                        unsigned int line) {
    return MEM_memalign__FUiUi12MEMAssetTypePcUi(0x10, size, type, name, line);
}
