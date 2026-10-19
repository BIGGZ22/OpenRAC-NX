/* sceSifSendCmd 0x01ED3B10..0x01ED3B50 (64 bytes, body 60 + 4 zero fill).
 * Sends a command packet with the dma/extra arguments shuffled into the
 * 7-argument worker; argument 2 of the worker is always 0. */
extern int func_1ED39D8(int, int, int, int, int, int, int);

int sceSifSendCmd(int a0, int a1, int a2, int a3, int t0, int t1) {
    return func_1ED39D8(a0, 0, a1, a2, a3, t0, t1);
}
