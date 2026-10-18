extern void snd_SendIOPCommandAndWait(int command, int length, void *data);

void snd_StreamSafeCheckCDIdle(int handle) {
    snd_SendIOPCommandAndWait(0x36, 4, &handle);
}
