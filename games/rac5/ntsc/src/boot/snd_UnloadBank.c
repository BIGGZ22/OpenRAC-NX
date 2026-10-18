extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_UnloadBank(int bank) {
    snd_SendIOPCommandNoWait(6, 4, &bank, 0, 0);
}
