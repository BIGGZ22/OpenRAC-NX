/* libcdvd sceCdGetDiskType, retail 0x01EC3678 (40 bytes):
 *   r = sceCdGetDiskType2(); return ~r ? r : 0;   (i.e. -1 -> 0)
 */
extern int sceCdGetDiskType2(void);

int sceCdGetDiskType(void) {
    int r = sceCdGetDiskType2();
    return ~r ? r : 0;
}
