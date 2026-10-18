/* ps2fioRegisterDevice - prepend a driver to the registered-device list,
   refusing a duplicate. */

typedef struct fio_dev_s fio_dev_t;
struct fio_dev_s {
    unsigned char pad[0xCC];
    fio_dev_t *next;   /* +0xCC */
};

extern fio_dev_t *ps2fio_drv_list;

int ps2fioRegisterDevice(fio_dev_t *dev)
{
    fio_dev_t *p = ps2fio_drv_list;

    if (p != 0) {
        do {
            if (dev == p)
                return -0x40A;
            p = p->next;
        } while (p);
    }
    dev->next = ps2fio_drv_list;
    ps2fio_drv_list = dev;
    return 0;
}
