/* libcdvd cdvdfsGetEntryFromIndex, retail 0x01EC2488 (56 bytes):
 * signed short index, bounds-checked against the entry count, 32-byte stride.
 */
extern int rac5_cdvdfs_count;           /* 0x01FDBC00 */
extern void *rac5_cdvdfs_base;          /* 0x01EFC6E8 */

void *cdvdfsGetEntryFromIndex(short index) {
    if (index < 0 || index >= rac5_cdvdfs_count)
        return 0;
    return (char *)rac5_cdvdfs_base + (index << 5);
}
