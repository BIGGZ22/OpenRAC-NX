extern void *cdvdfs_fs;
extern int cdvdfs_field_c04;
extern int cdvdfs_field_c00;
extern void free(void *p);

int cdvdfsFreeFS(void) {
    if (cdvdfs_fs == 0) return -1;
    free(cdvdfs_fs);
    cdvdfs_fs = 0;
    cdvdfs_field_c04 = 0;
    cdvdfs_field_c00 = 0;
    return 0;
}
