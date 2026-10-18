/* ps2fioGetStat - run the device's getstat entry point. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned int pad00[21];             /* +0x00 */
    int (*getstat)(int, void *);        /* +0x54 */
};

extern fio_dev_t *ps2fio_get_handle(int);

int ps2fioGetStat(int dev, void *stat)
{
    fio_dev_t *d = ps2fio_get_handle(dev);

    if (d == 0)
        return -0x13;
    if (d->getstat == 0)
        return -0x30;
    return d->getstat(dev, stat);
}
