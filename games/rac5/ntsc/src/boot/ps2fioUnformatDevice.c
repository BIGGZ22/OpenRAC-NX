/* Authored from the pinned RAC5 boot instructions and measured
 * device-method offsets; no Sony SDK source or header used. */
extern void *func_1EBB8C0(int dev);

int ps2fioUnformatDevice(int dev) {
    void *h;
    int (*method)(int);
    h = func_1EBB8C0(dev);
    if (h == 0) return -0x13;
    method = (int (*)(int))*(int *)((char *)h + 0x38);
    if (method == 0) return -0x30;
    return method(dev);
}
