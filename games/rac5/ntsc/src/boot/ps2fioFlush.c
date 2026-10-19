typedef struct ps2fio_device {
    int *handle;
    short field4;
    short field6;
} ps2fio_device;

extern ps2fio_device ps2fio_handles[];

int ps2fioFlush(unsigned int dev) {
    ps2fio_device *d;
    int (*method)(int);
    if (dev >= 0x10) return -9;
    d = &ps2fio_handles[dev];
    if (d->field6 == 0) return -9;
    if (d->handle == 0) return -0x13;
    method = (int (*)(int))*(int *)((char *)d->handle + 0x40);
    if (method == 0) return -0x30;
    return method(d->field4);
}
