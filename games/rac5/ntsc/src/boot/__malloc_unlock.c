/* newlib __malloc_unlock, retail 0x01EC8838 (64 bytes):
 *   if (--lock_count == 0) { SignalSema(sema); owner = -1; }
 */
extern int __malloc_lock_count;   /* 0x01EFEA94 */
extern int __malloc_owner;        /* 0x01EFEA90 */
extern int __malloc_sema;         /* 0x01EFF018 */

extern int SignalSema(int sema);

int __malloc_unlock(void) {
    if (--__malloc_lock_count == 0) {
        __malloc_owner = -1;
        SignalSema(__malloc_sema);
    }
}
