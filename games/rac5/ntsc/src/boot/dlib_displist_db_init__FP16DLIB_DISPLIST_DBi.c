/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int g_dlibBuffer;
extern int g_dlibBufferSize;
extern char dlib_helpers_path[];
extern int MEM_memalign__FUiUi12MEMAssetTypePcUi(unsigned int align, unsigned int size,
                                                 unsigned int flag, char *file,
                                                 unsigned int line);
extern void FlushCache(int mode);
extern void dlib_init__Fv(void);
extern void dlib_displist_init__FP13DLIB_DISPLISTP6QWdataUi(void *list, void *data,
                                                            unsigned int count);

void dlib_displist_db_init__FP16DLIB_DISPLIST_DBi(char *db, int count) {
    dlib_init__Fv();
    if (count) {
        g_dlibBuffer = MEM_memalign__FUiUi12MEMAssetTypePcUi(0x40, count << 5, 0,
                                                             dlib_helpers_path, 0xB3);
        g_dlibBufferSize = count;
        FlushCache(0);
        dlib_displist_init__FP13DLIB_DISPLISTP6QWdataUi(db, (void *)g_dlibBuffer, count);
        dlib_displist_init__FP13DLIB_DISPLISTP6QWdataUi(db + 0x28,
                                                        (void *)(g_dlibBuffer + (count << 4)),
                                                        count);
    }
    *(int *)(db + 0x50) = 0;
    *(int *)(db + 0x54) = (int)db;
}
