/* ps2fioWrite - dispatch a write through the registered device driver. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned int pad00[5];
    int (*read)(int, void *, int);   /* +0x14 */
    int (*write)(int, void *, int);  /* +0x18 */
    int (*lseek)(int, int, int);     /* +0x1C */
    unsigned int pad20[11];
    int (*dread)(int, void *, int);  /* +0x4C */
};

typedef struct {
    fio_dev_t *dev;    /* +0 */
    short fd;          /* +4 */
    short used;        /* +6 */
} fio_handle_t;

extern fio_handle_t ps2fio_handles[16];

int ps2fioWrite(int fd, void *buf, int size)
{
    if (fd < 0 || fd >= 16 || ps2fio_handles[fd].used == 0)
        return -0x9;
    if (ps2fio_handles[fd].dev->write == 0)
        return -0x30;
    return ps2fio_handles[fd].dev->write(ps2fio_handles[fd].fd, buf, size);
}
