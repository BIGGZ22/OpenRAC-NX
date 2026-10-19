extern void snd_SendIOPCommandNoWait(int command, int size, void *buffer,
                                      int arg4, int arg5);

void snd_StreamSafeCheckCDIdle_CB(int a0, int a1, int a2) {
    int idle = a0;
    snd_SendIOPCommandNoWait(0x36, 4, &idle, a1, a2);
}
