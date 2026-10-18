typedef struct {
    char pad[0xA0];
    int (*loadExec)(void *a, void *b, void *c);
} PS2FIO_DEVICE;

extern PS2FIO_DEVICE *func_1EBB8C0(void *a);

int ps2fioLoadExec(void *a, void *b, void *c) {
    PS2FIO_DEVICE *dev = func_1EBB8C0(a);
    int (*fn)(void *, void *, void *);

    if (!dev)
        return -0x13;
    fn = dev->loadExec;
    if (!fn)
        return -0x30;
    fn(a, b, c);
    return 0;
}
