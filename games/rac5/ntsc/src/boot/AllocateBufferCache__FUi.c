extern unsigned int buffer_cache_used;
extern void *buffer_cache_pending;
extern void UpdateBufferCacheState__Fv(void);

int AllocateBufferCache__FUi(unsigned int size) {
    if (buffer_cache_used != 0) return 0;
    if (buffer_cache_pending == 0) {
        buffer_cache_used = size;
        UpdateBufferCacheState__Fv();
        return 1;
    }
    return 0;
}
