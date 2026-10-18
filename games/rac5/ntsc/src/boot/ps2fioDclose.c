/* Authored from the pinned RAC5 boot instructions and measured
 * device-method offsets; no Sony SDK source or header used. */
/* ps2fioDclose - close a directory handle, then release the slot. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned int pad00[20];             /* +0x00 */
    int (*dclose)(int);                 /* +0x50 */
};

typedef struct {
    fio_dev_t *dev;    /* +0 */
    short fd;          /* +4 */
    short used;        /* +6 */
} fio_handle_t;

extern fio_handle_t ps2fio_handles[16];
extern void ps2fioFreeHandle(int);

int ps2fioDclose(int fd)
{
    fio_dev_t *d;
    int r;

    if (fd < 0 || fd >= 16 || ps2fio_handles[fd].used == 0)
        return -0x9;
    d = ps2fio_handles[fd].dev;
    if (d->dclose == 0)
        return -0x30;
    r = d->dclose(ps2fio_handles[fd].fd);
    if (r < 0)
        return r;
    ps2fioFreeHandle(fd);
    return 0;
}
