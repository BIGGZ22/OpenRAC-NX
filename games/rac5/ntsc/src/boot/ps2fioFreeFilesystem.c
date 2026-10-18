extern void *func_1EBB8C0(int dev);

int ps2fioFreeFilesystem(int dev) {
    void *p;
    int (*method)(void);
    p = func_1EBB8C0(dev);
    if (p == 0) return -0x13;
    method = (int (*)(void))*(int *)((char *)p + 0x90);
    if (method == 0) return -0x30;
    return method();
}
