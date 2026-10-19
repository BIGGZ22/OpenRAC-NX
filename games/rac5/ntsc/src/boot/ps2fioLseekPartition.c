typedef struct ps2fio_device {
    int *handle;
    short field4;
    short field6;
} ps2fio_device;

extern ps2fio_device ps2fio_dev_table[];

int ps2fioLseekPartition(unsigned int dev, int offset, int whence) {
    ps2fio_device *d;
    int (*method)(int, int, int);
    if (dev >= 0x10) return -9;
    d = &ps2fio_dev_table[dev];
    if (d->field6 == 0) return -9;
    method = (int (*)(int, int, int))*(int *)((char *)d->handle + 0x80);
    return method(d->field4, offset, whence);
}
