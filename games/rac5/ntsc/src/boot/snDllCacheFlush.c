/* snDllCacheFlush 0x01E9BF00..0x01E9BF30 */
extern void FlushCache(int mode);
void snDllCacheFlush(void) {
    FlushCache(0);
    FlushCache(2);
}
