/* ps2fioDopen - open a directory handle through its device. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned int pad00[18];             /* +0x00 */
    int (*dopen)(int);                  /* +0x48 */
};

typedef struct {
    fio_dev_t *dev;    /* +0 */
    short fd;          /* +4 */
    short used;        /* +6 */
} fio_handle_t;

extern fio_handle_t ps2fio_handles[16];
extern fio_dev_t *ps2fio_get_handle(int);
extern int ps2fioAllocHandle(void);
extern void ps2fioFreeHandle(int);

int ps2fioDopen(int dev)
{
    fio_dev_t *d = ps2fio_get_handle(dev);
    fio_handle_t *h;
    int idx;
    int r;

    if (d == 0)
        return -0x13;
    if (d->dopen == 0)
        return -0x30;
    idx = ps2fioAllocHandle();
    h = &ps2fio_handles[idx];
    h->dev = d;
    r = d->dopen(dev);
    if (r < 0) {
        ps2fioFreeHandle(idx);
        return r;
    }
    h->fd = r;
    return idx;
}
