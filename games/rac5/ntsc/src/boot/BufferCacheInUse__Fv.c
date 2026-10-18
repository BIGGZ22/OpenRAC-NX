extern int buffer_cache_used;    /* 0x01EF57AC */
extern int buffer_cache_pending; /* 0x01EF57B0 */

unsigned int BufferCacheInUse__Fv(void) {
    int used = buffer_cache_used;
    if (used || buffer_cache_pending) return used;
    return 0xFFFFFFFF;
}
