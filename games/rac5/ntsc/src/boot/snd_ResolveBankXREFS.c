extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_ResolveBankXREFS(void) {
    snd_SendIOPCommandNoWait(8, 0, 0, 0, 0);
}
