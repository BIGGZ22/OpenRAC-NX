/* Authored from the pinned RAC5 boot instructions and measured
 * device-method offsets; no Sony SDK source or header used. */
/* ps2fioChStat - run the device's chstat entry point. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned int pad00[22];                  /* +0x00 */
    int (*chstat)(int, void *, void *);      /* +0x58 */
};

extern fio_dev_t *func_1EBB8C0(int);

int ps2fioChStat(int dev, void *stat, void *mask)
{
    fio_dev_t *d = func_1EBB8C0(dev);

    if (d == 0)
        return -0x13;
    if (d->chstat == 0)
        return -0x30;
    return d->chstat(dev, stat, mask);
}
