extern void *func_1EBB8C0(int dev);

int ps2fioFormatDevice(int dev) {
    void *h;
    int (*method)(int);
    h = func_1EBB8C0(dev);
    if (h == 0) return -0x13;
    method = (int (*)(int))*(int *)((char *)h + 0x34);
    if (method == 0) return -0x30;
    return method(dev);
}
